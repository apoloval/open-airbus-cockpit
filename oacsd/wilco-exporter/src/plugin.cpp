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

plugin::plugin()
 : logger_component("plugin"),
   _sc(
         "Wilco Exporter Plugin",
         std::bind(&plugin::on_simconnect_open, this, _1, _2))
{
   _sc.register_on_quit_callback(
         std::bind(&plugin::on_simconnect_quit, this, _1, _2));
   _sc.register_on_exception_callback(
         std::bind(&plugin::on_simconnect_exception, this, _1, _2));
   _sc.register_on_event_callback(
         std::bind(&plugin::on_simconnect_event, this, _1, _2));
   _sc.register_on_simobject_data_callback(
         std::bind(&plugin::on_simconnect_simobject_data, this, _1, _2));
   _sc.subscribe_to_system_event(
         simconnect_client::SYSTEM_EVENT_1SEC, EVENT_1SEC_ELAPSED);
   _sc.subscribe_to_system_event(
         simconnect_client::SYSTEM_EVENT_FLIGHT_LOADED, EVENT_FLIGHT_LOADED);
}

void
plugin::on_simconnect_open(
      simconnect_client& client, const SIMCONNECT_RECV_OPEN& msg)
{
   log(log_level::INFO, "Connection to SimConnect successfully open.");
   this->request_aircraft_title();
   this->request_on_aircraft_loaded_callback();
}

void
plugin::on_simconnect_quit(simconnect_client& client, const SIMCONNECT_RECV& msg)
{
   log(log_level::INFO, "Connection to SimConnect closed.");
}

void
plugin::on_simconnect_exception(
      simconnect_client& client, const SIMCONNECT_RECV_EXCEPTION& msg)
{
   log(log_level::WARN, "unexpected exception raised: code %d", msg.dwException);
}

void
plugin::on_simconnect_event(
      simconnect_client& client, const SIMCONNECT_RECV_EVENT& msg)
{
   switch (msg.uEventID)
   {
      case EVENT_1SEC_ELAPSED:
         this->on_1sec_elapsed();
         break;
   }
}

void
plugin::on_simconnect_event_filename(simconnect_client& client,
      const SIMCONNECT_RECV_EVENT_FILENAME& msg)
{
   switch (msg.uEventID)
   {
      case EVENT_AIRCRAFT_LOADED:
         this->request_aircraft_title();
         break;
      case EVENT_FLIGHT_LOADED:
         log(log_level::INFO, "New flight is loaded, reset and check the new aircraft");
         this->reset_cockpit();
         this->request_aircraft_title();
         break;
   }
}

void
plugin::on_simconnect_simobject_data(simconnect_client& client,
      const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
{
   switch (msg.dwRequestID)
   {
      case DATA_REQ_TITLE:
         this->on_new_aircraft((char*) &msg.dwData);
         break;
   }
}

void plugin::on_1sec_elapsed()
{
   if (_wilco && _fsuipc)
      _fsuipc->map(*_wilco);
}

void
plugin::on_new_aircraft(const aircraft_title& title)
{
   log(log_level::INFO, "New aircraft loaded: %s", title);
   try
   {
      this->reset_cockpit(aircraft(title));
   } catch (aircraft::invalid_title& e) {
      log(
            log_level::INFO,
            "No Wilco Airbus aircraft resolved for title '%s':\n%s",
            title,
            e.report());
      this->reset_cockpit();
   }
}

void
plugin::request_aircraft_title()
{
   auto data = _sc.new_data_definition()
         .add("TITLE", "", SIMCONNECT_DATATYPE_STRING256);
   simconnect_client::data_pull_request(_sc, data, DATA_REQ_TITLE)
         .submit();
}

void
plugin::request_on_aircraft_loaded_callback()
{
   _sc.register_on_event_filename_callback(
         std::bind(&plugin::on_simconnect_event_filename, this, _1, _2));
   _sc.subscribe_to_system_event(
            simconnect_client::SYSTEM_EVENT_AIRCRAFT_LOADED,
            EVENT_AIRCRAFT_LOADED);
}

void
plugin::reset_cockpit()
{
   log(log_level::INFO, "Wilco cockpit reset");
   _wilco.reset();
   _fsuipc.reset();
}

void 
plugin::reset_cockpit(const aircraft& aircraft)
{
   try
   {
      log(
            log_level::INFO,
            "Initializing Wilco cockpit for %s... ",
            aircraft.title);
      _wilco = wilco_cockpit::new_cockpit(aircraft);
      _fsuipc = std::make_shared<fsuipc_cockpit_back>(
            std::make_shared<fsuipc::local_fsuipc::factory>());
      log(log_level::INFO, "Wilco Cockpit successfully initialized");
   } catch (std::exception& ex) {
      log(log_level::WARN, ex.what());
      this->reset_cockpit();
   }   
}

}}; // namespace oac::we
