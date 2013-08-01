/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012, 2013 Alvaro Polo
 *
 * Open Airbus Cockpit is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Open Airbus Cockpit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
 */

// This pragme disables warnings caused by boost::is_any_of()
#pragma warning( disable : 4996 )

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <liboac/logging.h>

#include "fsuipc.h"

namespace oac { namespace fv {

namespace {

const char* VAR_GROUP_TAG = "fsuipc/offset";

std::uint16_t parse_address(const std::string& str)
{
   std::stringstream ss;
   std::uint16_t r;
   ss << std::hex << str;
   ss >> r;
   return r;
}

template <typename IntegerType>
bool
parse_hex_number(
      const std::string& str,
      IntegerType& result)
{
   std::stringstream ss;
   ss << std::hex << str;
   ss >> result;
   return !(ss.fail() || !ss.eof());
}

template <typename IntegerType>
bool
parse_dec_number(
      const std::string& str,
      IntegerType& result)
{
   try
   {
      result = boost::lexical_cast<IntegerType>(str);
      return true;
   } catch (boost::bad_lexical_cast&)
   {
      return false;
   }
}

template <typename IntegerType>
bool
parse_textual_length(
      const std::string& str,
      IntegerType& result)
{
   if (boost::algorithm::to_lower_copy(str) == "byte")
   {
      result = 1;
      return true;
   }
   else if (boost::algorithm::to_lower_copy(str)== "word")
   {
      result = 2;
      return true;
   }
   else if (boost::algorithm::to_lower_copy(str)== "dword")
   {
      result = 4;
      return true;
   }
   return false;
}

} // anonymous namespace

std::size_t
fsuipc_offset_meta::hash(
      const fsuipc_offset_meta& meta)
{
   return 0;
}

fsuipc_offset_meta::fsuipc_offset_meta(
      const variable_id& var_id)
throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error)
{
   auto var_group = get_var_group(var_id);
   if (var_group.get_tag() != VAR_GROUP_TAG)
      BOOST_THROW_EXCEPTION(
               fsuipc::invalid_var_group_error() <<
               variable_group_info(var_group));

   auto var_name = get_var_name(var_id);
   std::vector<std::string> parts;
   boost::split(parts, var_name.get_tag(), boost::is_any_of(":"));
   if (parts.size() <= 2)
   {
      // If no offset size is specified, assume BYTE
      if (parts.size() == 1)
         parts.push_back("1");

      if (!parse_dec_number(parts[0], _address) &&
          !parse_hex_number(parts[0], _address))
         goto syntax_error;

      if (!parse_dec_number(parts[1], _length) &&
          !parse_hex_number(parts[1], _length) &&
          !parse_textual_length(parts[1], _length))
         goto syntax_error;
      if (_length != 1 && _length != 2 && _length != 4)
         goto syntax_error;
      return;
   }
syntax_error:
   BOOST_THROW_EXCEPTION(
            fsuipc::var_name_syntax_error() << variable_name_info(var_name));
}



subscription_meta
fsuipc_offset_db::create_subscription(
      const variable_id& var_id,
      const flight_vars::var_update_handler& callback)
throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error)
{
   fsuipc_offset_meta offset(var_id);
   insert_known_offset(offset);
   return insert_subscription(offset, var_id, callback);
}

void
fsuipc_offset_db::remove_subscription(
      const subscription_id& subs)
{
   auto it =_offset_handlers.begin(), end = _offset_handlers.end();
   while (it != end)
   {
      auto& offset = it->first;
      if (remove_subscription(offset, subs) == 0)
      {
         remove_known_offset(offset);
         it = _offset_handlers.erase(it);
      }
      else
         it++;
   }
}

const fsuipc_offset_db::subscription_list&
fsuipc_offset_db::get_subscriptions_for_offset(
      const fsuipc_offset_meta& offset) const
throw (unknown_offset_error)
{
   auto match = _offset_handlers.find(offset);
   if (match == _offset_handlers.end())
      BOOST_THROW_EXCEPTION(unknown_offset_error());
   return match->second;
}

fsuipc_offset_meta
fsuipc_offset_db::get_offset_for_subscription(
      const subscription_id& subs_id)
throw (unknown_subscription_error)
{
   for (auto& elem : _offset_handlers)
   {
      auto& offset = elem.first;
      auto& subscriptions = elem.second;
      for (auto& subs : subscriptions)
         if (subs.get_subscription_id() == subs_id)
            return offset;
   }
   BOOST_THROW_EXCEPTION(unknown_subscription_error());
}

void
fsuipc_offset_db::insert_known_offset(
      const fsuipc_offset_meta& offset)
{
   if (std::none_of(
          _known_offsets.begin(),
          _known_offsets.end(),
          [&offset](const fsuipc_offset_meta& elem) { return elem == offset; }))
   {
      _known_offsets.push_back(offset);
   }
}

void
fsuipc_offset_db::remove_known_offset(
      const fsuipc_offset_meta& offset)
{
   _known_offsets.remove(offset);
}

subscription_meta
fsuipc_offset_db::insert_subscription(
      const fsuipc_offset_meta& offset,
      const variable_id& var_id,
      const flight_vars::var_update_handler& callback)
{
   subscription_meta subs(var_id, callback);
   _offset_handlers[offset].push_back(subs);
   return subs;
}

std::size_t
fsuipc_offset_db::remove_subscription(
      const fsuipc_offset_meta& offset,
      const subscription_id& subs_id)
{
   auto& list = _offset_handlers[offset];
   list.remove_if([&subs_id](const subscription_meta& meta)
   {
      return meta.get_subscription_id() == subs_id;
   });
   return list.size();
}



const variable_group fsuipc_flight_vars::VAR_GROUP(VAR_GROUP_TAG);

fsuipc_flight_vars::fsuipc_flight_vars()
   : _sc("flight_vars - FSUIPC"),
     _fsuipc(new local_fsuipc()),
     _buffer(double_buffer<>::factory(
                new linear_buffer::factory(0xffff)).create_buffer())
{
   /**
    * Register a timed callback on SimConnect to check var changes
    * every 1/6 seconds.
    */
   log(INFO, "@FSUIPC; Registering 6HZ event in SimConnect...");
   _sc.register_on_event_callback(
            [this](simconnect_client&, const SIMCONNECT_RECV_EVENT&) {
      // We only register SYSTEM_EVENT_6HZ event, so args check is not needed
      notify_changes();
   });
   _sc.subscribe_to_system_event(simconnect_client::SYSTEM_EVENT_6HZ);
   _sc.dispatch_message();
   log(INFO, "@FSUIPC; 6HZ event successfully registered!");
}

subscription_id
fsuipc_flight_vars::subscribe(
      const variable_id& var,
      const var_update_handler& handler)
throw (unknown_variable_error)
{
   auto subs = _db.create_subscription(var, handler);
   auto subs_id = subs.get_subscription_id();

   log(
      INFO,
      boost::format("@FSUIPC; Subscribing on %s with ID %d...") %
         var_to_string(var) % subs_id);

   return subs_id;
}

void
fsuipc_flight_vars::unsubscribe(const subscription_id& id)
{
   _db.remove_subscription(id);
}

void
fsuipc_flight_vars::update(
      const subscription_id& subs_id,
      const variable_value& var_value)
throw (unknown_subscription_error, illegal_value_error)
{
   try
   {
      auto offset = _db.get_offset_for_subscription(subs_id);
      write_offset(offset, var_value);
   }
   catch (fsuipc_offset_db::unknown_subscription_error&)
   {
      BOOST_THROW_EXCEPTION(
         unknown_subscription_error() << subscription_info(subs_id));
   }
   catch (variable_value::invalid_type_error&)
   {
      BOOST_THROW_EXCEPTION(illegal_value_error());
   }
}

void
fsuipc_flight_vars::notify_changes()
{
   auto& offsets = _db.get_all_offsets();
   for (auto& offset : offsets)
   {
      sync_offset(offset);
      if (is_offset_updated(offset))
      {
         auto offset_value = read_offset(offset);
         for (auto& subs : _db.get_subscriptions_for_offset(offset))
            subs.get_update_handler()(subs.get_variable(), offset_value);
      }
   }
   _buffer->swap();
}

void
fsuipc_flight_vars::sync_offset(const fsuipc_offset_meta& offset)
{
   auto address = offset.address();
   auto length = offset.length();
   _buffer->copy(*_fsuipc, address, address, length);
}

bool
fsuipc_flight_vars::is_offset_updated(
      const fsuipc_offset_meta& offset) const
{
   auto address = offset.address();
   auto length = offset.length();
   switch (length)
   {
      case 1: return _buffer->is_modified_as<std::uint8_t>(address);
      case 2: return _buffer->is_modified_as<std::uint16_t>(address);
      case 4: return _buffer->is_modified_as<std::uint32_t>(address);
      default: BOOST_THROW_EXCEPTION(illegal_state_error()); // never happens
   }
}

variable_value
fsuipc_flight_vars::read_offset(
      const fsuipc_offset_meta& offset) const
{
   auto address = offset.address();
   auto length = offset.length();
   switch (length)
   {
      case 1:
         return variable_value::from_byte(
               buffer::read_as<std::uint8_t>(*_buffer, address));
      case 2:
         return variable_value::from_word(
               buffer::read_as<std::uint16_t>(*_buffer, address));
      case 4:
         return variable_value::from_dword(
               buffer::read_as<std::uint32_t>(*_buffer, address));
      default:
         BOOST_THROW_EXCEPTION(illegal_state_error()); // never happens
   }
}

void
fsuipc_flight_vars::write_offset(
      const fsuipc_offset_meta& offset,
      const variable_value& value)
{
   auto address = offset.address();
   auto length = offset.length();
   switch (length)
   {
      case 1:
         buffer::write_as<std::uint8_t>(
            *_fsuipc, address, value.as_byte());
      case 2:
         buffer::write_as<std::uint16_t>(
            *_fsuipc, address, value.as_word());
      case 4:
         buffer::write_as<std::uint32_t>(
            *_fsuipc, address, value.as_dword());
      default:
         BOOST_THROW_EXCEPTION(illegal_state_error()); // never happens
   }
}

}} // namespace oac::fv
