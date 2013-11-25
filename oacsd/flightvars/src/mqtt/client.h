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

#ifndef OAC_FV_CLIENT_H
#define OAC_FV_CLIENT_H

#include <cstdint>
#include <map>
#include <string>

#include <liboac/exception.h>
#include <liboac/logging.h>

#include "mqtt/topic.h"

namespace oac { namespace fv { namespace mqtt {

enum class qos_level
{
   LEVEL_0,
   LEVEL_1,
   LEVEL_2
};

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

   /**
    * Subscribe to given topic pattern using the given callback function.
    * This function may be use to subscribe the client to a topic pattern
    * and register a callback function to be used to process incoming
    * messages. The callback function determines the data type of the expected
    * messages. Internally, if the message length doesn't match the size of
    * expected Data, the message is discarded.
    */
   template <typename Data>
   void subscribe_as(
         const topic_pattern& pattern,
         const qos_level& qos,
         std::function<void(const Data&)> callback)
   {
      _subscriptions[pattern] = [this, callback, pattern](const message& msg)
      {
         if (sizeof(Data) == msg.data_len)
            callback(*static_cast<const Data*>(msg.data));
         else
            log_warn(
                  "cannot deliver message for topic pattern %s: "
                  "message length (%d bytes) does not match expected "
                  "length of %d bytes",
                  pattern.to_string(),
                  msg.data_len,
                  sizeof(Data));
      };
      subscribe(pattern, qos);
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

   struct message
   {
      topic tpc;
      const void* data;
      std::size_t data_len;
      qos_level qos;
      bool retain;
   };

   client(const oac::log_author& author);

   virtual void subscribe(
         const topic_pattern& pattern,
         const qos_level& qos) = 0;

   void on_message(const message& msg);

private:

   struct pattern_compare
   {
      bool operator()(
            const topic_pattern& lhs,
            const topic_pattern& rhs)
      { return lhs.to_string() < rhs.to_string(); }
   };

   typedef std::function<void(const message&)> callback;
   typedef std::map<topic_pattern, callback, pattern_compare> subscription_map;

   subscription_map _subscriptions;
};

}}} // namespace oac::fv::mqtt

#endif
