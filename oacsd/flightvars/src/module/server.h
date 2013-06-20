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
#include <liboac/network.h>

#include "protocol.h"

namespace oac { namespace fv {

class flight_vars;

class flight_vars_server :
      public shared_by_ptr<flight_vars_server>,
      public std::enable_shared_from_this<flight_vars_server>
{
public:

   static const int DEFAULT_PORT;
   static const proto::peer_name PEER_NAME;

   flight_vars_server(const ptr<flight_vars>& delegate = nullptr,
                      int port = DEFAULT_PORT);

   inline ~flight_vars_server()
   { _tcp_server.stop(); }

   inline void run()
   { _tcp_server.run(); }

   inline void run_in_background()
   { _tcp_server.run_in_background(); }

private:

   struct session : shared_by_ptr<session>
   {
      std::shared_ptr<flight_vars_server> server;
      std::list<flight_vars::subscription_id> subscriptions;
      ring_buffer::ptr_type input_buffer;
      ring_buffer::ptr_type output_buffer;
      async_tcp_connection::ptr_type conn;

      session(const std::shared_ptr<flight_vars_server>& srv,
              const async_tcp_connection::ptr_type& c)
         : server(srv),
           input_buffer(ring_buffer::create(64*1024)),
           output_buffer(ring_buffer::create(64*1024)),
           conn(c)
      {}

      ~session();
   };

   friend struct session;

   typedef std::function<void(void)> after_write_handler;

   ptr<flight_vars> _delegate;
   async_tcp_server _tcp_server;

   void accept_connection(const async_tcp_connection::ptr_type& conn);

   void read_begin_session(
         const session::ptr_type& session);

   void on_read_begin_session(
         const session::ptr_type& session,
         const boost::system::error_code& ec,
         std::size_t bytes_transferred);

   void read_request(
         const session::ptr_type& session);

   void on_read_request(
         const session::ptr_type& session,
         const boost::system::error_code& ec,
         std::size_t bytes_transferred);

   proto::subscription_reply_message handle_subscription_request(
         const session::ptr_type& session,
         const proto::subscription_request_message& req);

   void write_message(
         const session::ptr_type& session,
         const proto::message& msg,
         const after_write_handler& after_write);

   void on_write_message(
         const session::ptr_type& session,
         const after_write_handler& after_write,
         const boost::system::error_code& ec,
         std::size_t bytes_transferred);
};

}} // namespace oac::fv

#endif
