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
 * You Must have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <cstdlib>

#include <liboac/network.h>

#include <boost/uuid/random_generator.hpp>
#include <liboac/filesystem.h>
#include <liboac/logging.h>

#include "server.h"
#include "core.h"
#include "fsuipc.h"
#include "subscription.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(FlightVarsServerTest)

struct let_test
{
   let_test()
   {
      // Comment in/out this line to enable/disable logging to stderr
      set_main_logger(make_logger(log_level::INFO, file_output_stream::STDERR));

      _io_service = std::make_shared<boost::asio::io_service>();

      // A random port between 1025 and 7025 ensures socket is not occupied
      // by a previous test
      _port = rand() % 7000 + 1025;

      _fsuipc = std::make_shared<dummy_fsuipc_flight_vars>();
      _server = flight_vars_server::create(_fsuipc, _port, _io_service);
      _server_thread = boost::thread([this]() {
         _io_service->run();
      });
   }

   ~let_test()
   {
      _server->io_service().stop();
      _server_thread.join();
   }

   let_test& connect()
   {
      _client = std::make_shared<tcp_client>(
            "localhost", _port);
      return *this;
   }

   let_test& send_garbage()
   {
      for (int i = 0; i < 12; i++)
         stream::write_as<int>(*_client->output(), rand());
      return *this;
   }

   let_test& handshake()
   {
      proto::message open_req = proto::begin_session_message("Test Client");
      send_message_as(open_req);

      auto open_rep = receive_message_as<proto::begin_session_message>();

      BOOST_CHECK_EQUAL(flight_vars_server::PEER_NAME, open_rep.pname);
      BOOST_CHECK_EQUAL(proto::CURRENT_PROTOCOL_VERSION, open_rep.proto_ver);

      return *this;
   }

   let_test& disconnect()
   {
      auto msg = proto::end_session_message("Client disconnected, bye!");
      send_message_as(msg);

      // Have to wait a little while to let the server process the message
      boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

      BOOST_CHECK(connection_is_closed());

      _client.reset();

      return *this;
   }

   let_test& check_remote_peer_disconects()
   {
      // Have to wait a little while to let the server process previous message
      boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

      BOOST_CHECK(connection_is_reset());
      return *this;
   }

   let_test& subscribe(
         const variable_group::tag_type& var_group_tag,
         const variable_name::tag_type& var_name_tag,
         bool expect_success = true)
   {
      auto var_group = variable_group(var_group_tag);
      auto var_name = variable_name(var_name_tag);
      auto var_id = make_var_id(var_group, var_name);

      auto req = proto::subscription_request_message(var_group, var_name);
      send_message_as(req);

      auto rep = receive_message_as<proto::subscription_reply_message>();

      if (expect_success)
      {
         _subscriptions[var_id] = rep.subs_id;
         BOOST_CHECK_EQUAL(
                  proto::subscription_status::SUCCESS,
                  rep.st);
      }
      else
         BOOST_CHECK_EQUAL(
               proto::subscription_status::NO_SUCH_VAR,
               rep.st);
      BOOST_CHECK_EQUAL(var_group.get_tag(), rep.var_grp.get_tag());
      BOOST_CHECK_EQUAL(var_name.get_tag(), rep.var_name.get_tag());
      return *this;
   }

   let_test& receive_var_update(
         const variable_group::tag_type& var_group_tag,
         const variable_name::tag_type& var_name_tag,
         const variable_value& value)
   {
      auto var_id = make_var_id(var_group_tag, var_name_tag);
      auto rep = receive_message_as<proto::var_update_message>();
      auto expected_subs_id = _subscriptions[var_id];

      BOOST_CHECK_EQUAL(expected_subs_id, rep.subs_id);
      BOOST_CHECK_EQUAL(value.get_type(), rep.var_value.get_type());
      switch (value.get_type())
      {
         case variable_type::BOOLEAN:
            BOOST_CHECK_EQUAL(value.as_bool(), rep.var_value.as_bool());
            break;
         case variable_type::BYTE:
            BOOST_CHECK_EQUAL(value.as_byte(), rep.var_value.as_byte());
            break;
         case variable_type::WORD:
            BOOST_CHECK_EQUAL(value.as_word(), rep.var_value.as_word());
            break;
         case variable_type::DWORD:
            BOOST_CHECK_EQUAL(value.as_dword(), rep.var_value.as_dword());
            break;
         case variable_type::FLOAT:
            BOOST_CHECK_EQUAL(value.as_float(), rep.var_value.as_float());
            break;
      }

      return *this;
   }

   let_test& on_offset_change(
         const fsuipc_offset_address& address,
         const fsuipc_offset_length& length,
         const fsuipc_offset_value& value)
   {
      _fsuipc->user_adapter().write_value_to_buffer(address, length, value);
      return *this;
   }

   let_test& assert_offset_value(
         const fsuipc_offset_address& address,
         const fsuipc_offset_length& length,
         const fsuipc_offset_value& value)
   {
      auto val = _fsuipc->user_adapter()
            .read_value_from_buffer(address, length);
      BOOST_CHECK_EQUAL(value, val);
      return *this;
   }

   let_test& fsuipc_polls_for_changes()
   {
      _io_service->dispatch(
            std::bind(&dummy_fsuipc_flight_vars::check_for_updates, _fsuipc));
      return *this;
   }

   let_test& send_var_update(
         const variable_group::tag_type& var_group_tag,
         const variable_name::tag_type& var_name_tag,
         const variable_value& value)
   {
      auto var_id = make_var_id(var_group_tag, var_name_tag);
      auto subs_id = _subscriptions[var_id];
      return send_var_update(subs_id, value);
   }

   let_test& send_var_update(
         const subscription_id& subs_id,
         const variable_value& value)
   {
      auto req = proto::var_update_message(subs_id, value);
      send_message_as(req);

      // Have to wait a little while to let the server process the message
      boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

      return *this;
   }

private:

   std::shared_ptr<boost::asio::io_service> _io_service;
   std::uint16_t _port;
   std::shared_ptr<dummy_fsuipc_flight_vars> _fsuipc;
   flight_vars_server::ptr_type _server;
   boost::thread _server_thread;
   std::shared_ptr<tcp_client> _client;
   std::unordered_map<
         variable_id,
         subscription_id,
         variable_id_hash> _subscriptions;

   proto::message receive_message()
   {
      return proto::deserialize<proto::binary_message_deserializer>(
            *_client->input());
   }

   template <typename MessageType>
   MessageType receive_message_as()
   {
      auto msg = receive_message();
      auto casted_msg = boost::get<MessageType>(&msg);
      BOOST_CHECK(casted_msg != nullptr);
      return *casted_msg;
   }

   template <typename MessageType>
   void send_message_as(const MessageType& msg)
   {
      proto::serialize<proto::binary_message_serializer>(
            msg, *_client->output());
   }

   bool connection_is_closed()
   {
     return one_byte_read_error_code() ==  boost::asio::error::eof;
   }

   bool connection_is_reset()
   {
      return one_byte_read_error_code() ==  boost::asio::error::connection_reset;
   }

   int one_byte_read_error_code()
   {
      auto& socket = _client->connection().socket();
      std::uint8_t byte;
      boost::system::error_code ec;

      boost::asio::read(
            socket,
            boost::asio::buffer(&byte, 1),
            ec);
      return ec.value();
   }

};

BOOST_AUTO_TEST_CASE(MustHandshake)
{
   let_test()
         .connect()
         .handshake()
         .disconnect();
}

BOOST_AUTO_TEST_CASE(MustDisconnectOnInvalidMessageReceivedBeforeHandshake)
{
   let_test()
         .connect()
         .send_garbage()
         .check_remote_peer_disconects();
}

BOOST_AUTO_TEST_CASE(MustDisconnectOnInvalidMessageReceivedAfterHandshake)
{
   let_test()
         .connect()
         .handshake()
         .send_garbage()
         .check_remote_peer_disconects();
}

BOOST_AUTO_TEST_CASE(MustRespondNormallyAfterDisconnected)
{
   let_test()
         .connect()
         .handshake()
         .send_garbage()
         .check_remote_peer_disconects()
         .connect()
         .handshake()
         .disconnect();
}

BOOST_AUTO_TEST_CASE(MustRespondSuccessToSubscriptionRequest)
{
   let_test()
         .connect()
         .handshake()
         .subscribe("fsuipc/offset", "0x700:4")
         .disconnect();
}

BOOST_AUTO_TEST_CASE(MustRespondErrorToWrongSubscriptionRequest)
{
   let_test()
         .connect()
         .handshake()
         .subscribe("unexisting/group", "unexisting/variable", false)
         .disconnect();
}

BOOST_AUTO_TEST_CASE(MustRespondSuccessToManySubscriptionRequests)
{
   let_test()
         .connect()
         .handshake()
         .subscribe("fsuipc/offset", "0x700:4")
         .subscribe("fsuipc/offset", "0x704:4")
         .subscribe("fsuipc/offset", "0x708:2")
         .subscribe("unexisting/group", "unexisting/variable", false)
         .subscribe("fsuipc/offset", "0x70a:2")
         .subscribe("fsuipc/offset", "0x70c:4")
         .disconnect();
}

BOOST_AUTO_TEST_CASE(MustNotifyVarUpdates)
{
   let_test()
         .connect()
         .handshake()
         .subscribe("fsuipc/offset", "0x700:4")
         .subscribe("fsuipc/offset", "0x800:1")
         .on_offset_change(0x700, OFFSET_LEN_DWORD, 0x0a0b0c0d)
         .fsuipc_polls_for_changes()
         .receive_var_update(
               "fsuipc/offset",
               "0x700:4",
               variable_value::from_dword(0x0a0b0c0d))
         .on_offset_change(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .on_offset_change(0x700, OFFSET_LEN_DWORD, 0x05060708)
         .fsuipc_polls_for_changes()
         .on_offset_change(0x800, OFFSET_LEN_BYTE, 0xab)
         .fsuipc_polls_for_changes()
         .receive_var_update(
               "fsuipc/offset",
               "0x700:4",
               variable_value::from_dword(0x05060708))
         .receive_var_update(
               "fsuipc/offset",
               "0x800:1",
               variable_value::from_byte(0xab))
         .disconnect();
}

BOOST_AUTO_TEST_CASE(MustAcceptVarUpdatesFromClient)
{
   let_test()
         .connect()
         .handshake()
         .on_offset_change(0x700, OFFSET_LEN_DWORD, 0x0a0b0c0d)
         .subscribe("fsuipc/offset", "0x700:4")
         .send_var_update(
               "fsuipc/offset",
               "0x700:4",
               variable_value::from_dword(0x01020304))
         .assert_offset_value(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .fsuipc_polls_for_changes()
         .receive_var_update(
               "fsuipc/offset",
               "0x700:4",
               variable_value::from_dword(0x01020304))
          // check server still responds
         .subscribe("fsuipc/offset", "0x800:1")
         .on_offset_change(0x800, OFFSET_LEN_BYTE, 0x4a)
         .fsuipc_polls_for_changes()
         .receive_var_update(
               "fsuipc/offset",
               "0x800:1",
               variable_value::from_byte(0x4a))
         .disconnect();
}

BOOST_AUTO_TEST_CASE(MustIgnoreVarUpdatesWithUnknownSubscriptionId)
{
   let_test()
         .connect()
         .handshake()
         .on_offset_change(0x700, OFFSET_LEN_DWORD, 0x0a0b0c0d)
         .subscribe("fsuipc/offset", "0x700:4")
         .send_var_update(
               1234567,
               variable_value::from_dword(0x01020304)) // ignored by server
         .assert_offset_value(0x700, OFFSET_LEN_DWORD, 0x0a0b0c0d)
          // check server still responds
         .subscribe("fsuipc/offset", "0x800:1")
         .on_offset_change(0x800, OFFSET_LEN_BYTE, 0x4a)
         .fsuipc_polls_for_changes()
         .receive_var_update(
               "fsuipc/offset",
               "0x800:1",
               variable_value::from_byte(0x4a))
         .disconnect();
}

BOOST_AUTO_TEST_SUITE_END()
