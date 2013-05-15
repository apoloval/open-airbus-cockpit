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

   typedef std::list<flight_vars::subscription_id> subscription_id_list;

   void handle_connection(const ptr<tcp_connection>& conn);

   void handle_handshake(const ptr<tcp_connection>& conn);

   proto::subscription_reply_message handle_subscription_request(
         const ptr<tcp_connection>& conn,
         const proto::subscription_request_message& req,
         subscription_id_list& subscriptions);

   void remove_subscriptions(subscription_id_list& subscriptions);

   proto::message read_message(const ptr<tcp_connection>& conn);

   void write_message(
         const ptr<tcp_connection>& conn,
         const proto::message& msg);

   ptr<flight_vars> _delegate;
   tcp_server<network::dedicated_thread_connection_handler> _tcp_server;

};

}} // namespace oac::fv

#endif
