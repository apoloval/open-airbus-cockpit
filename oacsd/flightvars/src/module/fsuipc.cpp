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
#include <liboac/logging.h>

#include "fsuipc.h"

namespace oac { namespace fv {

namespace {

DWORD parseHexadecimal(const std::string& str)
{
   std::stringstream ss;
   DWORD r;
   ss << std::hex << str;
   ss >> r;
   return r;
}

}

const VariableGroup FsuipcFlightVars::VAR_GROUP("fsuipc/offset");

FsuipcFlightVars::FsuipcFlightVars(const Ptr<Buffer>& fsuipc)
   : _sc("FlightVars - FSUIPC"),
     _fsuipc(fsuipc),
     _buffer(DoubleBuffer::Factory(
                new FixedBuffer::Factory(0xffff)).createBuffer())
{
   /**
    * Register a timed callback on SimConnect to check var changes
    * every 1/6 seconds.
    */
   Log(INFO, "@FSUIPC; Registering 6HZ event in SimConnect...");
   _sc.registerOnEventCallback(
            [this](SimConnectClient&, const SIMCONNECT_RECV_EVENT&) {
      // We only register SYSTEM_EVENT_6HZ event, so args check is not needed
      notifyChanges();
   });
   _sc.subscribeToSystemEvent(SimConnectClient::SYSTEM_EVENT_6HZ);
   _sc.dispatchMessage();
   Log(INFO, "@FSUIPC; 6HZ event successfully registered!");

   // Initialize FSUIPC interface
   if (!_fsuipc)
   {
      Log(INFO, "@FSUIPC; Initializing FSUIPC interface... ");
      _fsuipc = new LocalFSUIPC();
      Log(INFO, "@FSUIPC; FSUIPC interface successfully initialized!");
   }
}

void
FsuipcFlightVars::subscribe(
      const VariableGroup& grp,
      const VariableName& name,
      const Subscription& subs)
throw (UnknownVariableError)
{
   Log(INFO, boost::format("@FSUIPC; Subscribing on %s -> %s...")
       % grp.tag() % name.tag());
   checkGroup(grp);
   Offset offset(name);
   subscribe(offset, subs);
   Log(INFO, "@FSUIPC; Subscribed successfully!");
}

void
FsuipcFlightVars::checkGroup(const VariableGroup& grp)
throw (UnknownVariableGroupError)
{
   if (grp != VAR_GROUP)
      THROW_ERROR(UnknownVariableGroupError() << VariableGroupInfo(grp));
}

void
FsuipcFlightVars::subscribe(const Offset& offset,
                            const Subscription& subs)
{
   // Insert a new subscription list, or take the existing one if any
   auto entry = _subscribers.find(offset);
   if (entry == _subscribers.end())
   {
      _subscribers[offset] = SubscriptionList();
      entry = _subscribers.find(offset);
   }
   entry->second.push_back(subs);
}

void
FsuipcFlightVars::notifyChanges()
{
   for (auto entry : _subscribers)
   {
      auto offset = entry.first;
      syncOffset(offset);
      if (offset.isUpdated(*_buffer))
         for (auto subs : entry.second)
         {
            subs(VAR_GROUP, offset.var_name, offset.read(*_buffer));
         }
   }
   _buffer->swap();
}

void
FsuipcFlightVars::syncOffset(const Offset& offset)
{
   _buffer->copy(*_fsuipc, offset.address, offset.address, offset.length);
}

FsuipcFlightVars::Offset::Offset(const VariableName& var_name)
throw (UnknownVariableNameError)
   : TaggedElement(var_name),
     var_name(var_name)
{
   try
   {
      std::vector<std::string> parts;
      boost::split(parts, var_name.tag(), boost::is_any_of(":"));
      if (parts.size() <= 2)
      {
         // If no offset size is specified, assume BYTE
         if (parts.size() == 1)
            parts.push_back("1");

         address = parseHexadecimal(parts[0]);
         length = boost::lexical_cast<DWORD>(parts[1]);
         if (length == 1 || length == 2 || length == 4)
            return; // Valid len, otherwise continue to throw
      }
   }
   catch(boost::bad_lexical_cast&) {
      // Let's continue and throw below
   }
   THROW_ERROR(UnknownVariableNameError() << VariableNameInfo(var_name));
}

bool
FsuipcFlightVars::Offset::isUpdated(DoubleBuffer& buf)
{
   switch (length)
   {
      case 1: return buf.isModifiedAs<BYTE>(address);
      case 2: return buf.isModifiedAs<WORD>(address);
      case 4: return buf.isModifiedAs<DWORD>(address);
      default: THROW_ERROR(IllegalStateError()); // never happens
   }
}

VariableValue
FsuipcFlightVars::Offset::read(DoubleBuffer& buf)
{
   switch (length)
   {
      case 1: return VariableValue::fromByte(buf.readAs<BYTE>(address));
      case 2: return VariableValue::fromWord(buf.readAs<WORD>(address));
      case 4: return VariableValue::fromDWord(buf.readAs<DWORD>(address));
      default: THROW_ERROR(IllegalStateError()); // never happens
   }

}


}} // namespace oac::fv
