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



fsuipc_offset
to_fsuipc_offset(
      const variable_id& var_id)
throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error)
{
   fsuipc_offset_address addr;
   unsigned int len;
   auto var_group = get_var_group(var_id);
   auto var_name = get_var_name(var_id);

   if (var_group.get_tag() != VAR_GROUP_TAG)
      OAC_THROW_EXCEPTION(
            fsuipc::invalid_var_group_error(
                  VAR_GROUP_TAG, var_group.get_tag()));

   std::vector<std::string> parts;
   boost::split(parts, var_name.get_tag(), boost::is_any_of(":"));
   if (parts.size() <= 2)
   {
      // If no offset size is specified, assume BYTE
      if (parts.size() == 1)
         parts.push_back("1");

      if (!parse_dec_number(parts[0], addr) &&
          !parse_hex_number(parts[0], addr))
         goto syntax_error;

      if (!parse_dec_number(parts[1], len) &&
          !parse_hex_number(parts[1], len) &&
          !parse_textual_length(parts[1], len))
         goto syntax_error;
      if (len != 1 && len != 2 && len != 4)
         goto syntax_error;

      return fsuipc_offset(addr, fsuipc_offset_length(len));
   }
syntax_error:
   OAC_THROW_EXCEPTION(fsuipc::var_name_syntax_error(var_name));
}

variable_value
to_variable_value(
      const fsuipc_valued_offset& valued_offset)
{
   switch (valued_offset.length)
   {
      case OFFSET_LEN_BYTE:
         return variable_value::from_byte(valued_offset.value);
      case OFFSET_LEN_WORD:
         return variable_value::from_word(valued_offset.value);
      case OFFSET_LEN_DWORD:
         return variable_value::from_dword(valued_offset.value);
      default:
         // never reached
         OAC_THROW_EXCEPTION(
               enum_out_of_range_error<fsuipc_offset_length>(
                     valued_offset.length));
   }
}

fsuipc_offset_value
to_fsuipc_offset_value(
      const variable_value& var_value)
{
   // The float case is controversial. What kind of conversion should be done?
   // Rounding to integer? Binary conversion? Exception?
   switch (var_value.get_type())
   {
      case variable_type::BOOLEAN:
         return var_value.as_bool();
      case variable_type::BYTE:
         return var_value.as_byte();
      case variable_type::WORD:
         return var_value.as_word();
      case variable_type::DWORD:
         return var_value.as_dword();
      case variable_type::FLOAT:
         return fsuipc_offset_value(var_value.as_float());
      default:
         // never reached
         OAC_THROW_EXCEPTION(
               enum_out_of_range_error<variable_type>(
                     var_value.get_type()));
   }
}



subscription_meta
fsuipc_offset_db::create_subscription(
      const variable_id& var_id,
      const flight_vars::var_update_handler& callback)
throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error)
{
   auto offset = to_fsuipc_offset(var_id);
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
      const fsuipc_offset& offset) const
throw (no_such_offset_error)
{
   auto match = _offset_handlers.find(offset);
   if (match == _offset_handlers.end())
      OAC_THROW_EXCEPTION(no_such_offset_error(offset.address, offset.length));
   return match->second;
}

fsuipc_offset
fsuipc_offset_db::get_offset_for_subscription(
      const subscription_id& subs_id)
throw (no_such_subscription_error)
{
   for (auto& elem : _offset_handlers)
   {
      auto& offset = elem.first;
      auto& subscriptions = elem.second;
      for (auto& subs : subscriptions)
         if (subs.get_subscription_id() == subs_id)
            return offset;
   }
   OAC_THROW_EXCEPTION(no_such_subscription_error(subs_id));
}

void
fsuipc_offset_db::insert_known_offset(
      const fsuipc_offset& offset)
{
   if (std::none_of(
          _known_offsets.begin(),
          _known_offsets.end(),
          [&offset](const fsuipc_offset& elem) { return elem == offset; }))
   {
      _known_offsets.push_back(offset);
   }
}

void
fsuipc_offset_db::remove_known_offset(
      const fsuipc_offset& offset)
{
   _known_offsets.remove(offset);
}

subscription_meta
fsuipc_offset_db::insert_subscription(
      const fsuipc_offset& offset,
      const variable_id& var_id,
      const flight_vars::var_update_handler& callback)
{
   subscription_meta subs(var_id, callback);
   _offset_handlers[offset].push_back(subs);
   return subs;
}

std::size_t
fsuipc_offset_db::remove_subscription(
      const fsuipc_offset& offset,
      const subscription_id& subs_id)
{
   auto& list = _offset_handlers[offset];
   list.remove_if([&subs_id](const subscription_meta& meta)
   {
      return meta.get_subscription_id() == subs_id;
   });
   return list.size();
}

const variable_group local_fsuipc_flight_vars::VAR_GROUP(VAR_GROUP_TAG);

}} // namespace oac::fv
