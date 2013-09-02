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

/**
 * An exception indicating a invalid variable type code while deserializing.
 */
OAC_EXCEPTION_BEGIN(invalid_variable_type, protocol_exception)
   OAC_EXCEPTION_FIELD(var_code, std::uint8_t)
   OAC_EXCEPTION_MSG(
      "invalid variable type code 0x%x received",
      var_code)
OAC_EXCEPTION_END()

/**
 * An exception indicating a invalid message type code while deserializing.
 */
OAC_EXCEPTION_BEGIN(invalid_message_type, protocol_exception)
   OAC_EXCEPTION_FIELD(message_code, std::uint16_t)
   OAC_EXCEPTION_MSG(
      "invalid message type code 0x%x received",
      message_code)
OAC_EXCEPTION_END()


/**
 * An exception indicating a invalid message termination mark
 * while deserializing.
 */
OAC_EXCEPTION_BEGIN(invalid_termination_mark, protocol_exception)
   OAC_EXCEPTION_FIELD(termination_mark, std::uint16_t)
   OAC_EXCEPTION_MSG(
      "invalid termination mark 0x%x received (expected 0x0D0A)",
      termination_mark)
OAC_EXCEPTION_END()

std::uint16_t msg_type_to_code(message_type msg_type)
{
   return std::uint16_t(msg_type) + 0x700;
};

message_type code_to_msg_type(std::uint16_t code)
{
   return message_type(code - 0x700);
}

std::uint8_t var_type_to_code(variable_type var_type)
{
   return std::uint8_t(var_type);
}

variable_type code_to_var_type(std::uint8_t code)
{
   return variable_type(code);
}

template <typename Serializer, typename OutputStream>
void
serialize_begin_session(
      const begin_session_message& msg,
      OutputStream& output)
throw (io_exception)
{
   Serializer::write_msg_begin(output, message_type::BEGIN_SESSION);
   Serializer::write_string_value(output, msg.pname);
   Serializer::write_uint16_value(output, msg.proto_ver);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_end_session(
      const end_session_message& msg,
      OutputStream& output)
throw (io_exception)
{
   Serializer::write_msg_begin(output, message_type::END_SESSION);
   Serializer::write_string_value(output, msg.cause);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_subscription_request(
      const subscription_request_message& msg,
      OutputStream& output)
throw (io_exception)
{
   Serializer::write_msg_begin(output, message_type::SUBSCRIPTION_REQ);
   Serializer::write_string_value(output, msg.var_grp);
   Serializer::write_string_value(output, msg.var_name);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_subscription_reply(
      const subscription_reply_message& msg,
      OutputStream& output)
throw (io_exception)
{
   Serializer::write_msg_begin(output, message_type::SUBSCRIPTION_REP);
   Serializer::write_uint8_value(output, static_cast<int>(msg.st));
   Serializer::write_string_value(output, msg.var_grp);
   Serializer::write_string_value(output, msg.var_name);
   Serializer::write_uint32_value(output, msg.subs_id);
   Serializer::write_string_value(output, msg.cause);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_unsubscription_request(
      const unsubscription_request_message& msg,
      OutputStream& output)
throw (io_exception)
{
   Serializer::write_msg_begin(output, message_type::UNSUBSCRIPTION_REQ);
   Serializer::write_uint32_value(output, msg.subs_id);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_unsubscription_reply(
      const unsubscription_reply_message& msg,
      OutputStream& output)
throw (io_exception)
{
   Serializer::write_msg_begin(output, message_type::UNSUBSCRIPTION_REP);
   Serializer::write_uint8_value(output, static_cast<int>(msg.st));
   Serializer::write_uint32_value(output, msg.subs_id);
   Serializer::write_string_value(output, msg.cause);
   Serializer::write_msg_end(output);
}

template <typename Serializer, typename OutputStream>
void
serialize_var_update(
      const var_update_message& msg,
      OutputStream& output)
throw (io_exception)
{
   auto var_type = msg.var_value.get_type();
   Serializer::write_msg_begin(output, message_type::VAR_UPDATE);
   Serializer::write_uint32_value(output, msg.subs_id);
   switch (var_type)
   {
      case variable_type::BOOLEAN:
         Serializer::write_uint8_value(
                  output, var_type_to_code(variable_type::BOOLEAN));
         Serializer::write_uint8_value(
                  output, msg.var_value.as_bool() ? 1 : 0);
         break;
      case variable_type::BYTE:
         Serializer::write_uint8_value(
                  output, var_type_to_code(variable_type::BYTE));
         Serializer::write_uint8_value(
                  output, msg.var_value.as_byte());
         break;
      case variable_type::WORD:
         Serializer::write_uint8_value(
                  output, var_type_to_code(variable_type::WORD));
         Serializer::write_uint16_value(
                  output, msg.var_value.as_word());
         break;
      case variable_type::DWORD:
         Serializer::write_uint8_value(
                  output, var_type_to_code(variable_type::DWORD));
         Serializer::write_uint32_value(
                  output, msg.var_value.as_dword());
         break;
      case variable_type::FLOAT:
         Serializer::write_uint8_value(
                  output, var_type_to_code(variable_type::FLOAT));
         Serializer::write_float_value(
                  output, msg.var_value.as_float());
         break;
   }
   Serializer::write_msg_end(output);
}

template <typename Deserializer, typename InputStream>
begin_session_message
deserialize_begin_session_contents(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   auto pname = Deserializer::read_string_value(input);
   auto proto_ver = Deserializer::read_uint16_value(input);
   return begin_session_message(pname, proto_ver);
}

template <typename Deserializer, typename InputStream>
end_session_message
deserialize_end_session_contents(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   auto cause = Deserializer::read_string_value(input);
   return end_session_message(cause);
}

template <typename Deserializer, typename InputStream>
subscription_request_message
deserialize_subscription_request_contents(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   auto var_grp = Deserializer::read_string_value(input);
   auto var_name = Deserializer::read_string_value(input);
   return subscription_request_message(var_grp, var_name);
}

template <typename Deserializer, typename InputStream>
subscription_reply_message
deserialize_subscription_reply_contents(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   auto st = Deserializer::read_uint8_value(input);
   auto grp = Deserializer::read_string_value(input);
   auto name = Deserializer::read_string_value(input);
   auto subs_id = Deserializer::read_uint32_value(input);
   auto cause = Deserializer::read_string_value(input);
   return subscription_reply_message(
            static_cast<subscription_status>(st), grp, name, subs_id, cause);
}

template <typename Deserializer, typename InputStream>
unsubscription_request_message
deserialize_unsubscription_request_contents(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   auto subs_id = Deserializer::read_uint32_value(input);
   return unsubscription_request_message(subs_id);
}

template <typename Deserializer, typename InputStream>
unsubscription_reply_message
deserialize_unsubscription_reply_contents(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   auto st = Deserializer::read_uint8_value(input);
   auto subs_id = Deserializer::read_uint32_value(input);
   auto cause = Deserializer::read_string_value(input);
   return unsubscription_reply_message(
            static_cast<subscription_status>(st), subs_id, cause);
}

template <typename Deserializer, typename InputStream>
var_update_message
deserialize_var_update_contents(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   auto subs_id = Deserializer::read_uint32_value(input);
   auto var_type = Deserializer::read_uint8_value(input);
   switch (var_type)
   {
      case variable_type::BOOLEAN:
         return var_update_message(
                  subs_id,
                  variable_value::from_bool(
                     (Deserializer::read_uint8_value(input) > 0) ?
                                         true : false));
      case variable_type::BYTE:
         return var_update_message(
                  subs_id,
                  variable_value::from_byte(
                     Deserializer::read_uint8_value(input)));
      case variable_type::WORD:
         return var_update_message(
                  subs_id,
                  variable_value::from_word(
                     Deserializer::read_uint16_value(input)));
      case variable_type::DWORD:
         return var_update_message(
                  subs_id,
                  variable_value::from_dword(
                     Deserializer::read_uint32_value(input)));
      case variable_type::FLOAT:
         return var_update_message(
                  subs_id,
                  variable_value::from_float(
                     Deserializer::read_float_value(input)));
      default:
         OAC_THROW_EXCEPTION(invalid_variable_type().with_var_code(var_type));
   }
}

} // anonymous namespace

inline
std::string
message_type_to_string(
      message_type msg_type)
{
   switch (msg_type)
   {
      case message_type::BEGIN_SESSION:
         return "begin session message";
      case message_type::END_SESSION:
         return "end session message";
      case message_type::SUBSCRIPTION_REQ:
         return "subscription request message";
      case message_type::SUBSCRIPTION_REP:
         return "subscription reply message";
      case message_type::UNSUBSCRIPTION_REQ:
         return "unsubscription request message";
      case message_type::UNSUBSCRIPTION_REP:
         return "unsubscription reply message";
      case message_type::VAR_UPDATE:
         return "variable update message";
      default:
         OAC_THROW_EXCEPTION(enum_out_of_range_error<message_type>()
               .with_value(msg_type));
   }
}

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
std::string
to_string(subscription_status status)
{
   switch (status)
   {
      case subscription_status::SUBSCRIBED:
         return "subscribed";
      case subscription_status::UNSUBSCRIBED:
         return "unsubscribed";
      case subscription_status::NO_SUCH_VAR:
         return "no such var";
      case subscription_status::NO_SUCH_SUBSCRIPTION:
         return "no such subscription";
      case subscription_status::UNKNOWN:
         return "unknown";
      default:
         OAC_THROW_EXCEPTION(enum_out_of_range_error<subscription_status>()
               .with_value(status));
   }
}

inline
std::ostream&
operator <<(
      std::ostream& s,
      subscription_status status)
{
   return s << to_string(status);
}

inline
subscription_reply_message::subscription_reply_message(
      subscription_status st,
      const variable_group& grp,
      const variable_name& name,
      const subscription_id& subs,
      const std::string& cause)
   : st(st),
     var_grp(grp),
     var_name(name),
     subs_id(subs),
     cause(cause)
{}

inline
unsubscription_request_message::unsubscription_request_message(
      subscription_id subs)
   : subs_id(subs)
{}

inline
unsubscription_reply_message::unsubscription_reply_message(
      subscription_status st,
      subscription_id subs_id,
      std::string cause)
   : st(st),
     subs_id(subs_id),
     cause(cause)
{}

inline
var_update_message::var_update_message(
      const subscription_id& subs,
      const variable_value& value)
   : subs_id(subs),
     var_value(value)
{}

inline
message_type
get_message_type(
      const message& msg)
{
   struct visitor : public boost::static_visitor<message_type>
   {
      message_type operator()(const begin_session_message& msg) const
      throw (io_exception)
      {
         return message_type::BEGIN_SESSION;
      }

      message_type operator()(const end_session_message& msg) const
      throw (io_exception)
      {
         return message_type::END_SESSION;
      }

      message_type operator()(const subscription_request_message& msg) const
      throw (io_exception)
      {
         return message_type::SUBSCRIPTION_REQ;
      }

      message_type operator()(const subscription_reply_message& msg) const
      throw (io_exception)
      {
         return message_type::SUBSCRIPTION_REP;
      }

      message_type operator()(const unsubscription_request_message& msg) const
      throw (io_exception)
      {
         return message_type::UNSUBSCRIPTION_REQ;
      }

      message_type operator()(const unsubscription_reply_message& msg) const
      throw (io_exception)
      {
         return message_type::UNSUBSCRIPTION_REP;
      }

      message_type operator()(const var_update_message& msg) const
      throw (io_exception)
      {
         return message_type::VAR_UPDATE;
      }

   } visit;
   return boost::apply_visitor(visit, msg);
}

template <typename Message>
bool
if_message_type(
      const message& msg,
      const std::function<void(const Message& msg)>& action)
{
   auto match = boost::get<Message>(&msg);
   if (match)
      action(*match);
   return match != nullptr;
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
      throw (io_exception)
      {
         return serialize_begin_session<Serializer, OutputStream>(msg, output);
      }

      void operator()(const end_session_message& msg) const
      throw (io_exception)
      {
         return serialize_end_session<Serializer, OutputStream>(msg, output);
      }

      void operator()(const subscription_request_message& msg) const
      throw (io_exception)
      {
         return serialize_subscription_request<Serializer, OutputStream>(
               msg, output);
      }

      void operator()(const subscription_reply_message& msg) const
      throw (io_exception)
      {
         return serialize_subscription_reply<Serializer, OutputStream>(
               msg, output);
      }

      void operator()(const unsubscription_request_message& msg) const
      throw (io_exception)
      {
         return serialize_unsubscription_request<Serializer, OutputStream>(
               msg, output);
      }

      void operator()(const unsubscription_reply_message& msg) const
      throw (io_exception)
      {
         return serialize_unsubscription_reply<Serializer, OutputStream>(
               msg, output);
      }

      void operator()(const var_update_message& msg) const
      throw (io_exception)
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
throw (protocol_exception, io_exception)
{
   auto msg_begin = Deserializer::read_msg_begin(input);
   switch (msg_begin)
   {
      case message_type::BEGIN_SESSION:
      {
         auto msg = deserialize_begin_session_contents<Deserializer>(input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_type::END_SESSION:
      {
         auto msg = deserialize_end_session_contents<Deserializer>(input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_type::SUBSCRIPTION_REQ:
      {
         auto msg = deserialize_subscription_request_contents<Deserializer>(
                  input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_type::SUBSCRIPTION_REP:
      {
         auto msg = deserialize_subscription_reply_contents<Deserializer>(
                  input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_type::UNSUBSCRIPTION_REQ:
      {
         auto msg = deserialize_unsubscription_request_contents<Deserializer>(
                  input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_type::UNSUBSCRIPTION_REP:
      {
         auto msg = deserialize_unsubscription_reply_contents<Deserializer>(
                  input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      case message_type::VAR_UPDATE:
      {
         auto msg = deserialize_var_update_contents<Deserializer>(input);
         Deserializer::read_msg_end(input);
         return msg;
      }
      default:
         OAC_THROW_EXCEPTION(invalid_message_type()
               .with_message_code(int(msg_begin)));
   }
}

template <typename OutputStream>
void
binary_message_serializer::write_msg_begin(
      OutputStream& output,
      message_type msg_type)
throw (io_exception)
{
   stream::write_as(
            output,
            native_to_big<std::uint16_t>(msg_type_to_code(msg_type)));
}

template <typename OutputStream>
void
binary_message_serializer::write_msg_end(
      OutputStream& output)
throw (io_exception)
{
   stream::write_as(output, native_to_big<std::uint16_t>(0x0d0a));
}

template <typename OutputStream>
void
binary_message_serializer::write_string_value(
      OutputStream& output,
      const std::string& value)
throw (io_exception)
{
   stream::write_as(output, native_to_big<std::uint16_t>(value.length()));
   stream::write_as_string(output, value);
}

template <typename OutputStream>
void
binary_message_serializer::write_uint8_value(
      OutputStream& output,
      std::uint8_t value)
throw (io_exception)
{
   stream::write_as(output, value);
}

template <typename OutputStream>
void
binary_message_serializer::write_uint16_value(
      OutputStream& output,
      std::uint16_t value)
throw (io_exception)
{
   stream::write_as(output, native_to_big<std::uint16_t>(value));
}

template <typename OutputStream>
void
binary_message_serializer::write_uint32_value(
      OutputStream& output,
      std::uint32_t value)
throw (io_exception)
{
   stream::write_as(output, native_to_big<std::uint32_t>(value));
}

template <typename OutputStream>
void
binary_message_serializer::write_float_value(
      OutputStream& output,
      float value)
throw (io_exception)
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
message_type
binary_message_deserializer::read_msg_begin(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   return code_to_msg_type(
            big_to_native(stream::read_as<std::uint16_t>(input)));
}

template <typename InputStream>
void
binary_message_deserializer::read_msg_end(InputStream& input)
throw (protocol_exception, io_exception)
{
   auto eol = native_to_big(stream::read_as<uint16_t>(input));
   if (eol != 0x0d0a)
      OAC_THROW_EXCEPTION(invalid_termination_mark()
            .with_termination_mark(eol));
}

template <typename InputStream>
std::string
binary_message_deserializer::read_string_value(InputStream& input)
throw (protocol_exception, io_exception)
{
   auto str_len = big_to_native(stream::read_as<std::uint16_t>(input));
   return stream::read_as_string(input, str_len);
}

template <typename InputStream>
std::uint16_t
binary_message_deserializer::read_uint16_value(InputStream& input)
throw (protocol_exception, io_exception)
{
   return big_to_native(stream::read_as<std::uint16_t>(input));
}

template <typename InputStream>
std::uint8_t
binary_message_deserializer::read_uint8_value(InputStream& input)
throw (protocol_exception, io_exception)
{
   return stream::read_as<std::uint8_t>(input);
}

template <typename InputStream>
std::uint32_t
binary_message_deserializer::read_uint32_value(
      InputStream& input)
throw (protocol_exception, io_exception)
{
   return big_to_native(stream::read_as<std::uint32_t>(input));
}

template <typename InputStream>
float
binary_message_deserializer::read_float_value(
      InputStream& input)
throw (protocol_exception, io_exception)
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
