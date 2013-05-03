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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#include "protocol.h"

#include <liboac/endian.h>

namespace oac { namespace fv { namespace proto {

using ::oac::fv::proto::message_internals;

const protocol_version CURRENT_PROTOCOL_VERSION(0x0100);

void
message_serializer::serialize(const message& msg, output_stream& output)
{
   visitor v(*this, output);
   boost::apply_visitor(v, msg);
}

void
message_serializer::visitor::operator()(
      const begin_session_message& msg) const
{
   serializer.write_msg_begin(output, message_internals::MSG_BEGIN_SESSION);
   serializer.write_string_value(output, msg.ep_name);
   serializer.write_uint16_value(output, msg.proto_ver);
   serializer.write_msg_end(output);
}

message message_deserializer::deserialize(input_stream& input)
throw(protocol_error)
{
   auto msg_begin = read_msg_begin(input);
   switch (msg_begin)
   {
      case message_internals::MSG_BEGIN_SESSION:
         return deserialize_begin_session(input);
      default:
         BOOST_THROW_EXCEPTION(illegal_state_error() <<
               message_info("received an unknown value for message type"));
   }
   read_msg_end(input);
}

begin_session_message
message_deserializer::deserialize_begin_session(input_stream& input)
throw(protocol_error)
{
   auto ep_name = read_string_value(input);
   auto proto_ver = read_uint16_value(input);
   return begin_session_message(ep_name, proto_ver);
}

void
binary_message_serializer::write_msg_begin(
      output_stream& output,
      message_internals::message_type msg_type)
{
   output.write_as<std::uint16_t>(native_to_big<std::uint16_t>(msg_type));
}

void
binary_message_serializer::write_msg_end(output_stream& output)
{
   output.write_as(native_to_big<std::uint16_t>(0x0d0a));
}

void
binary_message_serializer::write_string_value(
      output_stream& output, const std::string& value)
{
   output.write_as(native_to_big<std::uint16_t>(value.length()));
   output.write_as<std::string>(value);
}

void
binary_message_serializer::write_uint16_value(
      output_stream& output, std::uint16_t value)
{
   output.write_as(native_to_big<std::uint16_t>(value));
}

message_internals::message_type
binary_message_deserializer::read_msg_begin(
      input_stream& input)
throw (protocol_error)
{
   return message_internals::message_type(
            big_to_native(input.read_as<std::uint16_t>()));
}

void
binary_message_deserializer::read_msg_end(
      input_stream& input)
throw (protocol_error)
{
   auto eol = native_to_big(input.read_as<uint16_t>());
   if (eol != 0x0d0a)
      BOOST_THROW_EXCEPTION(protocol_error() <<
         expected_input_info("a message termination mark 0x0D0A") <<
         actual_input_info(str(boost::format("bytes 0x%x") % eol)));
}

std::string
binary_message_deserializer::read_string_value(input_stream& input)
throw (protocol_error)
{
   auto str_len = big_to_native(input.read_as<std::uint16_t>());
   return input.read_as_string(str_len);
}

std::uint16_t
binary_message_deserializer::read_uint16_value(input_stream& input)
throw (protocol_error)
{
   return big_to_native(input.read_as<std::uint16_t>());
}

}}} // namespace oac::fv::proto
