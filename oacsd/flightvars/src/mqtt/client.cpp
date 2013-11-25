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

#include <iostream>

#include "mqtt/client.h"

namespace oac { namespace fv { namespace mqtt {

client::client(const oac::log_author& author) : logger_component(author) {}

void
client::on_message(const message& msg)
{
   log_trace("new message received from topic %s", msg.tpc.to_string());
   for (auto& subs : _subscriptions)
   {
      if (subs.first.match(msg.tpc))
      {
         subs.second(msg);
         return;
      }
   }
   log_warn(
         "no callback registered for message on topic %s",
         msg.tpc.to_string());
}

}}} // namespace oac::fv::mqtt
