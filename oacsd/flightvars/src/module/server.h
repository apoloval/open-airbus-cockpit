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

#ifndef OAC_FV_SERVER_H
#define OAC_FV_SERVER_H

#include <list>
#include <memory>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <liboac/logging.h>
#include <liboac/network.h>

#include "api.h"
#include "protocol.h"
#include "subscription.h"

namespace oac { namespace fv {

class flight_vars_server :
      public std::enable_shared_from_this<flight_vars_server>,
      public logger_component
{
public:

   static const int DEFAULT_PORT;
   static const proto::peer_name PEER_NAME;

   flight_vars_server(
         const std::shared_ptr<flight_vars>& delegate = nullptr,
         int port = DEFAULT_PORT,
         const std::shared_ptr<boost::asio::io_service>& io_srv =
            std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service));

   ~flight_vars_server();

   const boost::asio::io_service& io_service() const
   { return _tcp_server.io_service(); }

   boost::asio::io_service& io_service()
   { return _tcp_server.io_service(); }

private:

   struct session : logger_component
   {
      typedef buffer::ring_buffer input_buffer_type;
      typedef input_buffer_type::ptr_type input_buffer_ptr;

      std::shared_ptr<flight_vars_server> server;
      subscription_mapper subscriptions;
      input_buffer_ptr input_buffer;
      network::async_tcp_connection_ptr conn;

      session(const std::shared_ptr<flight_vars_server>& srv,
              const network::async_tcp_connection_ptr& c)
         : logger_component("server-session"),
           server(srv),
           input_buffer(std::make_shared<input_buffer_type>(64*1024)),
           conn(c)
      {}

      ~session();

      void unsubscribe_all();
   };     

   friend struct session;

   typedef std::shared_ptr<session> session_ptr;
   typedef std::weak_ptr<session> session_wptr;

   typedef buffer::linear_buffer output_buffer_type;
   typedef output_buffer_type::ptr_type output_buffer_ptr;

   typedef std::function<void(void)> after_write_handler;

   std::shared_ptr<flight_vars> _delegate;
   network::async_tcp_server _tcp_server;

   void accept_connection(const network::async_tcp_connection_ptr& conn);

   void read_begin_session(
         const session_ptr& session);

   void on_read_begin_session(
         const session_ptr& session,
         const attempt<std::size_t>& bytes_transferred);

   void read_request(
         const session_ptr& session);

   void on_read_request(
         const session_ptr& session,
         const attempt<std::size_t>& bytes_transferred);

   proto::subscription_reply_message handle_subscription_request(
         const session_ptr& session,
         const proto::subscription_request_message& req);

   proto::unsubscription_reply_message handle_unsubscription_request(
         const session_ptr& session,
         const proto::unsubscription_request_message& req);

   void handle_var_update(
         const session_wptr& session,
         const variable_id& var_id,
         const variable_value& var_value);

   void send_var_update(
         const session_ptr& session,
         const variable_id& var_id,
         const variable_value& var_value);

   void write_message(
         const network::async_tcp_connection_ptr& conn,
         const proto::message& msg,
         const after_write_handler& after_write);

   void on_write_message(
         const output_buffer_ptr& buffer,
         const after_write_handler& after_write,
         const attempt<std::size_t>& bytes_transferred);

   void handle_var_update_request(
         const proto::var_update_message& req);
};

typedef std::shared_ptr<flight_vars_server> flight_vars_server_ptr;

}} // namespace oac::fv

#endif
