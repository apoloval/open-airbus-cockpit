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

#ifndef OAC_FV_PROTOCOL_H
#define OAC_FV_PROTOCOL_H

#include <cstdint>
#include <string>

#include <boost/variant.hpp>

#include <liboac/exception.h>
#include <liboac/stream.h>

namespace oac { namespace fv { namespace proto {

/**
 * Thrown when a protocol error is found. Contains:
 *  - expected_input_info, indicating the expected protocol element
 *  - actual_input_info, indicating the actual protocol element found
 */
OAC_DECL_ERROR(protocol_error, invalid_input_error);

OAC_DECL_ERROR_INFO(expected_input_info, std::string);
OAC_DECL_ERROR_INFO(actual_input_info, std::string);

typedef std::uint16_t protocol_version;

typedef std::string endpoint_name;

extern const protocol_version CURRENT_PROTOCOL_VERSION;

/**
 * Begin session message. This message is sent by the client when initiates
 * the session and the server as response to that. It indicates the endpoint
 * name and the protocol version it implements.
 */
struct begin_session_message {
   endpoint_name ep_name;
   protocol_version proto_ver;

   inline begin_session_message(
         const endpoint_name& ep_name,
         protocol_version proto_ver = CURRENT_PROTOCOL_VERSION)
      : ep_name(ep_name), proto_ver(proto_ver)
   {}
};

/**
 * This union wraps all kinds of messages into a single one.
 */
typedef boost::variant<
      begin_session_message
> message;

struct message_internals
{
   enum message_type {
      MSG_BEGIN_SESSION = 0x700
   };
};

/**
 * Serialize given message into given output stream.
 */
template <typename Serializer, typename OutputStream>
inline void serialize(const message& msg, OutputStream& output)
{
   struct visitor : public boost::static_visitor<void>
   {
      OutputStream& output;

      inline visitor(OutputStream& os) : output(os) {}

      void operator()(const begin_session_message& msg) const
      {
         Serializer::write_msg_begin(
                  output, message_internals::MSG_BEGIN_SESSION);
         Serializer::write_string_value(output, msg.ep_name);
         Serializer::write_uint16_value(output, msg.proto_ver);
         Serializer::write_msg_end(output);
      }
   } visit(output);
   boost::apply_visitor(visit, msg);
}

template <typename Deserializer, typename InputStream>
begin_session_message
deserialize_begin_session(InputStream& input)
throw (protocol_error)
{
   auto ep_name = Deserializer::read_string_value(input);
   auto proto_ver = Deserializer::read_uint16_value(input);
   return begin_session_message(ep_name, proto_ver);
}

template <typename Deserializer, typename InputStream>
message
deserialize(InputStream& input)
throw (protocol_error)
{
   auto msg_begin = Deserializer::read_msg_begin(input);
   switch (msg_begin)
   {
      case message_internals::MSG_BEGIN_SESSION:
         return deserialize_begin_session<Deserializer>(input);
      default:
         BOOST_THROW_EXCEPTION(illegal_state_error() <<
               message_info("received an unknown value for message type"));
   }
   Deserializer::read_msg_end(input);
}

struct binary_message_serializer
{

   template <typename OutputStream>
   inline static void write_msg_begin(
         OutputStream& output,
         message_internals::message_type msg_type)
   {
      stream::write_as(output, native_to_big<std::uint16_t>(msg_type));
   }

   template <typename OutputStream>
   inline static void write_msg_end(OutputStream& output)
   {
      stream::write_as(output, native_to_big<std::uint16_t>(0x0d0a));
   }

   template <typename OutputStream>
   inline static void write_string_value(
         OutputStream& output, const std::string& value)
   {
      stream::write_as(output, native_to_big<std::uint16_t>(value.length()));
      stream::write_as_string(output, value);
   }

   template <typename OutputStream>
   inline static void write_uint16_value(
         OutputStream& output, std::uint16_t value)
   {
      stream::write_as(output, native_to_big<std::uint16_t>(value));
   }
};

struct binary_message_deserializer
{
   template <typename InputStream>
   inline static message_internals::message_type read_msg_begin(
         InputStream& input)
   throw (protocol_error)
   {
      return message_internals::message_type(
               big_to_native(stream::read_as<std::uint16_t>(input)));
   }

   template <typename InputStream>
   inline static void read_msg_end(InputStream& input)
   throw (protocol_error)
   {
      auto eol = native_to_big(stream::read_as<uint16_t>(input));
      if (eol != 0x0d0a)
         BOOST_THROW_EXCEPTION(protocol_error() <<
            expected_input_info("a message termination mark 0x0D0A") <<
            actual_input_info(str(boost::format("bytes 0x%x") % eol)));
   }

   template <typename InputStream>
   inline static std::string read_string_value(InputStream& input)
   throw (protocol_error)
   {
      auto str_len = big_to_native(stream::read_as<std::uint16_t>(input));
      return stream::read_as_string(input, str_len);
   }

   template <typename InputStream>
   inline static std::uint16_t read_uint16_value(InputStream& input)
   throw (protocol_error)
   {
      return big_to_native(stream::read_as<std::uint16_t>(input));
   }
};

}}} // namespace oac::fv::proto

#endif
