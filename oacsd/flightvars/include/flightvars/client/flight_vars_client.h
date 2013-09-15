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

#ifndef OAC_FV_CLIENT_FLIGHT_VARS_CLIENT_H
#define OAC_FV_CLIENT_FLIGHT_VARS_CLIENT_H

#include <list>
#include <unordered_map>

#include <liboac/logging.h>
#include <liboac/network.h>

#include "api.h"
#include "client/subscription_db.h"
#include "protocol.h"
#include "subscription.h"

namespace oac { namespace fv { namespace client {

/**
 * A FlightVars object that connects to a remote server to serve variables.
 * This class provides a FlightVars-like object that connects to a remote
 * server via TCP protocol.
 *
 * This implementation is not thread-safe. Invoking subscribe(), unsuscribe()
 * or update() from different threads will have a undetermined behaviour.
 */
class flight_vars_client : public flight_vars, public logger_component
{
public:

   /**
    * Exception thrown when a communication error occurs. Contains:
    *  - nested_error_info, indicating the nested IO error
    */
   OAC_DECL_EXCEPTION(
         communication_error,
         oac::exception,
         "something went wrong while communicating with FlightVars server");

   // A handler for errors occurred in asynchronous operations.
   typedef std::function<void(const communication_error&)> error_handler;

   flight_vars_client(
         const std::string& client_name,
         const network::hostname& server_host,
         network::tcp_port server_port,
         const error_handler& ehandler =
               error_handler(),
         const boost::chrono::seconds& request_timeout =
               boost::chrono::seconds(60))
   throw (communication_error);

   virtual ~flight_vars_client();

   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler)
   throw (no_such_variable_error);

   virtual void unsubscribe(
         const subscription_id& id)
   throw (no_such_subscription_error);

   virtual void update(
         const subscription_id& subs_id,
         const variable_value& var_value)
   throw (no_such_subscription_error, illegal_value_error);

private:

   struct subscription_request
   {
      variable_id var_id;
      flight_vars::var_update_handler handler;
      boost::promise<subscription_id> promise;

      subscription_request(
            const variable_id& var_id,
            const flight_vars::var_update_handler& handler)
         : var_id(var_id),
           handler(handler)
      {}
   };

   struct unsubscription_request
   {
      subscription_id master_subs_id;
      subscription_id slave_subs_id;
      boost::promise<void> promise;

      unsubscription_request(
            subscription_id subs_id)
         : slave_subs_id(subs_id)
      {}
   };

   error_handler _error_handler;
   std::shared_ptr<boost::asio::io_service> _io_service;
   boost::chrono::seconds _request_timeout;
   async_tcp_client _client;
   ring_buffer _input_buffer;
   boost::thread _client_thread;
   subscription_db _db;

   std::unique_ptr<subscription_request> _subscription_request;
   std::unique_ptr<unsubscription_request> _unsubscription_request;

   void handshake(
         const std::string& client_name)
   throw (communication_error);

   void close() throw (communication_error);

   void start_receive();

   void run_io_service_thread();

   void stop_io_service_thread();

   void on_subscription_requested();

   void on_unsubscription_requested();

   void on_message_received(
         const boost::system::error_code& ec,
         std::size_t bytes_read);

   void on_subscription_reply_received(
         const proto::subscription_reply_message& msg);

   void on_unsubscription_reply_received(
         const proto::unsubscription_reply_message& msg);

   void on_variable_update_received(
         const proto::var_update_message& msg);

   void send_data(
         const std::shared_ptr<linear_buffer>& output_buff);

   void on_data_sent(
         const boost::system::error_code& ec,
         std::size_t bytes_read);
};

}}} // namespace oac::fv::client

#endif
