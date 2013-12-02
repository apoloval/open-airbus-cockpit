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

#ifndef OAC_FV_CLIENT_MOSQUITTO_H
#define OAC_FV_CLIENT_MOSQUITTO_H

#include <string>

#include <liboac/exception.h>
#include <liboac/logging.h>
#include <liboac/thread/channel.h>
#include <mosquitto.h>

#include "mqtt/client.h"
#include "mqtt/topic.h"

namespace oac { namespace fv { namespace mqtt {

OAC_DECL_EXCEPTION_WITH_PARAMS(mosquitto_internal_error, client_exception,
   ("unexpected error while %s: error code %d", action, error_code),
   (action, std::string),
   (error_code, int)
);

OAC_DECL_EXCEPTION_WITH_PARAMS(mosquitto_timeout_error, client_exception,
   ("time out while expecting for %s", expectation),
   (expectation, std::string)
);

/**
 * A MQTT client implementation based on Mosquitto library.
 */
class mosquitto_client : public client
{
public:

   static const std::chrono::milliseconds DEFAULT_MSG_TIMEOUT;
   static const std::string DEFAULT_HOST;
   static const unsigned int DEFAULT_KEEP_ALIVE;
   static const std::uint16_t DEFAULT_PORT;

   mosquitto_client();

   ~mosquitto_client();

   virtual void publish(
         const topic& t,
         const void* data,
         std::size_t data_len,
         const qos_level& qos,
         bool retain);

protected:

   virtual void subscribe(
         const topic_pattern& pattern,
         const qos_level& qos);

private:

   enum class async_op
   {
      CONNECT,
      DISCONNECT,
      SUBSCRIBE
   };

   struct async_result
   {
      async_op op;
      int mid;
      int ec;
   };

   typedef thread::channel<async_result> async_result_channel;

   mosquitto* _mosq;
   std::string _server_host;
   std::uint16_t _server_port;

   async_result_channel _async_result_chan;

   static void connect_callback_dispatcher(
         mosquitto* mosq,
         void* client,
         int rc);

   static void disconnect_callback_dispatcher(
         mosquitto* mosq,
         void* client,
         int rc);

   static void message_callback_dispatcher(
         mosquitto* mosq,
         void* client,
         const mosquitto_message* msg);

   static void subscribe_callback_dispatcher(
         mosquitto* mosq,
         void* client,
         int mid,
         int qos_count,
         const int* granted_qos);

   static void log_callback_dispatcher(
         mosquitto* mosq,
         void* client,
         int level,
         const char* msg);

   void on_connect(int rc);

   void on_disconnect(int rc);

   void on_subscription_completed(int mid);

   void init();

   void connect(
         const std::string& host = DEFAULT_HOST,
         std::uint16_t port = DEFAULT_PORT,
         int keepalive = DEFAULT_KEEP_ALIVE);

   void set_callbacks();

   void start();

   void stop();

   void disconnect();

   void destroy();

   async_result wait_for_async(std::function<bool(const async_result&)> pred);

   int make_error_code(int mosq_error);
};

}}} // namespace oac::fv::mqtt

#endif
