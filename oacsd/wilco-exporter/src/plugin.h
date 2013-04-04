/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
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

#ifndef OAC_WE_PLUGIN_H
#define OAC_WE_PLUGIN_H

#include <liboac/fsuipc.h>
#include <liboac/simconn.h>

#include "wilco.h"

namespace oac { namespace we {

class Plugin
{
public:

   Plugin();

private:

   void onSimConnectOpen(
         SimConnectClient& client, const SIMCONNECT_RECV_OPEN& msg);

   void onSimConnectQuit(
         SimConnectClient& client, const SIMCONNECT_RECV& msg);

   void onSimConnectException(
         SimConnectClient& client, const SIMCONNECT_RECV_EXCEPTION& msg);

   void onSimConnectEvent(
         SimConnectClient& client, const SIMCONNECT_RECV_EVENT& msg);

   void onSimConnectEventFilename(
         SimConnectClient& client, const SIMCONNECT_RECV_EVENT_FILENAME& msg);

   void onSimConnectSimObjectData(
         SimConnectClient& client, const SIMCONNECT_RECV_SIMOBJECT_DATA& msg);

   void on1secElapsed();

   void onNewAircraft(const Aircraft::Title& title);

   void requestAircraftTitle();

   void registerOnAircraftLoadedCallback();

   void resetCockpit();
   void resetCockpit(const Aircraft& aircraft);

   SimConnectClient _sc;
   Ptr<WilcoCockpit> _wilco;
   Ptr<CockpitBack> _fsuipc;
};

}}; // namespace oac::we

#endif
