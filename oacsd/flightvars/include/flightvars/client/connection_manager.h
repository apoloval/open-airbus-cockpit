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

#ifndef OAC_FV_CLIENT_CONNECTION_MANAGER_H
#define OAC_FV_CLIENT_CONNECTION_MANAGER_H

#include <liboac/logging.h>
#include <liboac/network.h>

#include "api.h"
#include "client/errors.h"
#include "client/requests.h"
#include "client/subscription_db.h"
#include "protocol.h"
#include "subscription.h"

namespace oac { namespace fv { namespace client {

class connection_manager : public logger_component
{
public:

   // A handler for errors occurred in asynchronous operations.
   typedef std::function<void(const communication_error&)> error_handler;

   connection_manager(
         const std::string& client_name,
         const network::hostname& server_host,
         network::tcp_port server_port,
         const error_handler& ehandler = error_handler())
   throw (communication_error);

   virtual ~connection_manager();

   /**
    * Submit a subscription request to this manager.
    */
   void submit(const subscription_request_ptr& req);

   /**
    * Submit an unsubscription request to this manager.
    */
   void submit(const unsubscription_request_ptr& req);

   /**
    * Submit a variable update request to this manager.
    */
   void submit(const variable_update_request_ptr& req);

private:

   error_handler _error_handler;
   std::shared_ptr<boost::asio::io_service> _io_service;
   async_tcp_client _client;
   ring_buffer _input_buffer;
   std::thread _client_thread;
   subscription_db _db;
   request_pool _request_pool;

   void handshake(
         const std::string& client_name)
   throw (communication_error);

   void close() throw (communication_error);

   void start_receive();

   void run_io_service_thread();

   void stop_io_service_thread();

   void on_subscription_requested(
         const subscription_request_ptr& req);

   void on_unsubscription_requested(
         const unsubscription_request_ptr& req);

   void on_variable_update_requested(
         const variable_update_request_ptr& req);

   void on_message_received(
         const boost::system::error_code& ec,
         std::size_t bytes_read);

   void on_subscription_reply_received(
         const proto::subscription_reply_message& msg);

   void on_unsubscription_reply_received(
         const proto::unsubscription_reply_message& msg);

   void on_variable_update_received(
         const proto::var_update_message& msg);

   template <typename Message>
   void send_message(const Message& msg);

   void send_data(
         const std::shared_ptr<linear_buffer>& output_buff);                  

   void on_data_sent(
         const std::shared_ptr<linear_buffer>& output_buff,
         const boost::system::error_code& ec,
         std::size_t bytes_read);
};

}}} // namespace oac::fv::client

#endif
