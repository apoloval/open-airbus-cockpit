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
   ptr<input_stream> input;
   ptr<output_stream> output;
   Serializer serializer;
   Deserializer deserializer;

   inline protocol_test()
   {
      ptr<fixed_buffer> buff = new fixed_buffer(1024);
      input = new buffer_input_stream(buff);
      output = new buffer_output_stream(buff);
   }

   void serialize(const message& msg)
   { serializer.serialize(msg, *output); }

   message deserialize()
   { return deserializer.deserialize(*input); }
};


BOOST_AUTO_TEST_SUITE(BinarySerializerTest)

BOOST_AUTO_TEST_CASE(ShouldSerializeSessionBegin)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   begin_session_message msg("FlightVars Test");
   test.serialize(msg);

   BOOST_CHECK_EQUAL(
            0x700, big_to_native(test.input->read_as<std::uint16_t>()));
   BOOST_CHECK_EQUAL(
            15, big_to_native(test.input->read_as<std::uint16_t>()));
   BOOST_CHECK_EQUAL(
            "FlightVars Test", test.input->read_as_string(15));
   BOOST_CHECK_EQUAL(
            0x0100, big_to_native(test.input->read_as<std::uint16_t>()));
   BOOST_CHECK_EQUAL(
            0x0d0a, big_to_native(test.input->read_as<std::uint16_t>()));
}

BOOST_AUTO_TEST_CASE(ShouldDeserializeSessionBegin)
{
   protocol_test<binary_message_serializer, binary_message_deserializer> test;

   test.output->write_as(native_to_big<std::uint16_t>(0x700));
   test.output->write_as(native_to_big<std::uint16_t>(15));
   test.output->write_as_string("FlightVars Test");
   test.output->write_as(native_to_big<std::uint16_t>(0x0115));
   test.output->write_as(native_to_big<std::uint16_t>(0x0d0a));
   message msg = test.deserialize();
   begin_session_message& bs_msg = boost::get<begin_session_message>(msg);

   BOOST_CHECK_EQUAL("FlightVars Test", bs_msg.ep_name);
   BOOST_CHECK_EQUAL(0x0115, bs_msg.proto_ver);
}

BOOST_AUTO_TEST_SUITE_END()
