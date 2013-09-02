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
 * An exception indicating a protocol error.
 */
OAC_ABSTRACT_EXCEPTION(protocol_exception);

/**
 * A 16-bits number indicating the version of the protocol.
 */
typedef std::uint16_t protocol_version;

/**
 * The name of a peer that communicates using the protocol.
 */
typedef std::string peer_name;

/**
 * The current protocol version implemented by this codebase.
 */
extern const protocol_version CURRENT_PROTOCOL_VERSION;

/**
 * An enumeration for the different types or messages that comprise the
 * protocol.
 */
enum class message_type
{
   BEGIN_SESSION,
   END_SESSION,
   SUBSCRIPTION_REQ,
   SUBSCRIPTION_REP,
   UNSUBSCRIPTION_REQ,
   UNSUBSCRIPTION_REP,
   VAR_UPDATE
};

/**
 * This message is sent by the client when initiates the session and the
 * server as response to that. It indicates the endpoint name and the
 * protocol version it implements.
 */
struct begin_session_message
{
   peer_name pname;
   protocol_version proto_ver;

   begin_session_message(
         const peer_name& pname,
         protocol_version proto_ver = CURRENT_PROTOCOL_VERSION);
};

/**
 * This message is sent by either server or client when it wants to close
 * the session. The cause field indicates the cause for the session to be ended.
 */
struct end_session_message {
   std::string cause;

   end_session_message(const std::string& cause);
};

/**
 * This message is sent by the client to request a new subscription for a
 * variable. The server responds with a subscription reply message.
 */
struct subscription_request_message
{
   variable_group var_grp;
   variable_name var_name;

   subscription_request_message(
         const variable_group& grp,
         const variable_name& name);
};

/**
 * The status of a variable subscription. This value is sent as part of
 * subscription and unsubscription replies indicating what's the status
 * of the subscription after the corresponding request was processed.
 */
enum class subscription_status
{
   SUBSCRIBED,
   UNSUBSCRIBED,
   NO_SUCH_VAR,
   NO_SUCH_SUBSCRIPTION
};

/**
 * Convert a subscription status value into string.
 */
std::string to_string(subscription_status status);

/**
 * Stream operator for subscription status.
 */
std::ostream& operator <<(
      std::ostream& s,
      subscription_status status);

/**
 * This message is sent by the server as response to a subscription request.
 */
struct subscription_reply_message
{   
   subscription_status st;
   variable_group var_grp;
   variable_name var_name;
   subscription_id subs_id;
   std::string cause;

   subscription_reply_message(
         subscription_status st,
         const variable_group& grp,
         const variable_name& name,
         const subscription_id& subs,
         const std::string& cause);
};

/**
 * This message is sent by the client to request a variable unsubscription.
 */
struct unsubscription_request_message
{
   subscription_id subs_id;

   unsubscription_request_message(subscription_id subs);
};

/**
 * This messag eis sent by the server as response to unsubscription request.
 */
struct unsubscription_reply_message
{
   subscription_status st;
   subscription_id subs_id;
   std::string cause;

   unsubscription_reply_message(
         subscription_status st,
         subscription_id subs_id,
         std::string cause);
};

/**
 * This message is sent by either server or client to report a variable update.
 * The server sends this message when the the client had subscribed to that
 * variable before and the value of that variable has been changed. The client
 * sends this message to the server when it wants the variable to be updated.
 * There is no response to this message to keep a good performance and reduce
 * the peer complexity. In case of client sending a var update message for an
 * unexisting variable, the server will simply ignore it.
 */
struct var_update_message
{
   subscription_id subs_id;
   variable_value var_value;

   var_update_message(
         const subscription_id& subs,
         const variable_value& value);
};

/**
 * This union wraps all kinds of messages into a single one.
 */
typedef boost::variant<
      begin_session_message,
      end_session_message,
      subscription_request_message,
      subscription_reply_message,
      unsubscription_request_message,
      unsubscription_reply_message,
      var_update_message
> message;

/**
 * Serialize given message into given output stream.
 */
template <typename Serializer, typename OutputStream>
void serialize(
      const message& msg,
      OutputStream& output)
throw (io_exception);

template <typename Deserializer, typename InputStream>
message deserialize(
      InputStream& input)
throw (protocol_exception, io_exception);

struct binary_message_serializer
{

   template <typename OutputStream>
   static void write_msg_begin(
         OutputStream& output,
         message_type msg_type)
   throw (io_exception);

   template <typename OutputStream>
   static void write_msg_end(
         OutputStream& output)
   throw (io_exception);

   template <typename OutputStream>
   static void write_string_value(
         OutputStream& output,
         const std::string& value)
   throw (io_exception);

   template <typename OutputStream>
   static void write_uint8_value(
         OutputStream& output,
         std::uint8_t value)
   throw (io_exception);

   template <typename OutputStream>
   static void write_uint16_value(
         OutputStream& output,
         std::uint16_t value)
   throw (io_exception);

   template <typename OutputStream>
   static void write_uint32_value(
         OutputStream& output,
         std::uint32_t value)
   throw (io_exception);

   template <typename OutputStream>
   static void write_float_value(
         OutputStream& output,
         float value)
   throw (io_exception);
};

struct binary_message_deserializer
{
   template <typename InputStream>
   static message_type read_msg_begin(
         InputStream& input)
   throw (protocol_exception);

   template <typename InputStream>
   static void read_msg_end(
         InputStream& input)
   throw (protocol_exception);

   template <typename InputStream>
   static std::string read_string_value(
         InputStream& input)
   throw (protocol_exception);

   template <typename InputStream>
   static std::uint8_t read_uint8_value(
         InputStream& input)
   throw (protocol_exception);

   template <typename InputStream>
   static std::uint16_t read_uint16_value(
         InputStream& input)
   throw (protocol_exception);

   template <typename InputStream>
   static std::uint32_t read_uint32_value(
         InputStream& input)
   throw (protocol_exception);

   template <typename InputStream>
   static float read_float_value(
         InputStream& input)
   throw (protocol_exception);
};

}}} // namespace oac::fv::proto

#include "protocol.inl"

#endif
