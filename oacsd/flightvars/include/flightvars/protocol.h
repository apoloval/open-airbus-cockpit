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
#include <liboac/endian.h>
#include <liboac/exception.h>
#include <liboac/stream.h>

#include "api.h"

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

typedef std::string peer_name;

extern const protocol_version CURRENT_PROTOCOL_VERSION;

/**
 * This message is sent by the client when initiates the session and the
 * server as response to that. It indicates the endpoint name and the
 * protocol version it implements.
 */
struct begin_session_message
{
   peer_name pname;
   protocol_version proto_ver;

   inline begin_session_message(
         const peer_name& pname,
         protocol_version proto_ver = CURRENT_PROTOCOL_VERSION)
      : pname(pname), proto_ver(proto_ver)
   {}
};

/**
 * This message is sent by either server or client when it wants to close
 * the session. The cause field indicates the cause for the session to be ended.
 */
struct end_session_message {
   std::string cause;

   inline end_session_message(const std::string& cause) : cause(cause) {}
};

/**
 * This message is sent by the client to request a new subscription for a
 * variable. The server responds with a subscription reply message.
 */
struct subscription_request_message
{
   variable_group var_grp;
   variable_name var_name;

   inline subscription_request_message(
         const variable_group& grp,
         const variable_name& name)
      : var_grp(grp), var_name(name)
   {}
};

/**
 * This message is sent by the server as response to a subscription request.
 */
struct subscription_reply_message
{
   enum status
   {
      /** The request was accepted and processed successfully */
      STATUS_SUCCESS,
      /** The requested variable is unknown to the server */
      STATUS_NO_SUCH_VAR,
      /** A server error ocurred which prevented the request to be success */
      STATUS_SERVER_ERROR
   };

   status st;
   variable_group var_grp;
   variable_name var_name;
   std::string cause;

   inline subscription_reply_message(
         status st,
         const variable_group& grp,
         const variable_name& name,
         const std::string& cause)
      : st(st), var_grp(grp), var_name(name), cause(cause)
   {}
};

/**
 * This union wraps all kinds of messages into a single one.
 */
typedef boost::variant<
      begin_session_message,
      end_session_message,
      subscription_request_message,
      subscription_reply_message
> message;

struct message_internals
{
   enum message_type {
      MSG_BEGIN_SESSION = 0x700,
      MSG_END_SESSION = 0x701,
      MSG_SUBSCRIPTION_REQ = 0x702,
      MSG_SUBSCRIPTION_REP = 0x703
   };
};

/**
 * Serialize a begin session message into given output stream.
 */
template <typename Serializer, typename OutputStream>
inline void serialize_begin_session(
      const begin_session_message& msg, OutputStream& output)
throw (stream::write_error, stream::eof_error)
{
   Serializer::write_msg_begin(
            output, message_internals::MSG_BEGIN_SESSION);
   Serializer::write_string_value(output, msg.pname);
   Serializer::write_uint16_value(output, msg.proto_ver);
   Serializer::write_msg_end(output);
}

/**
 * Serialize an end session message into given output stream.
 */
template <typename Serializer, typename OutputStream>
inline void serialize_end_session(
      const end_session_message& msg, OutputStream& output)
throw (stream::write_error, stream::eof_error)
{
   Serializer::write_msg_begin(
            output, message_internals::MSG_END_SESSION);
   Serializer::write_string_value(output, msg.cause);
   Serializer::write_msg_end(output);
}

/**
 * Serialize a subscription request message into given output stream.
 */
template <typename Serializer, typename OutputStream>
inline void serialize_subscription_request(
      const subscription_request_message& msg,
      OutputStream& output)
throw (stream::write_error, stream::eof_error)
{
   Serializer::write_msg_begin(
         output, message_internals::MSG_SUBSCRIPTION_REQ);
   Serializer::write_string_value(output, msg.var_grp);
   Serializer::write_string_value(output, msg.var_name);
   Serializer::write_msg_end(output);
}

/**
 * Serialize a subscription reply message into given output stream.
 */
template <typename Serializer, typename OutputStream>
inline void serialize_subscription_reply(
      const subscription_reply_message& msg,
      OutputStream& output)
throw (stream::write_error, stream::eof_error)
{
   Serializer::write_msg_begin(
         output, message_internals::MSG_SUBSCRIPTION_REP);
   Serializer::write_uint8_value(output, msg.st);
   Serializer::write_string_value(output, msg.var_grp);
   Serializer::write_string_value(output, msg.var_name);
   Serializer::write_string_value(output, msg.cause);
   Serializer::write_msg_end(output);
}

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
      { return serialize_begin_session<Serializer, OutputStream>(msg, output); }

      void operator()(const end_session_message& msg) const
      { return serialize_end_session<Serializer, OutputStream>(msg, output); }

      void operator()(const subscription_request_message& msg) const
      {
         return serialize_subscription_request<Serializer, OutputStream>(
               msg, output);
      }

      void operator()(const subscription_reply_message& msg) const
      {
         return serialize_subscription_reply<Serializer, OutputStream>(
               msg, output);
      }

   } visit(output);
   boost::apply_visitor(visit, msg);
}

template <typename Deserializer, typename InputStream>
begin_session_message
deserialize_begin_session_contents(InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto pname = Deserializer::read_string_value(input);
   auto proto_ver = Deserializer::read_uint16_value(input);
   return begin_session_message(pname, proto_ver);
}

template <typename Deserializer, typename InputStream>
end_session_message
deserialize_end_session_contents(InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto cause = Deserializer::read_string_value(input);
   return end_session_message(cause);
}

template <typename Deserializer, typename InputStream>
subscription_request_message
deserialize_subscription_request_contents(InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto var_grp = Deserializer::read_string_value(input);
   auto var_name = Deserializer::read_string_value(input);
   return subscription_request_message(var_grp, var_name);
}

template <typename Deserializer, typename InputStream>
subscription_reply_message
deserialize_subscription_reply_contents(InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto st = Deserializer::read_uint8_value(input);
   auto grp = Deserializer::read_string_value(input);
   auto name = Deserializer::read_string_value(input);
   auto cause = Deserializer::read_string_value(input);
   return subscription_reply_message(
            subscription_reply_message::status(st), grp, name, cause);
}

template <typename Deserializer, typename InputStream>
message
deserialize(InputStream& input)
throw (protocol_error, stream::eof_error)
{
   auto msg_begin = Deserializer::read_msg_begin(input);
   switch (msg_begin)
   {
      case message_internals::MSG_BEGIN_SESSION:
      {
         auto msg = deserialize_begin_session_contents<Deserializer>(input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_internals::MSG_END_SESSION:
      {
         auto msg = deserialize_end_session_contents<Deserializer>(input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_internals::MSG_SUBSCRIPTION_REQ:
      {
         auto msg = deserialize_subscription_request_contents<Deserializer>(
                  input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_internals::MSG_SUBSCRIPTION_REP:
      {
         auto msg = deserialize_subscription_reply_contents<Deserializer>(
                  input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      default:
         BOOST_THROW_EXCEPTION(protocol_error() <<
               message_info(str(
                     boost::format("received an invalid value "
                                   "0x%x for message type") %
                     int(msg_begin))));
   }
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
   inline static void write_uint8_value(
         OutputStream& output, std::uint8_t value)
   {
      stream::write_as(output, value);
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

   template <typename InputStream>
   inline static std::uint8_t read_uint8_value(InputStream& input)
   throw (protocol_error)
   {
      return stream::read_as<std::uint8_t>(input);
   }
};

}}} // namespace oac::fv::proto

#endif
