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
#include "fake.h"
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

void request_subscription(
      tcp_client& cli,
      const variable_group& var_grp,
      const variable_name& var_name)
{
   auto req = proto::subscription_request_message(var_grp, var_name);
   proto::serialize<proto::binary_message_serializer>(req, *cli.output());
}

proto::message
read_message(
      tcp_client& cli)
{
   return proto::deserialize<proto::binary_message_deserializer>(*cli.input());
}

proto::subscription_reply_message
as_subscription_reply(
      proto::message& rep)
{
   return boost::get<proto::subscription_reply_message>(rep);
}

proto::var_update_message
as_var_update(
      proto::message& rep)
{
   return boost::get<proto::var_update_message>(rep);
}

proto::subscription_reply_message
read_subscription_reply(
      tcp_client& cli)
{
   while (true)
   {
      auto msg = read_message(cli);
      if (auto rep = boost::get<proto::subscription_reply_message>(&msg))
         return *rep;
   }
}

BOOST_FIXTURE_TEST_SUITE(FlightVarsServerTest, setup_log);

BOOST_AUTO_TEST_CASE(ServerShouldHandshake)
{   
   auto server = flight_vars_server::create();
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
   auto server = flight_vars_server::create(
            std::make_shared<fake_flight_vars>());
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      request_subscription(
               cli,
               variable_group("testing"),
               variable_name("foobar"));
      auto rep = read_subscription_reply(cli);
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
   auto server = flight_vars_server::create(
            std::make_shared<fake_flight_vars>());
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      request_subscription(
               cli,
               variable_group("weezer"),
               variable_name("foobar"));
      auto rep = read_subscription_reply(cli);
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
   auto server = flight_vars_server::create(
            std::make_shared<fake_flight_vars>());
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      for (int i = 0; i < 8192; i++)
      {
         auto var_name = str(boost::format("foobar-%d") % i);
         request_subscription(
                  cli,
                  variable_group("testing"),
                  variable_name(var_name));
         auto rep = read_subscription_reply(cli);
         BOOST_CHECK_EQUAL(
                  proto::subscription_reply_message::STATUS_SUCCESS,
                  rep.st);
         BOOST_CHECK_EQUAL("testing", rep.var_grp.get_tag());
         BOOST_CHECK_EQUAL(var_name, rep.var_name.get_tag());
      }
      terminate(cli);
   }
}

BOOST_AUTO_TEST_CASE(ServerShouldNotifyVarUpdates)
{
   auto server = flight_vars_server::create(
            std::make_shared<fake_flight_vars>());
   server->run_in_background();

   {
      tcp_client cli("localhost", flight_vars_server::DEFAULT_PORT);
      auto bs_msg = handshake(cli);
      request_subscription(
               cli,
               variable_group("testing"),
               variable_name("foobar"));

      auto subs_id = as_subscription_reply(read_message(cli)).subs_id;
      for (int i = 0; i < 3; i++)
      {
         auto rep = as_var_update(read_message(cli));
         BOOST_CHECK_EQUAL(subs_id, rep.subs_id);
         BOOST_CHECK_EQUAL(VAR_DWORD, rep.var_value.get_type());
      }
      terminate(cli);
   }
}

BOOST_AUTO_TEST_SUITE_END()
