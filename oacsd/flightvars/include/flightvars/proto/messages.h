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

#ifndef OAC_FV_PROTO_MESSAGES_H
#define OAC_FV_PROTO_MESSAGES_H

#include <boost/variant.hpp>

#include <flightvars/proto/errors.h>
#include <flightvars/proto/types.h>
#include <flightvars/subscription.h>
#include <flightvars/var.h>

namespace oac { namespace fv { namespace proto {

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
         protocol_version proto_ver = FLIGHTVARS_PROTOCOL_VERSION)
      : pname(pname),
        proto_ver(proto_ver)
   {}
};

/**
 * This message is sent by either server or client when it wants to close
 * the session. The cause field indicates the cause for the session to be ended.
 */
struct end_session_message
{
   std::string cause;

   end_session_message(const std::string& cause) : cause(cause) {}
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
         const variable_name& name)
      : var_grp(grp),
        var_name(name)
   {}
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
   NO_SUCH_SUBSCRIPTION,
   VAR_ALREADY_SUBSCRIBED,
   UNKNOWN
};

/**
 * Convert a subscription status value into string.
 */
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
      case subscription_status::VAR_ALREADY_SUBSCRIBED:
         return "var already subscribed";
      case subscription_status::UNKNOWN:
         return "unknown";
      default:
         OAC_THROW_EXCEPTION(
               enum_out_of_range_error<subscription_status>(status));
   }
}

/**
 * Stream operator for subscription status.
 */
inline std::ostream& operator << (
      std::ostream& s,
      subscription_status status)
{ return s << to_string(status); }

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
         const std::string& cause)
      : st(st),
           var_grp(grp),
           var_name(name),
           subs_id(subs),
           cause(cause)
   {}
};

/**
 * This message is sent by the client to request a variable unsubscription.
 */
struct unsubscription_request_message
{
   subscription_id subs_id;

   unsubscription_request_message(subscription_id subs)
      : subs_id(subs)
   {}
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
         std::string cause)
      : st(st),
        subs_id(subs_id),
        cause(cause)
   {}
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
         const variable_value& value)
      : subs_id(subs),
        var_value(value)
   {}
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
 * Obtain the message type for given message.
 */
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

/**
 * Execute the given action if message type matches.
 *
 * @param msg     The message to be matched with given type
 * @param action  The action to be executed on the message if matched
 * @return        True if message type matches, false otherwise
 */
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

template <typename Deserializer, typename InputStream>
message deserialize(
      InputStream& input)
throw (protocol_exception, io_exception);

}}} // namespace oac::fv::proto

#endif
