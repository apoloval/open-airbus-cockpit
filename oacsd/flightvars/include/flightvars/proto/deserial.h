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

#ifndef OAC_FV_PROTO_DESERIAL_H
#define OAC_FV_PROTO_DESERIAL_H

#include <liboac/io.h>

#include <flightvars/proto/messages.h>

namespace oac { namespace fv { namespace proto {

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
         OAC_THROW_EXCEPTION(invalid_variable_type(var_type));
   }
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
         OAC_THROW_EXCEPTION(invalid_message_type(int(msg_begin)));
   }
}

}}} // namespace oac::fv::proto

#endif
