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
class client : public oac::logger_component
{
public:

   /** A callback function used to process messages published on a topic. */
   template <typename Data>
   using message_callback = std::function<void(const message<Data>&)>;

   /** A callback function used to process data published on a topic. */
   template <typename Data>
   using data_callback = std::function<void(const Data&)>;

   template <typename Data>
   void subscribe_as_message(
         const topic_pattern& pattern,
         const qos_level& qos,
         const message_callback<Data>& callback)
   {
      _subscriptions[pattern] = [this, callback, pattern](
            const buffered_message& msg)
      {
         try { callback(msg.convert_to<Data>()); }
         catch (const buffered_message::conversion_error& e)
         {
            log_warn(
                  "cannot deliver message for topic pattern %s: %s",
                  pattern.to_string(),
                  e.report());

         }
      };
      subscribe(pattern, qos);
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
      subscribe_as_message<Data>(pattern, qos, [callback](
            const message<Data>& msg)
      {
         callback(msg.data);
      });
   }

   /**
    * Publish a new message from raw bytes on given topic.
    */
   virtual void publish(
         const topic& t,
         const void* data,
         std::size_t data_len,
         const qos_level& qos = qos_level::LEVEL_0,
         bool retain = false) = 0;

   /**
    * Publish a new message from an object on given topic.
    */
   template <typename T>
   void publish_as(
         const topic& t,
         const T& data,
         const qos_level& qos = qos_level::LEVEL_0,
         bool retain = false)
   {
      publish(t, &data, sizeof(T), qos, retain);
   }

protected:

   client(const oac::log_author& author);

   virtual void subscribe(
         const topic_pattern& pattern,
         const qos_level& qos) = 0;

   void on_message(const buffered_message& msg);

private:

   struct pattern_compare
   {
      bool operator()(
            const topic_pattern& lhs,
            const topic_pattern& rhs)
      { return lhs.to_string() < rhs.to_string(); }
   };

   using buffered_message_callback =
         std::function<void(const buffered_message&)>;
   using subscription_map =
         std::map<topic_pattern, buffered_message_callback, pattern_compare>;

   subscription_map _subscriptions;
};

}}} // namespace oac::fv::mqtt

#endif
