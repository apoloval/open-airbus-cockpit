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

#ifndef OAC_FV_PROTOCOL_INL
#define OAC_FV_PROTOCOL_INL

#include <cstdint>
#include <string>

#include "protocol.h"

namespace oac { namespace fv { namespace proto {

namespace {

template <typename Deserializer, typename InputStream>
begin_session_message
deserialize_begin_session_contents(
      InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto pname = Deserializer::read_string_value(input);
   auto proto_ver = Deserializer::read_uint16_value(input);
   return begin_session_message(pname, proto_ver);
}

template <typename Deserializer, typename InputStream>
end_session_message
deserialize_end_session_contents(
      InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto cause = Deserializer::read_string_value(input);
   return end_session_message(cause);
}

template <typename Deserializer, typename InputStream>
subscription_request_message
deserialize_subscription_request_contents(
      InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto var_grp = Deserializer::read_string_value(input);
   auto var_name = Deserializer::read_string_value(input);
   return subscription_request_message(var_grp, var_name);
}

template <typename Deserializer, typename InputStream>
subscription_reply_message
deserialize_subscription_reply_contents(
      InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto st = Deserializer::read_uint8_value(input);
   auto grp = Deserializer::read_string_value(input);
   auto name = Deserializer::read_string_value(input);
   auto subs_id = Deserializer::read_uint32_value(input);
   auto cause = Deserializer::read_string_value(input);
   return subscription_reply_message(
            subscription_reply_message::status(st), grp, name, subs_id, cause);
}

template <typename Deserializer, typename InputStream>
var_update_message
deserialize_var_update_contents(
      InputStream& input)
throw (stream::read_error, stream::eof_error)
{
   auto subs_id = Deserializer::read_uint32_value(input);
   auto var_type = Deserializer::read_uint8_value(input);
   switch (var_type)
   {
      case VAR_BOOLEAN:
         return var_update_message(
                  subs_id,
                  variable_value::from_bool(
                     (Deserializer::read_uint8_value(input) > 0) ?
                                         true : false));
      case VAR_BYTE:
         return var_update_message(
                  subs_id,
                  variable_value::from_byte(
                     Deserializer::read_uint8_value(input)));
      case VAR_WORD:
         return var_update_message(
                  subs_id,
                  variable_value::from_word(
                     Deserializer::read_uint16_value(input)));
      case VAR_DWORD:
         return var_update_message(
                  subs_id,
                  variable_value::from_dword(
                     Deserializer::read_uint32_value(input)));
      case VAR_FLOAT:
         return var_update_message(
                  subs_id,
                  variable_value::from_float(
                     Deserializer::read_float_value(input)));
      default:
         BOOST_THROW_EXCEPTION(protocol_error() <<
               message_info(str(
                     boost::format(
                           "received an invalid value 0x%x for variable type") %
                     var_type)));
   }
}

} // anonymous namespace

inline
begin_session_message::begin_session_message(
      const peer_name& pname,
      protocol_version proto_ver)
   : pname(pname),
     proto_ver(proto_ver)
{}

inline
end_session_message::end_session_message(
      const std::string& cause)
   : cause(cause)
{}

inline
subscription_request_message::subscription_request_message(
      const variable_group& grp,
      const variable_name& name)
   : var_grp(grp),
     var_name(name)
{}

inline
subscription_reply_message::subscription_reply_message(
      status st,
      const variable_group& grp,
      const variable_name& name,
      const flight_vars::subscription_id& subs_id,
      const std::string& cause)
   : st(st),
     var_grp(grp),
     var_name(name),
     subscription_id(subs_id),
     cause(cause)
{}

inline
var_update_message::var_update_message(
      const flight_vars::subscription_id& subs_id,
      const variable_value& value)
   : subscription_id(subs_id),
     var_value(value)
{}

template <typename Serializer, typename OutputStream>
void
serialize_begin_session(
      const begin_session_message& msg,
      OutputStream& output)
throw (stream::write_error, stream::eof_error)
{
   Serializer::write_msg_begin(
            output, message_internals::MSG_BEGIN_SESSION);
   Serializer::write_string_value(output, msg.pname);
   Serializer::write_uint16_value(output, msg.proto_ver);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_end_session(
      const end_session_message& msg,
      OutputStream& output)
throw (stream::write_error, stream::eof_error)
{
   Serializer::write_msg_begin(
            output, message_internals::MSG_END_SESSION);
   Serializer::write_string_value(output, msg.cause);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_subscription_request(
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

template <typename Serializer, typename OutputStream>
void
serialize_subscription_reply(
      const subscription_reply_message& msg,
      OutputStream& output)
throw (stream::write_error, stream::eof_error)
{
   Serializer::write_msg_begin(
         output, message_internals::MSG_SUBSCRIPTION_REP);
   Serializer::write_uint8_value(output, msg.st);
   Serializer::write_string_value(output, msg.var_grp);
   Serializer::write_string_value(output, msg.var_name);
   Serializer::write_uint32_value(output, msg.subscription_id);
   Serializer::write_string_value(output, msg.cause);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_var_update(
      const var_update_message& msg,
      OutputStream& output)
{
   auto var_type = msg.var_value.get_type();
   Serializer::write_msg_begin(
            output, message_internals::MSG_VAR_UPDATE);
   Serializer::write_uint32_value(output, msg.subscription_id);
   switch (var_type)
   {
      case VAR_BOOLEAN:
         Serializer::write_uint8_value(
                  output, message_internals::VAR_TYPE_BOOLEAN);
         Serializer::write_uint8_value(
                  output, msg.var_value.as_bool() ? 1 : 0);
         break;
      case VAR_BYTE:
         Serializer::write_uint8_value(
                  output, message_internals::VAR_TYPE_BYTE);
         Serializer::write_uint8_value(
                  output, msg.var_value.as_byte());
         break;
      case VAR_WORD:
         Serializer::write_uint8_value(
                  output, message_internals::VAR_TYPE_WORD);
         Serializer::write_uint16_value(
                  output, msg.var_value.as_word());
         break;
      case VAR_DWORD:
         Serializer::write_uint8_value(
                  output, message_internals::VAR_TYPE_DWORD);
         Serializer::write_uint32_value(
                  output, msg.var_value.as_dword());
         break;
      case VAR_FLOAT:
         Serializer::write_uint8_value(
                  output, message_internals::VAR_TYPE_FLOAT);
         Serializer::write_float_value(
                  output, msg.var_value.as_float());
         break;
   }
   Serializer::write_msg_end(output);
}


template <typename Serializer, typename OutputStream>
void
serialize(
      const message& msg,
      OutputStream& output)
{
   struct visitor : public boost::static_visitor<void>
   {
      OutputStream& output;

      inline visitor(OutputStream& os) : output(os) {}

      void operator()(const begin_session_message& msg) const
      {
         return serialize_begin_session<Serializer, OutputStream>(msg, output);
      }

      void operator()(const end_session_message& msg) const
      {
         return serialize_end_session<Serializer, OutputStream>(msg, output);
      }

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

      void operator()(const var_update_message& msg) const
      {
         return serialize_var_update<Serializer, OutputStream>(msg, output);
      }

   } visit(output);
   boost::apply_visitor(visit, msg);
}

template <typename Deserializer, typename InputStream>
message
deserialize(
      InputStream& input)
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
      case message_internals::MSG_VAR_UPDATE:
      {
         auto msg = deserialize_var_update_contents<Deserializer>(input);
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

template <typename OutputStream>
void
binary_message_serializer::write_msg_begin(
      OutputStream& output,
      message_internals::message_type msg_type)
{
   stream::write_as(output, native_to_big<std::uint16_t>(msg_type));
}

template <typename OutputStream>
void
binary_message_serializer::write_msg_end(
      OutputStream& output)
{
   stream::write_as(output, native_to_big<std::uint16_t>(0x0d0a));
}

template <typename OutputStream>
void
binary_message_serializer::write_string_value(
      OutputStream& output,
      const std::string& value)
{
   stream::write_as(output, native_to_big<std::uint16_t>(value.length()));
   stream::write_as_string(output, value);
}

template <typename OutputStream>
void
binary_message_serializer::write_uint8_value(
      OutputStream& output,
      std::uint8_t value)
{
   stream::write_as(output, value);
}

template <typename OutputStream>
void
binary_message_serializer::write_uint16_value(
      OutputStream& output,
      std::uint16_t value)
{
   stream::write_as(output, native_to_big<std::uint16_t>(value));
}

template <typename OutputStream>
void
binary_message_serializer::write_uint32_value(
      OutputStream& output,
      std::uint32_t value)
{
   stream::write_as(output, native_to_big<std::uint32_t>(value));
}

template <typename OutputStream>
void
binary_message_serializer::write_float_value(
      OutputStream& output,
      float value)
{
   // Break the number info binary significant and integral exponent for 2.
   // Then, write the normalized significant and exponent as 32-bits values.
   int exp = 0;
   auto sig = std::frexp(value, &exp);
   std::uint32_t nsig = 2 * std::uint32_t((sig - 0.5f) * UINT32_MAX);
   stream::write_as(output, native_to_big<std::uint32_t>(nsig));
   stream::write_as(output, native_to_big<std::uint32_t>(exp));
}




template <typename InputStream>
message_internals::message_type
binary_message_deserializer::read_msg_begin(
      InputStream& input)
throw (protocol_error)
{
   return message_internals::message_type(
            big_to_native(stream::read_as<std::uint16_t>(input)));
}

template <typename InputStream>
void
binary_message_deserializer::read_msg_end(InputStream& input)
throw (protocol_error)
{
   auto eol = native_to_big(stream::read_as<uint16_t>(input));
   if (eol != 0x0d0a)
      BOOST_THROW_EXCEPTION(protocol_error() <<
         expected_input_info("a message termination mark 0x0D0A") <<
         actual_input_info(str(boost::format("bytes 0x%x") % eol)));
}

template <typename InputStream>
std::string
binary_message_deserializer::read_string_value(InputStream& input)
throw (protocol_error)
{
   auto str_len = big_to_native(stream::read_as<std::uint16_t>(input));
   return stream::read_as_string(input, str_len);
}

template <typename InputStream>
std::uint16_t
binary_message_deserializer::read_uint16_value(InputStream& input)
throw (protocol_error)
{
   return big_to_native(stream::read_as<std::uint16_t>(input));
}

template <typename InputStream>
std::uint8_t
binary_message_deserializer::read_uint8_value(InputStream& input)
throw (protocol_error)
{
   return stream::read_as<std::uint8_t>(input);
}

template <typename InputStream>
std::uint32_t
binary_message_deserializer::read_uint32_value(
      InputStream& input)
throw (protocol_error)
{
   return big_to_native(stream::read_as<std::uint32_t>(input));
}

template <typename InputStream>
float
binary_message_deserializer::read_float_value(
      InputStream& input)
throw (protocol_error)
{
   // The stream contains the normalized binary significant and the exponent
   // for 2. We extract them and convert again into float.
   auto nsig = big_to_native(stream::read_as<std::uint32_t>(input));
   auto exp = big_to_native(stream::read_as<std::uint32_t>(input));
   float sig = nsig * 0.5f / UINT32_MAX + 0.5f;
   return std::ldexp(sig, exp);
}

}}} // namespace oac::fv::proto

#endif
