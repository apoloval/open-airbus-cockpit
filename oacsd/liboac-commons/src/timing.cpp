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

#include "timing.h"

namespace oac {

simconnect_tick_observer::simconnect_tick_observer(
      const simconnect_client::event_name& sc_event)
throw (connection_error)
   : _simconnect("OACSD Tick Observer")
{
   _simconnect.register_on_event_callback(
         std::bind(
               &simconnect_tick_observer::on_event,
               this,
               std::placeholders::_1,
               std::placeholders::_2));
   _simconnect.subscribe_to_system_event(sc_event);
}

void
simconnect_tick_observer::dispatch()
{
   _simconnect.dispatch_message();
}

void
simconnect_tick_observer::on_event(
      simconnect_client& client,
      const SIMCONNECT_RECV_EVENT& msg)
{
   notify_all();
}

} // namespace oac
