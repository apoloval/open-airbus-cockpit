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

std::uint16_t parse_address(const std::string& str)
{
   std::stringstream ss;
   std::uint16_t r;
   ss << std::hex << str;
   ss >> r;
   return r;
}

} // anonymous namespace

const variable_group fsuipc_flight_vars::VAR_GROUP("fsuipc/offset");

fsuipc_flight_vars::fsuipc_flight_vars()
   : _sc("flight_vars - FSUIPC"),
     _fsuipc(new local_fsuipc()),
     _buffer(double_buffer<>::factory(
                new fixed_buffer::factory(0xffff)).create_buffer())
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

flight_vars::subscription_id
fsuipc_flight_vars::subscribe(
      const variable_group& grp,
      const variable_name& name,
      const var_update_handler& handler)
throw (unknown_variable_error)
{
   log(INFO, boost::format("@FSUIPC; Subscribing on %s -> %s...")
       % grp.get_tag() % name.get_tag());
   check_group(grp);
   offset offset(name);
   auto id = subscribe(offset, handler);
   log(INFO, boost::format("@FSUIPC; Subscribed successfully with ID %s!") %
       boost::uuids::to_string(id));
   return id;
}

void
fsuipc_flight_vars::unsubscribe(const flight_vars::subscription_id& id)
{
   for (auto& entry : _subscribers)
   {
      auto s = entry.second.begin(), end = entry.second.end();
      while (s != end)
      {
         if (s->id == id)
            s = entry.second.erase(s);
      }
   }
}

void
fsuipc_flight_vars::check_group(const variable_group& grp)
throw (unknown_variable_group_error)
{
   if (grp != VAR_GROUP)
      BOOST_THROW_EXCEPTION(unknown_variable_group_error() <<
            variable_group_info(grp));
}

flight_vars::subscription_id
fsuipc_flight_vars::subscribe(
      const offset& offset, const var_update_handler& handler)
{
   // Insert a new subscription list, or take the existing one if any
   auto entry = _subscribers.find(offset);
   if (entry == _subscribers.end())
   {
      _subscribers[offset] = subscription_list();
      entry = _subscribers.find(offset);
   }
   subscription subs(handler);
   entry->second.push_back(subs);
   return subs.id;
}

void
fsuipc_flight_vars::notify_changes()
{
   for (auto entry : _subscribers)
   {
      auto offset = entry.first;
      sync_offset(offset);
      if (offset.is_updated(*_buffer))
         for (auto subs : entry.second)
         {
            subs.handler(VAR_GROUP, offset.var_name, offset.read(*_buffer));
         }
   }
   _buffer->swap();
}

void
fsuipc_flight_vars::sync_offset(const offset& offset)
{
   _buffer->copy(*_fsuipc, offset.address, offset.address, offset.length);
}

fsuipc_flight_vars::offset::offset(const variable_name& var_name)
throw (unknown_variable_name_error)
   : tagged_element(var_name),
     var_name(var_name)
{
   try
   {
      std::vector<std::string> parts;
      boost::split(parts, var_name.get_tag(), boost::is_any_of(":"));
      if (parts.size() <= 2)
      {
         // If no offset size is specified, assume BYTE
         if (parts.size() == 1)
            parts.push_back("1");

         address = parse_address(parts[0]);
         length = boost::lexical_cast<std::uint32_t>(parts[1]);
         if (length == 1 || length == 2 || length == 4)
            return; // Valid len, otherwise continue to throw
      }
   }
   catch(boost::bad_lexical_cast&) {
      // Let's continue and throw below
   }
   BOOST_THROW_EXCEPTION(unknown_variable_name_error() <<
         variable_name_info(var_name));
}

bool
fsuipc_flight_vars::offset::is_updated(double_buffer<>& buf)
{
   switch (length)
   {
      case 1: return buf.is_modified_as<std::uint8_t>(address);
      case 2: return buf.is_modified_as<std::uint16_t>(address);
      case 4: return buf.is_modified_as<std::uint32_t>(address);
      default: BOOST_THROW_EXCEPTION(illegal_state_error()); // never happens
   }
}

variable_value
fsuipc_flight_vars::offset::read(double_buffer<>& buff)
{
   switch (length)
   {
      case 1:
         return variable_value::from_byte(
               buffer::read_as<std::uint8_t>(buff, address));
      case 2:
         return variable_value::from_word(
               buffer::read_as<std::uint16_t>(buff, address));
      case 4:
         return variable_value::from_dword(
               buffer::read_as<std::uint32_t>(buff, address));
      default:
         BOOST_THROW_EXCEPTION(illegal_state_error()); // never happens
   }

}


}} // namespace oac::fv
