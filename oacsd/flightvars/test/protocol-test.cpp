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
      input = buffer::make_input_stream(buff);
      output = buffer::make_output_stream(buff);
   }

   void serialize(const message& msg)
   { proto::serialize<Serializer>(msg, *output); }

   message deserialize()
   { return proto::deserialize<Deserializer>(*input); }
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

   BOOST_CHECK_EQUAL("FlightVars Test", bs_msg.ep_name);
   BOOST_CHECK_EQUAL(0x0115, bs_msg.proto_ver);
}

BOOST_AUTO_TEST_SUITE_END()
