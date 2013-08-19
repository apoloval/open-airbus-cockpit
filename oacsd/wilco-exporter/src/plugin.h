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

#ifndef OAC_WE_PLUGIN_H
#define OAC_WE_PLUGIN_H

#include <liboac/fsuipc.h>
#include <liboac/logging.h>
#include <liboac/simconn.h>

#include "wilco.h"

namespace oac { namespace we {

class plugin : public logger_component
{
public:

   plugin();

private:

   void on_simconnect_open(
         simconnect_client& client, const SIMCONNECT_RECV_OPEN& msg);

   void on_simconnect_quit(
         simconnect_client& client, const SIMCONNECT_RECV& msg);

   void on_simconnect_exception(
         simconnect_client& client, const SIMCONNECT_RECV_EXCEPTION& msg);

   void on_simconnect_event(
         simconnect_client& client, const SIMCONNECT_RECV_EVENT& msg);

   void on_simconnect_event_filename(
         simconnect_client& client, const SIMCONNECT_RECV_EVENT_FILENAME& msg);

   void on_simconnect_simobject_data(
         simconnect_client& client, const SIMCONNECT_RECV_SIMOBJECT_DATA& msg);

   void on_1sec_elapsed();

   void on_new_aircraft(const aircraft_title& title);

   void request_aircraft_title();

   void request_on_aircraft_loaded_callback();

   void reset_cockpit();
   void reset_cockpit(const aircraft& aircraft);

   simconnect_client _sc;
   ptr<wilco_cockpit> _wilco;
   ptr<cockpit_back> _fsuipc;
};

}}; // namespace oac::we

#endif
