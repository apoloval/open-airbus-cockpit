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

#include <liboac/buffer.h>
#include <liboac/endian.h>

#include "protocol.h"

using namespace oac;
using namespace oac::fv;
using namespace oac::fv::proto;

template <typename Serializer, typename Deserializer>
struct protocol_test
{
   ptr<buffer_input_stream<fixed_buffer>> input;
   ptr<buffer_output_stream<fixed_buffer>> output;

   inline protocol_test()
   {
      auto buff = make_ptr(new fixed_buffer(1024));
      buffer::fill(*buff, 0xE1);
      input = buffer::make_input_stream(buff);
      output = buffer::make_output_stream(buff);
   }

   void serialize(const message& msg)
   { proto::serialize<Serializer>(msg, *output); }

   message deserialize()
   { return proto::deserialize<Deserializer>(*input); }   

   bool input_eof()
   {
      return 0xE1 == stream::read_as<std::uint8_t>(*input);
   }
};


BOOST_AUTO_TEST_SUITE(BinarySerializerTest)

BOOST_AUTO_TEST_CASE(ShouldSerializeSessionBegin)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   begin_session_message msg("FlightVars Test");
   test.serialize(msg);

   BOOST_CHECK_EQUAL(
            0x700, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            15, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            "FlightVars Test", stream::read_as_string(*test.input, 15));
   BOOST_CHECK_EQUAL(
            0x0100, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            0x0d0a, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK(test.input_eof());
}

BOOST_AUTO_TEST_CASE(ShouldDeserializeSessionBegin)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x700));
   stream::write_as(*test.output, native_to_big<std::uint16_t>(15));
   stream::write_as_string(*test.output, "FlightVars Test");
   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x0115));
   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x0d0a));
   message msg = test.deserialize();
   begin_session_message& bs_msg = boost::get<begin_session_message>(msg);

   BOOST_CHECK_EQUAL("FlightVars Test", bs_msg.pname);
   BOOST_CHECK_EQUAL(0x0115, bs_msg.proto_ver);
}

BOOST_AUTO_TEST_CASE(ShouldSerializeSessionEnd)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   end_session_message msg("Server is closing, bye!");
   test.serialize(msg);

   BOOST_CHECK_EQUAL(
            0x701, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            23, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            "Server is closing, bye!", stream::read_as_string(*test.input, 23));
   BOOST_CHECK_EQUAL(
            0x0d0a, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK(test.input_eof());
}

BOOST_AUTO_TEST_CASE(ShouldDeserializeSessionEnd)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x701));
   stream::write_as(*test.output, native_to_big<std::uint16_t>(23));
   stream::write_as_string(*test.output, "Client is closing, bye!");
   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x0d0a));
   message msg = test.deserialize();
   end_session_message& es_msg = boost::get<end_session_message>(msg);

   BOOST_CHECK_EQUAL("Client is closing, bye!", es_msg.cause);
}

BOOST_AUTO_TEST_CASE(ShouldSerializeSubscriptionRequest)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   subscription_request_message msg(
            variable_group("fsuipc/offset"), variable_name("0x4ca1"));
   test.serialize(msg);

   BOOST_CHECK_EQUAL(
            0x702, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            13, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            "fsuipc/offset", stream::read_as_string(*test.input, 13));
   BOOST_CHECK_EQUAL(
            6, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            "0x4ca1", stream::read_as_string(*test.input, 6));
   BOOST_CHECK_EQUAL(
            0x0d0a, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK(test.input_eof());
}

BOOST_AUTO_TEST_CASE(ShouldDeserializeSubscriptionRequest)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x702));
   stream::write_as(*test.output, native_to_big<std::uint16_t>(13));
   stream::write_as_string(*test.output,"fsuipc/offset");
   stream::write_as(*test.output, native_to_big<std::uint16_t>(6));
   stream::write_as_string(*test.output,"0x4ca1");
   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x0d0a));
   message msg = test.deserialize();
   subscription_request_message& sr_msg =
         boost::get<subscription_request_message>(msg);

   BOOST_CHECK_EQUAL("fsuipc/offset", sr_msg.var_grp.get_tag());
   BOOST_CHECK_EQUAL("0x4ca1", sr_msg.var_name.get_tag());
}

BOOST_AUTO_TEST_CASE(ShouldSerializeSubscriptionReply)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   subscription_reply_message msg(
            subscription_reply_message::STATUS_SUCCESS,
            variable_group("fsuipc/offset"),
            variable_name("0x4ca1"),
            "Successfully subscribed!");
   test.serialize(msg);

   BOOST_CHECK_EQUAL(
            0x703, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            0, stream::read_as<std::uint8_t>(*test.input));
   BOOST_CHECK_EQUAL(
            13, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            "fsuipc/offset", stream::read_as_string(*test.input, 13));
   BOOST_CHECK_EQUAL(
            6, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            "0x4ca1", stream::read_as_string(*test.input, 6));
   BOOST_CHECK_EQUAL(
            24, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK_EQUAL(
            "Successfully subscribed!", stream::read_as_string(*test.input, 24));
   BOOST_CHECK_EQUAL(
            0x0d0a, big_to_native(stream::read_as<std::uint16_t>(*test.input)));
   BOOST_CHECK(test.input_eof());
}

BOOST_AUTO_TEST_CASE(ShouldDeserializeSubscriptionReply)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x703));
   stream::write_as(*test.output, std::uint8_t(1));
   stream::write_as(*test.output, native_to_big<std::uint16_t>(13));
   stream::write_as_string(*test.output,"fsuipc/offset");
   stream::write_as(*test.output, native_to_big<std::uint16_t>(6));
   stream::write_as_string(*test.output,"0x4ca1");
   stream::write_as(*test.output, native_to_big<std::uint16_t>(17));
   stream::write_as_string(*test.output,"No such variable!");
   stream::write_as(*test.output, native_to_big<std::uint16_t>(0x0d0a));
   message msg = test.deserialize();
   subscription_reply_message& rep_msg =
         boost::get<subscription_reply_message>(msg);

   BOOST_CHECK_EQUAL(
            subscription_reply_message::STATUS_NO_SUCH_VAR, rep_msg.st);
   BOOST_CHECK_EQUAL("fsuipc/offset", rep_msg.var_grp.get_tag());
   BOOST_CHECK_EQUAL("0x4ca1", rep_msg.var_name.get_tag());
   BOOST_CHECK_EQUAL("No such variable!", rep_msg.cause);
}

BOOST_AUTO_TEST_SUITE_END()
