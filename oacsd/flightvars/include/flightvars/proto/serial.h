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

#ifndef OAC_FV_PROTO_SERIAL_H
#define OAC_FV_PROTO_SERIAL_H

#include <liboac/io.h>

#include <flightvars/proto/messages.h>

namespace oac { namespace fv { namespace proto {

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

/**
 * Serialize given message into given output stream.
 */
template <typename Serializer, typename OutputStream>
void
serialize(
      const message& msg,
      OutputStream& output)
throw (io_exception)
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


}}} // namespace oac::fv::proto

#endif
