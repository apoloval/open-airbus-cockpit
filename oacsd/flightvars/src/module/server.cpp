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
      const std::shared_ptr<flight_vars>& delegate, int port)
   : _delegate(delegate),
     _tcp_server(
        port,
        std::bind(
           &flight_vars_server::accept_connection,
           this,
           std::placeholders::_1))
{
   log(log_level::INFO,
       boost::format("@server; Initialized on port %d") % port);
   if (!_delegate)
      _delegate = flight_vars_core::instance();
}

flight_vars_server::~flight_vars_server()
{
   log(log_level::INFO, "@server; stopping service");
   _tcp_server.stop();

   /*
    * This invocation to reset() is pretty important. For any reason I cannot
    * understand, the destructor of _delegate member is not invoked in MSVC11,
    * preventing the delegate object to be destroyed. I tried almost anything
    * to find out the cause of this unexpected behavior without success. The
    * most effective workaround I've found is to reset the shared pointer
    * explicitely.
    */
   _delegate.reset();
}

flight_vars_server::session::~session()
{
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
      const async_tcp_connection::ptr_type& conn)
{
   auto session = session::create(shared_from_this(), conn);
   read_begin_session(session);
}

void
flight_vars_server::read_begin_session(
      const session::ptr_type& session)
{
   session->conn->read(
            *session->input_buffer,
            std::bind(
               &flight_vars_server::on_read_begin_session,
               shared_from_this(),
               session,
               std::placeholders::_1,
               std::placeholders::_2));
}

void
flight_vars_server::on_read_begin_session(
      const session::ptr_type& session,
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
{
   using namespace proto;

   if (bytes_transferred == 0)
   {
      log(log_level::WARN,
          "@server; EOF while expecting begin session message");
      return;
   }
   try
   {
      auto msg = unmarshall(*session->input_buffer);
      if (auto* bs_msg = boost::get<begin_session_message>(&msg))
      {
         log(log_level::INFO,
             boost::format("@server; New client %s with protocol %d.%d") %
                  bs_msg->pname %
                  (bs_msg->proto_ver >> 8) %
                  (bs_msg->proto_ver & 0x00ff));
         auto rep = begin_session_message(PEER_NAME);
         write_message(
                  session->conn,
                  rep,
                  std::bind(
                     &flight_vars_server::read_request,
                     shared_from_this(),
                     session));
      } else
      {
         log(log_level::WARN,
             "@server; Protocol error: unexpected message "
             "while expecting begin session");
      }
   }
   catch (stream::eof_error&)
   {
      // message partially received, try to obtain more bytes
      session->input_buffer->reset();
      read_begin_session(session);
   }
}

void
flight_vars_server::read_request(
      const session::ptr_type& session)
{
   try
   {
      session->conn->read(
               *session->input_buffer,
               std::bind(
                  &flight_vars_server::on_read_request,
                  shared_from_this(),
                  session,
                  std::placeholders::_1,
                  std::placeholders::_2));
   }
   catch (...)
   {
      log(log_level::FAIL,
          boost::format(
             "@server; unexpected exception thrown while "
             "waiting for a request"));
   }
}

void
flight_vars_server::on_read_request(
      const session::ptr_type& session,
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
{
   using namespace proto;

   try
   {
      auto msg = unmarshall(*session->input_buffer);
      if (auto es_msg = boost::get<proto::end_session_message>(&msg))
      {
         session->unsubscribe_all();
         log(log_level::INFO,
             boost::format("@server; Session closed by peer (%s)") %
             es_msg->cause);
         return;
      } else if (auto s_req = boost::get<subscription_request_message>(&msg))
      {
         log(log_level::INFO,
             boost::format("@server; processing subscription request "
                           "for variable %s") %
             var_to_string(make_var_id(s_req->var_grp, s_req->var_name)));
         auto rep = handle_subscription_request(session, *s_req);
         write_message(
                  session->conn,
                  rep,
                  std::bind(
                     &flight_vars_server::read_request,
                     shared_from_this(),
                     session));
      } else {
         log(log_level::WARN,
             "@server; Protocol error: unexpected message while expecting "
             "an end session, supscription request or variable update message");
      }
   }
   catch (stream::eof_error&)
   {
      // message partially received, try to obtain more bytes
      session->input_buffer->reset();
      read_request(session);
   }
   catch (...)
   {
      log(log_level::FAIL,
          boost::format(
             "@server; unexpected exception thrown while "
             "waiting for a request"));
   }
}

proto::subscription_reply_message
flight_vars_server::handle_subscription_request(
      const session::ptr_type& session,
      const proto::subscription_request_message& req)
{
   try
   {
      auto var_id = make_var_id(req.var_grp, req.var_name);
      auto subs_id = _delegate->subscribe(
               var_id,
               std::bind(
                  &flight_vars_server::send_var_update,
                  shared_from_this(),
                  session,
                  std::placeholders::_1,
                  std::placeholders::_2));
      log(log_level::INFO, boost::format("@server; subscription for %s registered by delegate") % var_to_string(var_id));
      session->subscriptions.register_subscription(var_id, subs_id);
      return proto::subscription_reply_message(
            proto::subscription_reply_message::STATUS_SUCCESS,
            req.var_grp,
            req.var_name,
            subs_id,
            "");
   } catch (flight_vars::unknown_variable_error&)
   {
      return proto::subscription_reply_message(
            proto::subscription_reply_message::STATUS_NO_SUCH_VAR,
            req.var_grp,
            req.var_name,
            0,
            "No such variable defined in FlightVars module; missing plugin?");
   }
}

void
flight_vars_server::send_var_update(
      const session::ptr_type& session,
      const variable_id& var_id,
      const variable_value& var_value)
{
   try
   {
      auto subs_id = session->subscriptions.get_subscription_id(var_id);
      proto::var_update_message msg(subs_id, var_value);
      write_message(session->conn, msg, [](){});
   }
   catch (subscription_mapper::unknown_variable_error&)
   {
      log(log_level::WARN,
          boost::format(
             "@server; Internal state error: a var update was notified for "
             "variable %s, but there is no subscription ID associated with "
             "it") % var_to_string(var_id));
   }
}

void
flight_vars_server::write_message(
      const async_tcp_connection::ptr_type& conn,
      const proto::message& msg,
      const after_write_handler& after_write)
{
   auto buff = linear_buffer::create(1024);
   proto::serialize<proto::binary_message_serializer>(msg, *buff);
   conn->write(
            *buff,
            std::bind(
               &flight_vars_server::on_write_message,
               shared_from_this(),
               buff,
               after_write,
               std::placeholders::_1,
               std::placeholders::_2));
}

void
flight_vars_server::on_write_message(
      const linear_buffer::ptr_type& buffer,
      const after_write_handler& after_write,
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
{
   if (ec)
   {
      log(log_level::FAIL,
          boost::format(
             "@server; an error was returned while writing message: %s")
             % ec.message());
   }
   else if (bytes_transferred == 0)
   {
      log(log_level::WARN,
          "@server; peer disconnected while trying to write a message");
   }
   else
      after_write();
}

}} // namespace oac::fv
