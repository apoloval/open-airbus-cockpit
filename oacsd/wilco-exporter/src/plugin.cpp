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

#include <functional>

#include <liboac/cockpit-fsuipc.h>
#include <liboac/logging.h>

#include "plugin.h"

using namespace std::placeholders;

namespace oac { namespace we {

namespace {

enum DataRequest
{
   DATA_REQ_TITLE,
};

enum Event
{
   EVENT_1SEC_ELAPSED,
   EVENT_AIRCRAFT_LOADED,
   EVENT_FLIGHT_LOADED,
};

}; // anonymous namespace

Plugin::Plugin() : _sc(
   "Wilco Exporter Plugin", std::bind(&Plugin::onSimConnectOpen, this, _1, _2))
{
   _sc.registerOnQuitCallback(
         std::bind(&Plugin::onSimConnectQuit, this, _1, _2));
   _sc.registerOnExceptionCallback(
         std::bind(&Plugin::onSimConnectException, this, _1, _2));
   _sc.registerOnEventCallback(
         std::bind(&Plugin::onSimConnectEvent, this, _1, _2));
   _sc.registerOnSimObjectDataCallback(
         std::bind(&Plugin::onSimConnectSimObjectData, this, _1, _2));
   _sc.subscribeToSystemEvent(
         SimConnectClient::SYSTEM_EVENT_1SEC, EVENT_1SEC_ELAPSED);
   _sc.subscribeToSystemEvent(
         SimConnectClient::SYSTEM_EVENT_FLIGHT_LOADED, EVENT_FLIGHT_LOADED);
}

void
Plugin::onSimConnectOpen(
      SimConnectClient& client, const SIMCONNECT_RECV_OPEN& msg)
{
   Log(INFO, "Connection to SimConnect successfully open.");
   this->requestAircraftTitle();
   this->registerOnAircraftLoadedCallback();
}

void
Plugin::onSimConnectQuit(SimConnectClient& client, const SIMCONNECT_RECV& msg)
{
   Log(INFO, "Connection to SimConnect closed.");
}

void
Plugin::onSimConnectException(
      SimConnectClient& client, const SIMCONNECT_RECV_EXCEPTION& msg)
{
   Log(WARN, boost::format("unexpected exception raised: code %d")
         % msg.dwException);
}

void
Plugin::onSimConnectEvent(
      SimConnectClient& client, const SIMCONNECT_RECV_EVENT& msg)
{
   switch (msg.uEventID)
   {
      case EVENT_1SEC_ELAPSED:
         this->on1secElapsed();
         break;
   }
}

void
Plugin::onSimConnectEventFilename(SimConnectClient& client,
      const SIMCONNECT_RECV_EVENT_FILENAME& msg)
{
   switch (msg.uEventID)
   {
      case EVENT_AIRCRAFT_LOADED:
         this->requestAircraftTitle();
         break;
      case EVENT_FLIGHT_LOADED:
         Log(INFO, "New flight is loaded, reset and check the new aircraft");
         this->resetCockpit();
         this->requestAircraftTitle();
         break;
   }
}

void
Plugin::onSimConnectSimObjectData(SimConnectClient& client,
      const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
{
   switch (msg.dwRequestID)
   {
      case DATA_REQ_TITLE:
         this->onNewAircraft((char*) &msg.dwData);
         break;
   }
}

void Plugin::on1secElapsed()
{
   if (_wilco && _fsuipc)
      _fsuipc->map(*_wilco);
}

void
Plugin::onNewAircraft(const Aircraft::Title& title)
{
   Log(INFO, boost::format("New aircraft loaded: %s") % title);
   try
   {
      this->resetCockpit(Aircraft(title));
   } catch (Aircraft::InvalidTitle& e) {
      Log(INFO, boost::format(
            "No Wilco Airbus aircraft resolved for title '%s'") %
                  GetErrorInfo<Aircraft::TitleInfo>(e));
      this->resetCockpit();
   }
}

void
Plugin::requestAircraftTitle()
{
   auto data = _sc.newDataDefinition()
         .add("TITLE", "", SIMCONNECT_DATATYPE_STRING256);
   SimConnectClient::DataPullRequest(_sc, data, DATA_REQ_TITLE)
         .submit();
}

void
Plugin::registerOnAircraftLoadedCallback()
{
   _sc.registerOnEventFilenameCallback(
         std::bind(&Plugin::onSimConnectEventFilename, this, _1, _2));
   _sc.subscribeToSystemEvent(
            SimConnectClient::SYSTEM_EVENT_AIRCRAFT_LOADED,
            EVENT_AIRCRAFT_LOADED);
}

void
Plugin::resetCockpit()
{
   Log(INFO, "Wilco cockpit reset");
   _wilco.reset();
   _fsuipc.reset();
}

void 
Plugin::resetCockpit(const Aircraft& aircraft)
{
   try
   {
      Log(INFO, boost::format("Initializing Wilco cockpit for %s... ")
            % aircraft.title);
      _wilco = WilcoCockpit::newCockpit(aircraft);
      _fsuipc = new FSUIPCCockpitBack(new LocalFSUIPC::Factory());
      Log(INFO, "Wilco Cockpit successfully initialized");
   } catch (std::exception& ex) {
      Log(WARN, ex.what());
      this->resetCockpit();
   }   
}

}}; // namespace oac::we
