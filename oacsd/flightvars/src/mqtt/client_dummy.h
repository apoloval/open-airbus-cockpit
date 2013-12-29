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

#ifndef OAC_FV_CLIENT_DUMMY_H
#define OAC_FV_CLIENT_DUMMY_H

#include <string>

#include "mqtt/client.h"

namespace oac { namespace fv { namespace mqtt {

class dummy_client : public mqtt::client
{
public:

   struct subscription
   {
      mqtt::topic_pattern pattern;
      mqtt::qos_level qos;
   };

   using subscription_list = std::list<subscription>;
   using message_list = std::list<mqtt::raw_message>;

   dummy_client() : mqtt::client { "dummy_mqtt_client" } {}

   void subscribe_to(
         const mqtt::topic_pattern& pattern,
         const mqtt::qos_level& qos) override
   {
      subscription subs { pattern, qos };
      _subscriptions.push_back(subs);
   }

   void publish(const mqtt::raw_message& msg) override
   {
      _messages.push_back(msg);
      on_message(msg);
   }

   const subscription_list& subscriptions() const
   { return _subscriptions; }

   const message_list& messages() const
   { return _messages; }

private:

   subscription_list _subscriptions;
   message_list _messages;

};

using dummy_client_ptr = std::shared_ptr<dummy_client>;

}}}

#endif
