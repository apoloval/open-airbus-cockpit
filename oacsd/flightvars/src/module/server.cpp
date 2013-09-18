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
 * along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "server.h"

#include <liboac/logging.h>

#include "core.h"

namespace oac { namespace fv {

namespace {

template <typename StreamBuffer>
proto::message
unmarshall(StreamBuffer& buff)
{
   buff.set_mark();
   auto result = proto::deserialize<proto::binary_message_deserializer>(buff);
   buff.unset_mark();
   return result;
}

} // anonymous namespace

const int flight_vars_server::DEFAULT_PORT(8642);
const proto::peer_name flight_vars_server::PEER_NAME("FlightVars Server");

flight_vars_server::flight_vars_server(
      const std::shared_ptr<flight_vars>& delegate,
      int port,
      const std::shared_ptr<boost::asio::io_service>& io_srv)
   : logger_component("flight_vars_server"),
     _delegate(delegate),
     _tcp_server(
           port,
           std::bind(
                 &flight_vars_server::accept_connection,
                 this,
                 std::placeholders::_1),
           io_srv,
           network::error_handler())
{
   log(log_level::INFO, "Initialized on port %d", port);
   if (!_delegate)
      _delegate = flight_vars_core::instance();
}

flight_vars_server::~flight_vars_server()
{
   log_info("Stopping service");
}

flight_vars_server::session::~session()
{
   log_info(
      "Terminating session from %s",
      conn->remote_to_string());
   unsubscribe_all();
}

void
flight_vars_server::session::unsubscribe_all()
{
   subscriptions.for_each_subscription([this](const subscription_id& subs)
   {
      server->_delegate->unsubscribe(subs);
   });
   subscriptions.clear();
}

void
flight_vars_server::accept_connection(
      const async_tcp_connection_ptr& conn)
{
   auto s = std::make_shared<session>(shared_from_this(), conn);
   read_begin_session(s);
}

void
flight_vars_server::read_begin_session(
      const session_ptr& session)
{
   session->conn->read(
            *session->input_buffer,
            std::bind(
               &flight_vars_server::on_read_begin_session,
               shared_from_this(),
               session,
               std::placeholders::_1));
}

void
flight_vars_server::on_read_begin_session(
      const session_ptr& session,
      const attempt<std::size_t>& bytes_transferred)
{
   using namespace proto;

   try
   {
      bytes_transferred.get_value();

      auto msg = unmarshall(*session->input_buffer);
      if (auto* bs_msg = boost::get<begin_session_message>(&msg))
      {
         log(
               log_level::INFO,
               "New client %s with protocol %d.%d",
               bs_msg->pname,
               (bs_msg->proto_ver >> 8),
               (bs_msg->proto_ver & 0x00ff));
         auto rep = begin_session_message(PEER_NAME);
         write_message(
                  session->conn,
                  rep,
                  std::bind(
                     &flight_vars_server::read_request,
                     shared_from_this(),
                     session));
      }
      else
      {
         log_warn(
               "Protocol error: unexpected message "
               "while expecting begin session");
      }
   }
   catch (io::eof_error&)
   {
      // message partially received, try to obtain more bytes
      session->input_buffer->reset();
      read_begin_session(session);
   }
   catch (oac::exception& e)
   {
      log_warn(
            "Unexpected exception thrown while "
            "waiting for a begin session message:\n%s",
            e.report());
   }
}

void
flight_vars_server::read_request(
      const session_ptr& session)
{
   try
   {
      session->conn->read(
               *session->input_buffer,
               std::bind(
                  &flight_vars_server::on_read_request,
                  shared_from_this(),
                  session,
                  std::placeholders::_1));
   }
   catch (io_exception& e)
   {
      log_warn(
            "Unexpected IO exception thrown while reading from connection:\n%s",
            e.report());
   }
}

void
flight_vars_server::on_read_request(
      const session_ptr& session,
      const attempt<std::size_t>& bytes_transferred)
{
   using namespace proto;

   try
   {
      auto msg = unmarshall(*session->input_buffer);
      if (auto es_msg = boost::get<proto::end_session_message>(&msg))
      {
         log_info("Session closed by peer (%s)", es_msg->cause);
         return;
      }
      else if (auto s_req = boost::get<subscription_request_message>(&msg))
      {
         log(
               log_level::INFO,
               "Processing subscription request for variable %s",
               var_to_string(make_var_id(s_req->var_grp, s_req->var_name)));
         auto rep = handle_subscription_request(session, *s_req);
         write_message(
                  session->conn,
                  rep,
                  std::bind(
                     &flight_vars_server::read_request,
                     shared_from_this(),
                     session));
      }
      else if (auto us_req = boost::get<unsubscription_request_message>(&msg))
      {
         log_info(
               "Processing unsubscription request for ID %d",
               us_req->subs_id);
         auto rep = handle_unsubscription_request(session, *us_req);
         write_message(
                  session->conn,
                  rep,
                  std::bind(
                     &flight_vars_server::read_request,
                     shared_from_this(),
                     session));
      }
      else if (auto vu_req = boost::get<var_update_message>(&msg))
      {
         handle_var_update_request(*vu_req);
         read_request(session);
      }
      else
      {
         log_warn(
             "Protocol error: unexpected message while expecting "
             "an end session, supscription request or variable update message");
      }
   }
   catch (io::eof_error&)
   {
      // message partially received, try to obtain more bytes
      session->input_buffer->reset();
      read_request(session);
   }
   catch (oac::exception& e)
   {
      log_warn(
            "Unexpected exception thrown while processing a request:\n%s",
            e.report());
   }
}

proto::subscription_reply_message
flight_vars_server::handle_subscription_request(
      const session_ptr& session,
      const proto::subscription_request_message& req)
{
   auto var_id = make_var_id(req.var_grp, req.var_name);
   auto var_str = var_to_string(var_id);
   try
   {
      session_wptr weak_session(session);
      auto subs_id = _delegate->subscribe(
               var_id,
               std::bind(
                  &flight_vars_server::handle_var_update,
                  shared_from_this(),
                  weak_session,
                  std::placeholders::_1,
                  std::placeholders::_2));
      log(
            log_level::INFO,
            "Subscription for %s registered by delegate", var_str);
      session->subscriptions.register_subscription(var_id, subs_id);
      return proto::subscription_reply_message(
            proto::subscription_status::SUBSCRIBED,
            req.var_grp,
            req.var_name,
            subs_id,
            "");
   }
   catch (flight_vars::no_such_variable_error& e)
   {
      log(
            log_level::WARN,
            "cannot register variable subscription: unknown variable %s:\n%s",
            var_str,
            e.report());
      return proto::subscription_reply_message(
            proto::subscription_status::NO_SUCH_VAR,
            req.var_grp,
            req.var_name,
            0,
            "No such variable defined in FlightVars module; missing plugin?");
   }
}

proto::unsubscription_reply_message
flight_vars_server::handle_unsubscription_request(
      const session_ptr& session,
      const proto::unsubscription_request_message& req)
{
   auto subs_id = req.subs_id;
   try
   {
      _delegate->unsubscribe(subs_id);
      log_info("Unsubscription for %d registered by delegate", subs_id);
      session->subscriptions.unregister(subs_id);

      return proto::unsubscription_reply_message(
            proto::subscription_status::UNSUBSCRIBED,
            subs_id,
            "");
   }
   catch (const flight_vars::no_such_subscription_error& e)
   {
      log_warn(
            "cannot unsubscribe from %d: unknown subscription:\n%s",
            subs_id,
            e.report());
      return proto::unsubscription_reply_message(
            proto::subscription_status::NO_SUCH_SUBSCRIPTION,
            subs_id,
            format("No such subscription with ID %d", subs_id));
   }
   catch (const subscription_mapper::no_such_subscription_error& e)
   {
      log_warn(
            "internal inconsistency detected; delegate succeed to "
            "unsubscribe from %d, but subscription mapper indicates that "
            "the subscription doesn't exists:\n%s",
            subs_id,
            e.report());
      return proto::unsubscription_reply_message(
            proto::subscription_status::UNKNOWN,
            subs_id,
            format("Server error while unsubscribing from %d", subs_id));
   }
}

void
flight_vars_server::handle_var_update(
      const session_wptr& session,
      const variable_id& var_id,
      const variable_value& var_value)
{
   // This function does not send the var update directly. Instead, it
   // requests the IO service of the TCP server to do it. That avoids
   // the delegate notification thread to handle the session at the same
   // time the TCP server thread does. In other words, it guarantees only
   // one thread accessing the internal state of the server.
   auto s = session.lock();
   if (s)
   {
      _tcp_server.io_service().post(
            std::bind(
                  &flight_vars_server::send_var_update,
                  shared_from_this(),
                  s,
                  var_id,
                  var_value));
   }
   else
      log_warn(
         "Inconsistency found while sending variable update to remote peer: "
         "the session object has been destroyed but the subscription still "
         "persists");
}

void
flight_vars_server::send_var_update(
      const session_ptr& session,
      const variable_id& var_id,
      const variable_value& var_value)
{
   // This function does send the var update. It is guaranteed to be invoked
   // from the same thread that attends the TCP server, removing any chance
   // of concurrency issues. See the comment in handle_var_update().
   try
   {
      auto subs_id = session->subscriptions.get_subscription_id(var_id);
      proto::var_update_message msg(subs_id, var_value);
      write_message(session->conn, msg, [](){});
   }
   catch (subscription_mapper::no_such_variable_error& e)
   {
      log(
            log_level::WARN,
            "Internal state error: a var update was notified for "
            "variable %s, but no associated subscription ID was found:\n%s",
            var_to_string(var_id),
            e.report());
   }
   catch (io_exception& e)
   {
      log_error(
            "Unexpected IO exception thrown while "
            "sending a var update to the client:\n%s",
            e.report());
   }
}

void
flight_vars_server::write_message(
      const async_tcp_connection_ptr& conn,
      const proto::message& msg,
      const after_write_handler& after_write)
{
   auto buff = std::make_shared<linear_buffer>(1024);
   proto::serialize<proto::binary_message_serializer>(msg, *buff);
   conn->write(
            *buff,
            std::bind(
               &flight_vars_server::on_write_message,
               shared_from_this(),
               buff,
               after_write,
               std::placeholders::_1));
}

void
flight_vars_server::on_write_message(
      const linear_buffer_ptr& buffer,
      const after_write_handler& after_write,
      const attempt<std::size_t>& bytes_transferred)
{
   try
   {
      bytes_transferred.get_value();
      after_write();
   }
   catch (const oac::exception& e)
   {
      log_error(
            "An error was returned while writing message:\n%s", e.report());
   }
}

void
flight_vars_server::handle_var_update_request(
      const proto::var_update_message& req)
{
   try
   {
      _delegate->update(req.subs_id, req.var_value);
   }
   catch (flight_vars::no_such_subscription_error& e)
   {
      log(
            log_level::WARN,
            "Received a var update for unknown subscription ID %d:\n%s",
            req.subs_id,
            e.report());
   }
   catch (flight_vars::illegal_value_error& e)
   {
      log(
            log_level::WARN,
            "Received a var update for subscription ID %d "
            "with invalid value %s:\n%s",
            req.subs_id,
            req.var_value.to_string(),
            e.report());
   }
}

}} // namespace oac::fv
