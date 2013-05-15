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

#include "server.h"

#include <liboac/logging.h>

#include "core.h"

namespace oac { namespace fv {

const int flight_vars_server::DEFAULT_PORT(8642);
const proto::peer_name flight_vars_server::PEER_NAME("FlightVars Server");

flight_vars_server::flight_vars_server(
      const ptr<flight_vars>& delegate, int port)
   : _delegate(delegate),
     _tcp_server(
           port, network::dedicated_thread_connection_handler(
                 std::bind(
                    &flight_vars_server::handle_connection,
                    this,
                    std::placeholders::_1)))
{
   log(log_level::INFO,
       boost::format("@server; Initialized on port %d") % port);
   if (!_delegate)
      _delegate = flight_vars_core::instance();
}

void
flight_vars_server::handle_connection(const ptr<tcp_connection>& conn)
{
   using namespace proto;

   subscription_id_list subscriptions;
   try
   {
      handle_handshake(conn);

      while (true)
      {
         auto msg = read_message(conn);
         if (auto es_msg = boost::get<proto::end_session_message>(&msg))
         {
            log(log_level::INFO,
                boost::format("@server; Session closed by peer (%s)") %
                es_msg->cause);
            break;
         } else if (auto s_req = boost::get<subscription_request_message>(&msg))
         {
            auto rep = handle_subscription_request(
                     conn, *s_req, subscriptions);
            write_message(conn, rep);
         } else {
            BOOST_THROW_EXCEPTION(proto::protocol_error() <<
                  proto::expected_input_info(
                        "an end session, supscription request or "
                        "variable update message") <<
                  proto::actual_input_info("an unexpected message"));
         }
      }
   } catch (proto::protocol_error& e)
   {
      log(log_level::WARN,
          boost::format("@server; Protocol error: %s") % e.what());
   } catch (error& e)
   {
      log(log_level::WARN,
          boost::format("@server; Unexpected error: %s") % e.what());
   }
   remove_subscriptions(subscriptions);
}

void
flight_vars_server::handle_handshake(const ptr<tcp_connection>& conn)
{
   using namespace proto;

   auto msg = read_message(conn);
   if (auto* cli_msg = boost::get<begin_session_message>(&msg))
   {
      log(log_level::INFO,
          boost::format("@server; New client %s with protocol %d.%d") %
               cli_msg->pname %
               (cli_msg->proto_ver >> 8) %
               (cli_msg->proto_ver & 0x00ff));
      msg = begin_session_message(PEER_NAME);
      serialize<binary_message_serializer>(msg, *conn->output());
   } else
   {
      BOOST_THROW_EXCEPTION(protocol_error() <<
            expected_input_info("a begin session message") <<
            actual_input_info("an unexpected message"));
   }
}

proto::subscription_reply_message
flight_vars_server::handle_subscription_request(
      const ptr<tcp_connection>& conn,
      const proto::subscription_request_message& req,
      subscription_id_list& subscriptions)
{
   try
   {
      auto subs_id = _delegate->subscribe(
            req.var_grp,
            req.var_name,
            [this, conn](const variable_group& grp,
                         const variable_name& name,
                         const variable_value& value)
            {
               // TODO: write var update message to the client
            });
      subscriptions.push_back(subs_id);
   } catch (flight_vars::unknown_variable_error&)
   {
      return proto::subscription_reply_message(
            proto::subscription_reply_message::STATUS_NO_SUCH_VAR,
            req.var_grp,
            req.var_name,
            "No such variable defined in FlightVars module; missing plugin?");
   }
   return proto::subscription_reply_message(
         proto::subscription_reply_message::STATUS_SUCCESS,
         req.var_grp,
         req.var_name,
         "");

}

void
flight_vars_server::remove_subscriptions(
      subscription_id_list& subscriptions)
{
   for (auto& s : subscriptions)
      _delegate->unsubscribe(s);
   subscriptions.clear();
}

proto::message
flight_vars_server::read_message(const ptr<tcp_connection>& conn)
{
   return proto::deserialize<proto::binary_message_deserializer>(
            *conn->input());
}

void
flight_vars_server::write_message(
      const ptr<tcp_connection>& conn,
      const proto::message& msg)
{
   proto::serialize<proto::binary_message_serializer>(msg, *conn->output());
}

}} // namespace oac::fv
