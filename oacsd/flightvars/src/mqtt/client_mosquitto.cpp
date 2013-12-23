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

#include "mqtt/client_mosquitto.h"

namespace oac { namespace fv { namespace mqtt {

const std::chrono::milliseconds
mosquitto_client::DEFAULT_MSG_TIMEOUT(10000);

const std::string
mosquitto_client::DEFAULT_HOST("localhost");

const unsigned int
mosquitto_client::DEFAULT_KEEP_ALIVE(10);

const std::uint16_t
mosquitto_client::DEFAULT_PORT(1883);

mosquitto_client::mosquitto_client(
      const std::string& server_host,
      std::uint16_t server_port)
 : client("mosquitto_client"),
   _mosq(nullptr),
   _server_host(server_host),
   _server_port(server_port)
{
   init();
   set_callbacks();
   start();
   connect();
}

mosquitto_client::~mosquitto_client()
{
   disconnect();
   stop();
   destroy();
}

void
mosquitto_client::publish(const raw_message& msg)
{
   auto data_len = msg.data.capacity();
   log_trace("Publishing %d bytes of data to topic %s",
         data_len, msg.tpc.to_string());
   auto error = mosquitto_publish(
         _mosq,                        // mosq
         nullptr,                      // mid
         msg.tpc.to_c_str(),           // topic
         data_len,                     // payload_len
         msg.data.data(),              // payload
         static_cast<int>(msg.qos),    // qos
         msg.retain                    // retain
   );
   if (error != MOSQ_ERR_SUCCESS)
      OAC_THROW_EXCEPTION(publish_error(msg.tpc, make_error_code(error)));
}

void
mosquitto_client::subscribe_to(
      const topic_pattern& pattern,
      const qos_level& qos)
{
   log_info("subscribing to topic pattern %s... ", pattern.to_string());
   int mid;
   auto error = mosquitto_subscribe(
      _mosq,                  // mosq
      &mid,                   // mid
      pattern.to_c_str(),     // sub
      static_cast<int>(qos)   // qos
   );
   if (error != MOSQ_ERR_SUCCESS)
   {
      auto ec = make_error_code(error);
      log_error(
            "subscription to %s failed: error code %d",
            pattern.to_string(), ec);
      OAC_THROW_EXCEPTION(subscribe_error(pattern, ec));
   }

   try
   {
      wait_for_async([mid](const async_result& r)
      {
         return (r.op == async_op::SUBSCRIBE && r.mid == mid);
      });
      log_info(
            "subscribed to topic pattern %s successfully", pattern.to_string());
   }
   catch (thread::channel_timeout_error& e)
   {
      log_error(
            "subscription to topic pattern %s timed out", pattern.to_string());
      OAC_THROW_EXCEPTION(mosquitto_timeout_error("subscription ACK", e));
   }
}

void
mosquitto_client::connect_callback_dispatcher(
      mosquitto* mosq,
      void* client,
      int rc)
{
   mosquitto_client& instance = *static_cast<mosquitto_client*>(client);
   instance.on_connect(rc);
}

void
mosquitto_client::disconnect_callback_dispatcher(
      mosquitto* mosq,
      void* client,
      int rc)
{
   mosquitto_client& instance = *static_cast<mosquitto_client*>(client);
   instance.on_disconnect(rc);
}

void
mosquitto_client::message_callback_dispatcher(
      mosquitto* mosq,
      void* client,
      const mosquitto_message* msg)
{
   mosquitto_client& instance = *static_cast<mosquitto_client*>(client);
   raw_message m =
   {
      msg->topic,
      msg->payload,
      static_cast<std::size_t>(msg->payloadlen),
      static_cast<qos_level>(msg->qos),
      msg->retain
   };
   instance.on_message(m);
}

void
mosquitto_client::subscribe_callback_dispatcher(
      mosquitto* mosq,
      void* client,
      int mid,
      int qos_count,
      const int* granted_qos)
{
   mosquitto_client& instance = *static_cast<mosquitto_client*>(client);
   instance.on_subscription_completed(mid);
}

void
mosquitto_client::log_callback_dispatcher(
      mosquitto* mosq,
      void* client,
      int level,
      const char* msg)
{
   std::cerr << "mosquito says: " << msg << std::endl;
}

void
mosquitto_client::on_connect(int rc)
{
   async_result result = { async_op::CONNECT, 0, rc };
   _async_result_chan << result;
}

void
mosquitto_client::on_disconnect(int rc)
{
   async_result result = { async_op::DISCONNECT, 0, rc };
   _async_result_chan << result;
}

void
mosquitto_client::on_subscription_completed(
      int mid)
{
   async_result result = { async_op::SUBSCRIBE, mid, 0 };
   _async_result_chan << result;
}

void
mosquitto_client::init()
{
   static bool was_init = false;

   if (!was_init)
   {
      mosquitto_lib_init();
   }

   if ((_mosq = mosquitto_new(nullptr, true, this)) == nullptr)
   {
      OAC_THROW_EXCEPTION(mosquitto_internal_error(
            "initializing mosquitto client",
            errno));
   }
}

void
mosquitto_client::connect(int keepalive)
{
   log_info(
         "Connecting to %s:%d with keep-alive %d seconds... ",
         _server_host, _server_port, keepalive);
   try
   {
      auto error = mosquitto_connect(
            _mosq, _server_host.c_str(), _server_port, keepalive);
      if (error != MOSQ_ERR_SUCCESS)
         OAC_THROW_EXCEPTION(
               connection_error(
                     _server_host, _server_port, make_error_code(error)));

      wait_for_async([](const async_result& r)
      {
         return r.op == async_op::CONNECT;
      });
      log_info("Connected to remote broker successfully");
   }
   catch (const connection_error& e)
   {
      log_error(
            "cannot connect to %s:%d: error code %d",
            _server_host, _server_port, e.get_error_code());
      disconnect();
      stop();
      destroy();
      throw;
   }
   catch (const thread::channel_timeout_error& e)
   {
      log_error("cannot connect to %s:%d: connection timed out",
            _server_host, _server_port);
      disconnect();
      stop();
      destroy();
      OAC_THROW_EXCEPTION(mosquitto_timeout_error("connect response", e));
   }
}

void
mosquitto_client::set_callbacks()
{
   /*
   mosquitto_log_callback_set(
         _mosq,
         mosquitto_client::log_callback_dispatcher);
   */
   mosquitto_connect_callback_set(
         _mosq,
         mosquitto_client::connect_callback_dispatcher);
   mosquitto_disconnect_callback_set(
         _mosq,
         mosquitto_client::disconnect_callback_dispatcher);
   mosquitto_message_callback_set(
         _mosq,
         mosquitto_client::message_callback_dispatcher);
   mosquitto_subscribe_callback_set(
         _mosq,
         mosquitto_client::subscribe_callback_dispatcher);
}

void
mosquitto_client::start()
{
   auto error = mosquitto_loop_start(_mosq);
   if (error != MOSQ_ERR_SUCCESS)
   {
      OAC_THROW_EXCEPTION(mosquitto_internal_error(
            "starting internal loop",
            make_error_code(error)));
   }
}

void
mosquitto_client::stop()
{
   auto error = mosquitto_loop_stop(_mosq, true);
   if (error != MOSQ_ERR_SUCCESS)
   {
      OAC_THROW_EXCEPTION(mosquitto_internal_error(
            "starting internal loop",
            make_error_code(error)));
   }
}

void
mosquitto_client::disconnect()
{
   log_info("disconnecting from %s:%d... ", _server_host, _server_port);
   if (!_mosq)
   {
      log_error("cannot disconnect from broker: mosquitto was not initialized");
      return;
   }

   auto error = mosquitto_disconnect(_mosq);
   if (error != MOSQ_ERR_SUCCESS)
   {
      log_error(
            "cannot disconnect from broker: error code %d",
            make_error_code(error));
      return;
   }

   try
   {
      wait_for_async([](const async_result& r)
      {
         return r.op == async_op::DISCONNECT;
      });
      log_info("disconnected sucessfully");
   }
   catch (const thread::channel_timeout_error&)
   {
      log_error("cannot disconnect from broker: disconnection timed out");
   }
}

void
mosquitto_client::destroy()
{
   if (_mosq)
   {
      mosquitto_destroy(_mosq);
   }
}

mosquitto_client::async_result
mosquitto_client::wait_for_async(
      std::function<bool(const async_result&)> pred)
{
   return _async_result_chan.read_for(DEFAULT_MSG_TIMEOUT, pred);
}

int
mosquitto_client::make_error_code(
      int mosq_error)
{
   return mosq_error * 100 + errno;
}

}}} // namespace oac::fv::mqtt
