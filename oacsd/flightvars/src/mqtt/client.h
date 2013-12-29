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

#ifndef OAC_FV_MQTT_CLIENT_H
#define OAC_FV_MQTT_CLIENT_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>

#include <liboac/exception.h>
#include <liboac/logging.h>

#include "mqtt/topic.h"
#include "mqtt/message.h"
#include "mqtt/qos.h"

namespace oac { namespace fv { namespace mqtt {

OAC_DECL_ABSTRACT_EXCEPTION(client_exception);

OAC_DECL_EXCEPTION_WITH_PARAMS(connection_error, client_exception,
   ("cannot connect to mqtt server on %s:%d: error code %d",
         host, port, error_code),
   (host, std::string),
   (port, std::uint16_t),
   (error_code, int)
);

OAC_DECL_EXCEPTION_WITH_PARAMS(subscribe_error, client_exception,
   ("cannto subscribe to pattern %s: error code %d",
         pattern.to_string(), error_code),
   (pattern, topic_pattern),
   (error_code, int)
);

OAC_DECL_EXCEPTION_WITH_PARAMS(publish_error, client_exception,
   ("cannot publish on topic %s: error code %d", tpc.to_string(), error_code),
   (tpc, topic),
   (error_code, int)
);

/**
 * An MQTT client that may be used to publish messages and subscribe to topic
 * patterns.
 */
class client : public oac::logger_component, public message_publisher
{
public:

   /** A callback function used to process raw messages published on a topic. */
   using raw_message_callback = std::function<void(const raw_message&)>;

   /**
    * A callback function used to process typed messages published on a topic.
    */
   template <typename Data>
   using typed_message_callback = std::function<void(const message<Data>&)>;

   /** A callback function used to process data published on a topic. */
   template <typename Data>
   using data_callback = std::function<void(const Data&)>;

   void subscribe(
         const topic_pattern& pattern,
         const qos_level& qos,
         const raw_message_callback& callback)
   {
      std::unique_lock<std::mutex> lock { _mutex };
      _subscriptions[pattern] = callback;
      subscribe_to(pattern, qos);
   }

   template <typename Data>
   void subscribe_as(
         const topic_pattern& pattern,
         const qos_level& qos,
         const typed_message_callback<Data>& callback)
   {
      subscribe(
            pattern,
            qos,
            [this, callback, pattern](const raw_message& msg)
      {
         try { callback(msg.to_typed<Data>()); }
         catch (const raw_message::conversion_error& e)
         {
            log_warn(
                  "cannot deliver message for topic pattern %s: %s",
                  pattern.to_string(),
                  e.report());

         }
      });
   }

   /**
    * Subscribe to given topic pattern using the given data callback function.
    *
    * This function may be use to subscribe the client to a topic pattern
    * and register a data callback function to be used to process incoming
    * messages directly from its data. The callback function determines the
    * data type of the expected messages. Internally, if the message length
    * doesn't match the size of expected Data, the message is discarded.
    */
   template <typename Data>
   void subscribe_as_data(
         const topic_pattern& pattern,
         const qos_level& qos,
         const data_callback<Data>& callback)
   {
      subscribe_as<Data>(pattern, qos, [callback](
            const message<Data>& msg)
      {
         callback(msg.data);
      });
   }

   /**
    * Publish a new message from raw bytes on given topic.
    */
   virtual void publish(const raw_message& msg) = 0;

protected:

   client(const oac::log_author& author);

   virtual void subscribe_to(
         const topic_pattern& pattern,
         const qos_level& qos) = 0;

   void on_message(const raw_message& msg);

private:

   struct pattern_compare
   {
      bool operator()(
            const topic_pattern& lhs,
            const topic_pattern& rhs)
      { return lhs.to_string() < rhs.to_string(); }
   };

   using subscription_map =
         std::map<topic_pattern, raw_message_callback, pattern_compare>;

   subscription_map _subscriptions;
   std::mutex _mutex;
};

using client_ptr = std::shared_ptr<client>;

}}} // namespace oac::fv::mqtt

#endif
