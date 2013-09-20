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
#include <unordered_map>

#include <liboac/filesystem.h>

#include "client.h"
#include "protocol.h"
#include "subscription.h"

using namespace oac;
using namespace oac::fv;
using namespace oac::fv::client;

BOOST_AUTO_TEST_SUITE(FlightVarsClientTest)

struct let_test
{
   let_test()
      : _io_srv(std::make_shared<boost::asio::io_service>()),
        _srv_input_buff(1024)
   {
      // Comment in/out this line to enable/disable logging to stderr
      set_main_logger(make_logger(log_level::INFO, file_output_stream::STDERR));
   }

   ~let_test()
   {
      close();
   }

   let_test& prepare_server_for_handshake()
   {
      _current_srv_action = std::bind(
            &let_test::server_handshake,
            this,
            std::placeholders::_1);
      return *this;
   }

   let_test& prepare_server_for_close()
   {
      _current_srv_action = std::bind(
            &let_test::server_receive_close,
            this,
            std::placeholders::_1);
      return *this;
   }

   let_test& prepare_server_for_subscription(
         const variable_group::tag_type& var_group,
         const variable_name::tag_type& var_name,
         subscription_id subs_id = make_subscription_id())
   {
      _current_srv_action = std::bind(
            &let_test::server_subscribe,
            this,
            make_var_id(var_group, var_name),
            subs_id,
            std::placeholders::_1);
      return *this;
   }

   let_test& prepare_server_for_unsubscription(
         subscription_id expected_subs_id)
   {
      return prepare_server_for_unsubscription(
            expected_subs_id, expected_subs_id);
   }

   let_test& prepare_server_for_unsubscription(
         subscription_id expected_subs_id,
         subscription_id actual_subs_id)
   {
      _current_srv_action = std::bind(
            &let_test::server_unsubscribe,
            this,
            expected_subs_id,
            actual_subs_id,
            std::placeholders::_1);
      return *this;
   }

   let_test& prepare_server_for_var_update(
         subscription_id expected_subs_id,
         const variable_value& expected_var_value)
   {
      _current_srv_action = std::bind(
            &let_test::server_receive_var_update,
            this,
            expected_subs_id,
            expected_var_value,
            std::placeholders::_1);
      return *this;
   }

   let_test& prepare_server_to_close_on_next_request()
   {
      _current_srv_action = server_action();
      return *this;
   }

   let_test& prepare_server_to_send_garbage_on_next_request()
   {
      _current_srv_action = std::bind(
            &let_test::server_send_garbage,
            this,
            std::placeholders::_1);
      return *this;
   }

   template <typename MessageType>
   let_test& prepare_server_to_respond_with(
         const MessageType& msg)
   {
      _current_srv_action = std::bind(
            &let_test::server_write_message<MessageType>,
            this,
            std::placeholders::_1,
            msg,
            true);
      return *this;
   }

   let_test& connect()
   {
      auto port = rand() * 8000 + 1025;
      _server.reset(new network::async_tcp_server(
            port,
            std::bind(
                  &let_test::server_read_message,
                  this,
                  std::placeholders::_1),
            _io_srv,
            std::bind(
                  &let_test::on_server_error,
                  this,
                  std::placeholders::_1)));

      _server_thread = boost::thread([this]() {
         try
         { _io_srv->run(); }
         catch (oac::exception& e)
         {
            std::cerr <<
                  "Unexpected exception thrown in server thread" <<
                  std::endl <<
                  e.report() <<
                  std::endl;
         }
      });

      _client.reset(new flight_vars_client(
            "it-client",
            "localhost",
            port,
            flight_vars_client::error_handler(),
            std::chrono::seconds(1)));

      return *this;
   }

   let_test& subscribe(
         const variable_group::tag_type& grp,
         const variable_name::tag_type& name,
         int reception_id = 0)
   {
      auto var_id = make_var_id(grp, name);
      auto subs_id = _client->subscribe(
            var_id,
            std::bind(
                  &let_test::client_receive_var_update,
                  this,
                  reception_id,
                  std::placeholders::_1,
                  std::placeholders::_2));
      _subscriptions[var_id] = subs_id;
      return *this;
   }

   let_test& unsubscribe(
         const variable_group::tag_type& grp,
         const variable_name::tag_type& name)
   {
      auto var_id = make_var_id(grp, name);
      auto subs_id = _subscriptions[var_id];
      return unsubscribe(subs_id);
   }

   let_test& unsubscribe(
         subscription_id subs_id)
   {
      _client->unsubscribe(subs_id);
      return *this;
   }

   let_test& update(
         const variable_group::tag_type& grp,
         const variable_name::tag_type& name,
         const variable_value& value)
   {
      auto var_id = make_var_id(grp, name);
      auto subs_id = _subscriptions[var_id];
      return update(subs_id, value);
   }

   let_test& update(
         subscription_id subs_id,
         const variable_value& value)
   {
      _client->update(subs_id, value);
      sleep(100);
      return *this;
   }

   let_test& server_sends_var_update(
         subscription_id subs_id,
         const variable_value& var_value)
   {
      proto::var_update_message msg(subs_id, var_value);
      server_write_message(_server_conn.lock(), msg, false);
      return *this;
   }

   let_test& close()
   {
      _client.reset();

      _io_srv->stop();
      if (_server_thread.joinable())
         _server_thread.join();
      return *this;
   }

   let_test& check_var_update_reception(
         int reception_id,
         const variable_group::tag_type& var_grp,
         const variable_name::tag_type& var_name,
         const variable_value& var_value)
   {
      sleep(100); // let time for the server to reply

      auto& recp = _var_update_receptions[reception_id];

      BOOST_CHECK_EQUAL(var_grp, get_var_group(recp->var_id).get_tag());
      BOOST_CHECK_EQUAL(var_name, get_var_name(recp->var_id).get_tag());
      BOOST_CHECK(var_value == recp->var_value);
      return *this;
   }

   let_test& check_var_update_unreceived(
         int reception_id)
   {
      sleep(100); // let time for the server to reply

      BOOST_CHECK(_var_update_receptions[reception_id] == nullptr);
      return *this;
   }

private:

   typedef std::function<
         void(const network::async_tcp_connection_ptr&)> server_action;

   struct var_update_reception
   {
      variable_id var_id;
      variable_value var_value;

      var_update_reception(
            const variable_id& id,
            const variable_value& value)
         : var_id(id),
           var_value(value)
      {}
   };

   std::shared_ptr<boost::asio::io_service> _io_srv;
   std::unique_ptr<flight_vars_client> _client;
   std::unique_ptr<network::async_tcp_server> _server;
   std::weak_ptr<network::async_tcp_connection> _server_conn;
   boost::thread _server_thread;
   buffer::ring_buffer _srv_input_buff;
   std::unique_ptr<buffer::linear_buffer> _srv_output_buff;
   server_action _current_srv_action;
   std::unordered_map<
         variable_id,
         subscription_id,
         variable_id_hash> _subscriptions;

   std::unordered_map<
         int,
         std::unique_ptr<var_update_reception>> _var_update_receptions;

   void on_server_error(const io_exception&)
   {

   }

   void server_read_message(const network::async_tcp_connection_ptr& conn)
   {
      _server_conn = conn;
      conn->read(
            _srv_input_buff,
            std::bind(
                  &let_test::on_server_message_read,
                  this,
                  conn,
                  std::placeholders::_1));
   }

   void on_server_message_read(
         const network::async_tcp_connection_ptr& conn,
         const attempt<std::size_t>& bytes_read)
   {
      if (_current_srv_action)
         _current_srv_action(conn);
   }

   void server_handshake(
         const network::async_tcp_connection_ptr& conn)
   {
      auto req = server_receive_message_as<proto::begin_session_message>();
      BOOST_CHECK_EQUAL("it-client", req.pname);
      BOOST_CHECK_EQUAL(proto::CURRENT_PROTOCOL_VERSION, req.proto_ver);

      auto rep = proto::begin_session_message("it-server");
      server_write_message(conn, rep);
   }

   void server_receive_close(const network::async_tcp_connection_ptr& conn)
   {
      auto req = server_receive_message_as<proto::end_session_message>();
      BOOST_CHECK_EQUAL("Client disconnected", req.cause);
   }

   void server_subscribe(
         const variable_id& var_id,
         subscription_id subs_id,
         const network::async_tcp_connection_ptr& conn)
   {
      auto req = server_receive_message_as<
            proto::subscription_request_message>();
      auto var_grp = get_var_group(var_id);
      auto var_name = get_var_name(var_id);

      if (var_grp.get_tag() == "foobar")
         server_write_message(
               conn,
               proto::subscription_reply_message(
                     proto::subscription_status::SUBSCRIBED,
                     var_grp,
                     var_name,
                     subs_id,
                     "Variable found"));
      else
         server_write_message(
               conn,
               proto::subscription_reply_message(
                     proto::subscription_status::NO_SUCH_VAR,
                     var_grp,
                     var_name,
                     0,
                     "No such variable found in server"));
   }

   void server_unsubscribe(
         subscription_id expected_subs_id,
         subscription_id reply_subs_id,
         const network::async_tcp_connection_ptr& conn)
   {
      auto req = server_receive_message_as<
            proto::unsubscription_request_message>();
      if (req.subs_id == expected_subs_id)
         server_write_message(
               conn,
               proto::unsubscription_reply_message(
                     proto::subscription_status::UNSUBSCRIBED,
                     reply_subs_id,
                     ""));
      else
         server_write_message(
               conn,
               proto::unsubscription_reply_message(
                     proto::subscription_status::NO_SUCH_SUBSCRIPTION,
                     0,
                     "No such subscription found in server"));
   }

   void server_receive_var_update(
         subscription_id expected_subs_id,
         const variable_value& expected_var_value,
         const network::async_tcp_connection_ptr& conn)
   {
      auto req = server_receive_message_as<
            proto::var_update_message>();
      BOOST_REQUIRE_EQUAL(expected_subs_id, req.subs_id);
      BOOST_REQUIRE_EQUAL(expected_var_value, req.var_value);
   }

   void server_send_garbage(
         const network::async_tcp_connection_ptr& conn)
   {
      _srv_output_buff.reset(new buffer::linear_buffer(1024));
      for (int i = 0; i < 8; i++)
         stream::write_as<std::uint32_t>(*_srv_output_buff, rand());
      conn->write(
            *_srv_output_buff,
            std::bind(
                  &let_test::server_read_message,
                  this,
                  conn));
   }

   proto::message server_receive_message()
   {
      return proto::deserialize<proto::binary_message_deserializer>(
            _srv_input_buff);
   }

   template <typename MessageType>
   MessageType server_receive_message_as()
   {
      auto msg = server_receive_message();
      auto casted_msg = boost::get<MessageType>(&msg);
      BOOST_CHECK(casted_msg);
      return *casted_msg;
   }

   template <typename MessageType>
   void server_write_message(
         const network::async_tcp_connection_ptr& conn,
         const MessageType& msg,
         bool request_read = true)
   {
      _srv_output_buff.reset(new buffer::linear_buffer(1024));
      proto::serialize<proto::binary_message_serializer>(
            msg,
            *_srv_output_buff);
      if (request_read)
         conn->write(
               *_srv_output_buff,
               std::bind(
                     &let_test::server_read_message,
                     this,
                     conn));
      else
         conn->write(
               *_srv_output_buff,
               [](const attempt<std::size_t>&){});
   }

   void client_receive_var_update(
         int reception_id,
         const variable_id& id,
         const variable_value& value)
   {
      _var_update_receptions[reception_id].reset(
            new var_update_reception(id, value));
   }

   void sleep(unsigned int millis)
   {
      boost::this_thread::sleep_for(boost::chrono::milliseconds(millis));
   }
};

BOOST_AUTO_TEST_CASE(MustHandshake)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrowWhenServerClosesBeforeHandshake)
{
   BOOST_CHECK_THROW(
         let_test()
            .prepare_server_to_close_on_next_request()
            .connect()
            .close(),
         communication_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenServerRespondsWithGarbageToHandshake)
{
   BOOST_CHECK_THROW(
         let_test()
            .prepare_server_to_send_garbage_on_next_request()
            .connect()
            .close(),
         communication_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenServerRespondsWithWrongMessageToHandshake)
{
   BOOST_CHECK_THROW(
         let_test()
            .prepare_server_to_respond_with(
                  proto::subscription_reply_message(
                        proto::subscription_status::SUBSCRIBED,
                        variable_group("foo"),
                        variable_name("bar"),
                        1,
                        "Bad luck!"))
            .connect()
            .close(),
         communication_error);
}

BOOST_AUTO_TEST_CASE(MustSubscribeToVariable)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum")
      .subscribe("foobar", "datum")
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustSubscribeToDifferentVariables)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum1")
      .subscribe("foobar", "datum1")
      .prepare_server_for_subscription("foobar", "datum2")
      .subscribe("foobar", "datum2")
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustSubscribeToSameVariableSeveralTimes)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum1")
      .subscribe("foobar", "datum1")
      .subscribe("foobar", "datum1")
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrowOnSubscriptionToUnknownVariable)
{
   let_test test;
   BOOST_CHECK_THROW(
      test
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("kartoffen", "datum")
         .subscribe("kartoffen", "datum"),
      flight_vars::no_such_variable_error);
   test
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrowOnSubscriptionReplyWithRepeatedSubscriptionId)
{
   let_test test;
   BOOST_CHECK_THROW(
      test
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum1", 1)
         .subscribe("foobar", "datum1")
         .prepare_server_for_subscription("foobar", "datum2", 1)
         .subscribe("foobar", "datum2"),
      communication_error);
   test
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrowOnSubscriptionReplyWithRepeatedVariable)
{
   let_test test;
   BOOST_CHECK_THROW(
      test
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum1", 1)
         .subscribe("foobar", "datum1")
         .prepare_server_for_subscription("foobar", "datum1", 2)
         .subscribe("foobar", "datum2"),
      communication_error);
   test
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrownOnSubscriptionAndServerCloses)
{
   BOOST_CHECK_THROW(
      let_test()
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_to_close_on_next_request()
         .subscribe("foobar", "datum"),
      communication_error);
}

BOOST_AUTO_TEST_CASE(MustThrownOnSubscriptionAndReceivesGarbage)
{
   BOOST_CHECK_THROW(
      let_test()
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_to_send_garbage_on_next_request()
         .subscribe("foobar", "datum"),
      communication_error);
}

BOOST_AUTO_TEST_CASE(MustThrownOnSubscriptionAndReceivesUnexpectedMessage)
{
   BOOST_CHECK_THROW(
      let_test()
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_to_respond_with(
               proto::begin_session_message("Bad luck!"))
         .subscribe("foobar", "datum"),
      communication_error);
}

BOOST_AUTO_TEST_CASE(MustUnsubscribeFromVariable)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum", 700)
      .subscribe("foobar", "datum")
      .prepare_server_for_unsubscription(700)
      .unsubscribe("foobar", "datum")
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustUnsubscribeSlaveButNotMaster)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum", 800)
      .subscribe("foobar", "datum")
      .subscribe("foobar", "datum")
      .unsubscribe("foobar", "datum")
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrowOnUnsubscriptionFromUnknownId)
{
   let_test test;
   BOOST_CHECK_THROW(
      test
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum", 700)
         .subscribe("foobar", "datum")
         .prepare_server_for_unsubscription(700)
         .unsubscribe(1400),
      flight_vars::no_such_subscription_error);
   test
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrowOnUnsubscriptionReplyWithUnknownSubscription)
{
   let_test test;
   BOOST_CHECK_THROW(
      test
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum", 1001)
         .subscribe("foobar", "datum")
         .prepare_server_for_unsubscription(1001, 700)
         .unsubscribe("foobar", "datum"),
      communication_error);
   test
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrownOnUnsubscriptionAndServerCloses)
{
   BOOST_CHECK_THROW(
      let_test()
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum")
         .subscribe("foobar", "datum")
         .prepare_server_to_close_on_next_request()
         .unsubscribe("foobar", "datum"),
      communication_error);
}

BOOST_AUTO_TEST_CASE(MustThrownOnUnsubscriptionAndReceivesGarbage)
{
   BOOST_CHECK_THROW(
      let_test()
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum")
         .subscribe("foobar", "datum")
         .prepare_server_to_send_garbage_on_next_request()
         .unsubscribe("foobar", "datum"),
      communication_error);
}

BOOST_AUTO_TEST_CASE(MustThrownOnUnsubscriptionAndReceivesUnexpectedMessage)
{
   BOOST_CHECK_THROW(
      let_test()
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum")
         .subscribe("foobar", "datum")
         .prepare_server_to_respond_with(
               proto::begin_session_message("Bad luck!"))
         .unsubscribe("foobar", "datum"),
      communication_error);
}

BOOST_AUTO_TEST_CASE(MustReceiveVarUpdatesFromServer)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum", 600)
      .subscribe("foobar", "datum", 1)
      .server_sends_var_update(600, variable_value::from_dword(112233))
      .check_var_update_reception(
            1,
            "foobar",
            "datum",
            variable_value::from_dword(112233))
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustReceiveVarUpdatesFromServerToMultipleSubscriptions)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum", 600)
      .subscribe("foobar", "datum", 1)
      .subscribe("foobar", "datum", 2)
      .server_sends_var_update(600, variable_value::from_dword(112233))
      .check_var_update_reception(
            1,
            "foobar",
            "datum",
            variable_value::from_dword(112233))
      .check_var_update_reception(
            2,
            "foobar",
            "datum",
            variable_value::from_dword(112233))
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustIgnoreUnknownVarUpdatesFromServer)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum", 600)
      .subscribe("foobar", "datum", 1)
      .server_sends_var_update(601, variable_value::from_dword(112233))
      .check_var_update_unreceived(1)
      .server_sends_var_update(600, variable_value::from_dword(112233))
      .check_var_update_reception(
            1,
            "foobar",
            "datum",
            variable_value::from_dword(112233))
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustSendVarUpdateFromClient)
{
   let_test()
      .prepare_server_for_handshake()
      .connect()
      .prepare_server_for_subscription("foobar", "datum", 8000)
      .subscribe("foobar", "datum")
      .prepare_server_for_var_update(8000, variable_value::from_byte(127))
      .update("foobar", "datum", variable_value::from_byte(127))
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_CASE(MustThrowOnVarUpdateSentForUnknownSubscription)
{
   let_test test;
   BOOST_CHECK_THROW(
      test
         .prepare_server_for_handshake()
         .connect()
         .prepare_server_for_subscription("foobar", "datum", 8000)
         .subscribe("foobar", "datum")
         .update(1234, variable_value::from_byte(127)),
      flight_vars::no_such_subscription_error);
   test
      .prepare_server_for_close()
      .close();
}

BOOST_AUTO_TEST_SUITE_END()
