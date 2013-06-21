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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <liboac/network.h>

#include <boost/uuid/random_generator.hpp>
#include <liboac/filesystem.h>
#include <liboac/logging.h>

#include "server.h"
#include "core.h"
#include "subscription.h"

using namespace oac;
using namespace oac::fv;

struct setup_log
{
   inline setup_log()
   {
      set_main_logger(make_logger(log_level::INFO, file_output_stream::STDERR));
   }
};

struct fake_flight_vars : public flight_vars
{
   static variable_group VAR_GROUP;

   virtual subscription_id subscribe(
         const variable_group& grp,
         const variable_name& name,
         const var_update_handler& handler) throw (unknown_variable_error)
   {
      if (grp != VAR_GROUP)
         BOOST_THROW_EXCEPTION(unknown_variable_group_error());
      return make_subscription_id();
   }

   virtual void unsubscribe(const subscription_id& id)
   {
   }
};

variable_group fake_flight_vars::VAR_GROUP("testing");

proto::begin_session_message handshake(tcp_client& cli)
{
   proto::message cli_msg = proto::begin_session_message("Test Client");
   proto::serialize<proto::binary_message_serializer>(
            cli_msg, *cli.output());
   auto srv_msg = proto::deserialize<proto::binary_message_deserializer>(
            *cli.input());
   return boost::get<proto::begin_session_message>(srv_msg);
}

void terminate(tcp_client& cli)
{
   auto msg = proto::end_session_message("Client disconnected, bye!");
   proto::serialize<proto::binary_message_serializer>(
            msg, *cli.output());
}

proto::subscription_reply_message request_subscription(
      tcp_client& cli,
      const variable_group& var_grp,
      const variable_name& var_name)
{
   auto req = proto::subscription_request_message(var_grp, var_name);
   proto::serialize<proto::binary_message_serializer>(req, *cli.output());
   auto rep = proto::deserialize<proto::binary_message_deserializer>(
            *cli.input());
   return boost::get<proto::subscription_reply_message>(rep);
}

BOOST_FIXTURE_TEST_SUITE(FlightVarsServerTest, setup_log);

BOOST_AUTO_TEST_CASE(ServerShouldHandshake)
{   
   ptr<flight_vars_server> server = new flight_vars_server();
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      BOOST_CHECK_EQUAL(flight_vars_server::PEER_NAME, bs_msg.pname);
      BOOST_CHECK_EQUAL(proto::CURRENT_PROTOCOL_VERSION, bs_msg.proto_ver);
      terminate(cli);
   }
}

BOOST_AUTO_TEST_CASE(ServerShouldRespondSuccessToSubscriptionRequest)
{
   ptr<flight_vars_server> server = new flight_vars_server(
            new fake_flight_vars());
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      auto rep = request_subscription(
               cli,
               variable_group("testing"),
               variable_name("foobar"));
      BOOST_CHECK_EQUAL(
               proto::subscription_reply_message::STATUS_SUCCESS,
               rep.st);
      BOOST_CHECK_EQUAL("testing", rep.var_grp.get_tag());
      BOOST_CHECK_EQUAL("foobar", rep.var_name.get_tag());
      terminate(cli);
   }
}

BOOST_AUTO_TEST_CASE(ServerShouldRespondErrorToWrongSubscriptionRequest)
{
   ptr<flight_vars_server> server = new flight_vars_server(
            new fake_flight_vars());
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      auto rep = request_subscription(
               cli,
               variable_group("weezer"),
               variable_name("foobar"));
      BOOST_CHECK_EQUAL(
               proto::subscription_reply_message::STATUS_NO_SUCH_VAR,
               rep.st);
      BOOST_CHECK_EQUAL("weezer", rep.var_grp.get_tag());
      BOOST_CHECK_EQUAL("foobar", rep.var_name.get_tag());
      terminate(cli);
   }
}

BOOST_AUTO_TEST_CASE(ServerShouldRespondSuccessToManySubscriptionRequests)
{
   ptr<flight_vars_server> server = new flight_vars_server(
            new fake_flight_vars());
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      for (int i = 0; i < 8192; i++)
      {
         auto var_name = str(boost::format("foobar-%d") % i);
         auto rep = request_subscription(
                  cli,
                  variable_group("testing"),
                  variable_name(var_name));
         BOOST_CHECK_EQUAL(
                  proto::subscription_reply_message::STATUS_SUCCESS,
                  rep.st);
         BOOST_CHECK_EQUAL("testing", rep.var_grp.get_tag());
         BOOST_CHECK_EQUAL(var_name, rep.var_name.get_tag());
      }
      terminate(cli);
   }
}



BOOST_AUTO_TEST_SUITE_END()
