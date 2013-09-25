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

#include <boost/thread/future.hpp>

#include "client/flight_vars_client.h"

namespace oac { namespace fv { namespace client {

flight_vars_client::flight_vars_client(
      const std::string& client_name,
      const network::hostname& server_host,
      network::tcp_port server_port,
      const error_handler& ehandler,
      const boost::chrono::seconds& request_timeout)
throw (communication_error)
   : logger_component("flight_vars_client"),
     _error_handler(ehandler),
     _request_timeout(request_timeout),
     _io_service(std::make_shared<boost::asio::io_service>()),
     _client(server_host, server_port, _io_service),
     _input_buffer(1024)
{
   log_info("Starting FlightVars client initialization");
   handshake(client_name);
   start_receive();
   run_io_service_thread();
   log_info("FlightVars client initialization completed");
}

flight_vars_client::~flight_vars_client()
{
   log_info("Starting FlightVars client shutdown");
   stop_io_service_thread();
   close();
   log_info("FlightVars client shutdown completed");
}

subscription_id
flight_vars_client::subscribe(
      const variable_id& var,
      const var_update_handler& handler)
throw (no_such_variable_error)
{
   _subscription_request.reset(new subscription_request(var, handler));

   auto result = _subscription_request->promise.get_future();

   log_info(
         "Requesting subscription for variable %s",
         var_to_string(var));
   _io_service->post(std::bind(
         &flight_vars_client::on_subscription_requested,
         this));

   if (result.wait_for(_request_timeout) == boost::future_status::timeout)
   {
      log_error(
            "Subscription request for %s timed out",
            var_to_string(var));
      OAC_THROW_EXCEPTION(communication_error());
   }

   try { return result.get(); }
   catch (const no_such_variable_error&)
   {
      log_warn(
            "Cannot subscribe to variable %s: no such variable in server",
            var_to_string(var));
      throw;
   }
   catch (const oac::exception& e)
   {
      log_error(
            "An error occurred while expecting subscription reply:\n%s",
            e.report());
      OAC_THROW_EXCEPTION(communication_error().with_cause(e));
   }
}

void
flight_vars_client::unsubscribe(const subscription_id& id)
{
    _unsubscription_request.reset(new unsubscription_request(id));

   auto result = _unsubscription_request->promise.get_future();

   log_info("Requesting unsubscription for %d", id);
   _io_service->post(std::bind(
         &flight_vars_client::on_unsubscription_requested,
         this));

   if (result.wait_for(_request_timeout) == boost::future_status::timeout)
   {
      log_error("Unsubscription request for %d timed out", id);
      OAC_THROW_EXCEPTION(communication_error());
   }

   try { return result.get(); }
   catch (const no_such_subscription_error& e)
   {
      log_warn(
            "Cannot unsubscribe from %d: no such subscription in server:\n%s",
            id,
            e.report());
      throw;
   }
   catch (const oac::exception& e)
   {
      log_error(
            "An error occurred while expecting unsubscription reply:\n%s",
            e.report());
      OAC_THROW_EXCEPTION(communication_error().with_cause(e));
   }
}

void
flight_vars_client::update(
      const subscription_id& subs_id,
      const variable_value& var_value)
throw (no_such_subscription_error, illegal_value_error)
{

}

void
flight_vars_client::handshake(
      const std::string& client_name)
throw (communication_error)
{
   using namespace proto;
   linear_buffer output_buff(128);

   log_info("Sending begin session message to the server");
   auto begin_session_msg = proto::begin_session_message(client_name);
   serialize<binary_message_serializer>(begin_session_msg, output_buff);
   auto write_result = _client.connection().write(output_buff);

   _io_service->reset();
   _io_service->run();

   write_result.get();

   while (true)
   {
      try
      {
         auto read_result = _client.connection().read(_input_buffer);

         _io_service->reset();
         _io_service->run();

         read_result.get();

         auto msg = deserialize<binary_message_deserializer>(_input_buffer);
         if (auto* bs_msg = boost::get<begin_session_message>(&msg))
         {
            log_info(
                  "Begin session response received from server (%s)",
                  bs_msg->pname);
            break;
         }
         else
         {
            log_error(
                  "Server responded with an unexpected message while "
                  "waiting for a begin session message");
            OAC_THROW_EXCEPTION(communication_error());
         }
      }
      catch (const eof_error& e)
      {
         if (_input_buffer.available_for_read() == 0)
         {
            // The connection was closed and nothing was sent.
            log_warn(
                  "The remote server closed the connection while expecting "
                  "the response to begin session");
            OAC_THROW_EXCEPTION(communication_error()
                  .with_cause(e));
         }
         // Else: not enough bytes, read again
      }
      catch (const io_exception& e)
      {
         log_error(
               "IO error while reading response from the server:\n%s",
               e.report());
         OAC_THROW_EXCEPTION(communication_error()
               .with_cause(e));
      }
      catch (const proto::protocol_exception& e)
      {
         log_error(
               "protocol error while reading response from the server:\n%s",
               e.report());
         OAC_THROW_EXCEPTION(communication_error()
               .with_cause(e));
      }
   }
}

void
flight_vars_client::close()
throw (communication_error)
{
   using namespace proto;
   linear_buffer output_buff(128);

   log_info("Sending end session message to the server");
   auto end_session_msg = proto::end_session_message("Client disconnected");
   serialize<binary_message_serializer>(end_session_msg, output_buff);
   auto write_result = _client.connection().write(output_buff);

   _io_service->reset();
   _io_service->run();

   write_result.get();
}

void
flight_vars_client::start_receive()
{
   _client.connection().read(
         _input_buffer,
         std::bind(
               &flight_vars_client::on_message_received,
               this,
               std::placeholders::_1,
               std::placeholders::_2));
}

void
flight_vars_client::run_io_service_thread()
{
   _client_thread = boost::thread([this](){
      while (true)
      {
         try
         {
            _io_service->reset();
            _io_service->run();
            break;
         }
         catch (const oac::exception& e)
         {
            log_warn(
                  "Unexpected exception caught by flight vars "
                  "client thread:\n%s",
                  e.report());
         }
      }
   });
}

void
flight_vars_client::stop_io_service_thread()
{
   log_info("Stopping IO service thread");
   _io_service->stop();
   if (_client_thread.joinable())
   {
      _client_thread.join();
   }
}

void
flight_vars_client::on_subscription_requested()
{
   auto var_id = _subscription_request->var_id;
   if (_db.entry_defined(var_id))
   {
      try
      {
         auto slave_subs_id = _db.add_slave_subscription(
               var_id,
               _subscription_request->handler);
         log_info(
               "There is already an active subscription for variable %s; "
               "ommiting request to the server and binding a new slave "
               "subscription with ID %d",
               var_id.to_string(),
               slave_subs_id);
         _subscription_request->promise.set_value(slave_subs_id);
         _subscription_request.reset();
      }
      catch (const subscription_db::no_such_element_exception& e)
      {
         _subscription_request->promise.set_exception(
               boost::copy_exception(e));
         _subscription_request.reset();
      }
   }
   else
   {
      auto buff = std::make_shared<linear_buffer>(1024);
      auto req = proto::subscription_request_message(
            get_var_group(var_id), get_var_name(var_id));

      proto::serialize<proto::binary_message_serializer>(req, *buff);

      send_data(buff);
   }
}

void
flight_vars_client::on_unsubscription_requested()
{
   auto slave_subs_id = _unsubscription_request->slave_subs_id;
   try
   {
      auto master_subs_id = _db.get_master_subscription_id(slave_subs_id);
      _unsubscription_request->master_subs_id = master_subs_id;
      if (_db.remove_slave_subscription(slave_subs_id))
      {
         log_info(
               "After unsubscribed from %d, there are no more "
               "subscriptions for master %d: requesting unsubscription "
               "to server",
               slave_subs_id,
               master_subs_id);
         auto buff = std::make_shared<linear_buffer>(1024);
         auto req = proto::unsubscription_request_message(master_subs_id);

         proto::serialize<proto::binary_message_serializer>(req, *buff);

         send_data(buff);
      }
      else
      {
         log_info(
               "After unsubscribed from %d, there are other active slave "
               "subscriptions: unsubscription request deferred",
               slave_subs_id);
         _unsubscription_request->promise.set_value();
         _unsubscription_request.reset();
      }
   }
   catch (const subscription_db::no_such_slave_subscription_error& e)
   {
      _unsubscription_request->promise.set_exception(
            boost::copy_exception(
                  OAC_MAKE_EXCEPTION(flight_vars::no_such_subscription_error()
                        .with_cause(e))));
      _unsubscription_request.reset();
   }
}

void
flight_vars_client::on_message_received(
      const boost::system::error_code& ec,
      std::size_t bytes_read)
{
   // This function must only be invoked from the internal client thread.
   // Otherwise means that the client is closing and residual read handlers
   // are still present in the IO service. In such case we may harmless ignore
   // the handler. We determine whether this function is invoked from the
   // client thread or not checking the status of the client thread.
   if (_client_thread.get_id() == boost::thread::id())
      return;

   try
   {
      if (!ec)
      {
         auto msg = proto::deserialize<proto::binary_message_deserializer>(
               _input_buffer);
         bool match = false;
         match |= proto::if_message_type<proto::subscription_reply_message>(
               msg,
               std::bind(
                     &flight_vars_client::on_subscription_reply_received,
                     this,
                     std::placeholders::_1));
         match |= proto::if_message_type<proto::unsubscription_reply_message>(
               msg,
               std::bind(
                     &flight_vars_client::on_unsubscription_reply_received,
                     this,
                     std::placeholders::_1));
         match |= proto::if_message_type<proto::var_update_message>(
               msg,
               std::bind(
                     &flight_vars_client::on_variable_update_received,
                     this,
                     std::placeholders::_1));
         if (!match)
            OAC_THROW_EXCEPTION(proto::unexpected_message_error()
                  .with_msg_type(proto::get_message_type(msg)));
      }
      else
      {
         OAC_THROW_EXCEPTION(boost_asio_error().with_error_code(ec));
      }
   }
   catch (const eof_error&)
   {
      // Not enough bytes while deserialing message
      // Continue to read again
   }
   catch (const oac::exception& e)
   {
      // Check the different open promises to set the exception
      if (_subscription_request)
         _subscription_request->promise.set_exception(
               boost::copy_exception(e));
      else if (_unsubscription_request)
         _unsubscription_request->promise.set_exception(
               boost::copy_exception(e));
      else
      {
         // If there is no promise, we have to report the error using the
         // error handler provided at client construction.
         log_error(
               "Unexpected error occurred while receiving a message:\n%s",
               e.report());

         if (_error_handler)
         {
            _error_handler(OAC_MAKE_EXCEPTION(communication_error()
                  .with_cause(e)));
         }
      }
      return;
   }
   start_receive();
}

void flight_vars_client::on_subscription_reply_received(
      const proto::subscription_reply_message& msg)
{
   variable_id var_id(msg.var_grp, msg.var_name);
   if (_subscription_request)
   {
      switch (msg.st)
      {
         case proto::subscription_status::NO_SUCH_VAR:
            _subscription_request->promise.set_exception(
                  boost::copy_exception(no_such_variable_error()
                        .with_var_group_tag(msg.var_grp)
                        .with_var_name_tag(msg.var_name)));
            break;
         case proto::subscription_status::SUBSCRIBED:
            try
            {
               auto slave_subs_id = _db.create_entry(
                     var_id,
                     msg.subs_id,
                     _subscription_request->handler);
               log_info(
                     "Successful subscription reply message "
                     "received for %s with ID %d",
                     var_to_string(make_var_id(msg.var_grp, msg.var_name)),
                     msg.subs_id);
               _subscription_request->promise.set_value(slave_subs_id);
            }
            catch (const subscription_db::already_exists_exception& e)
            {
               OAC_THROW_EXCEPTION(communication_error().with_cause(e));
            }
            break;
         default:
            log_warn(
                  "Unexpected subscription status returned by the server: %s",
                  to_string(msg.st));
            OAC_THROW_EXCEPTION(proto::unexpected_message_error()
                  .with_msg_type(proto::message_type::SUBSCRIPTION_REQ));
      }
      _subscription_request.reset();
   }
   else
      OAC_THROW_EXCEPTION(proto::unexpected_message_error()
            .with_msg_type(proto::message_type::SUBSCRIPTION_REP));
}

void
flight_vars_client::on_unsubscription_reply_received(
      const proto::unsubscription_reply_message& msg)
{
   if (_unsubscription_request)
   {
      auto master_subs_id = _unsubscription_request->master_subs_id;
      if (msg.subs_id == master_subs_id)
         switch (msg.st)
         {
            case proto::subscription_status::NO_SUCH_SUBSCRIPTION:
               _unsubscription_request->promise.set_exception(
                     boost::copy_exception(no_such_subscription_error()
                           .with_subs_id(msg.subs_id)));
               _unsubscription_request.reset();
               return;
            case proto::subscription_status::UNSUBSCRIBED:
               log_info(
                     "Successful unsubscription reply message "
                     "received with ID %d",
                     msg.subs_id);
               _unsubscription_request->promise.set_value();
               _unsubscription_request.reset();
               return;
            default:
               log_warn(
                     "Unexpected subscription status returned by the server: %s",
                     to_string(msg.st));
         }
      else
         log_warn(
               "Received unsubscription reply for %d while expecting "
               "unsubscription from %d",
               msg.subs_id,
               master_subs_id);
   }
   OAC_THROW_EXCEPTION(proto::unexpected_message_error()
         .with_msg_type(proto::message_type::UNSUBSCRIPTION_REP));
}

void
flight_vars_client::on_variable_update_received(
      const proto::var_update_message& msg)
{
   try
   {
      _db.invoke_handlers(msg.subs_id, msg.var_value);
   }
   catch (const subscription_db::no_such_element_exception& e)
   {
      auto ce = OAC_MAKE_EXCEPTION(communication_error()
            .with_cause(e));
      log_error(
            "Variable update message received for unknown subscription %d\n%s",
            msg.subs_id,
            e.report());
      if (_error_handler)
         _error_handler(ce);
   }
}

void
flight_vars_client::send_data(
      const std::shared_ptr<linear_buffer>& output_buff)
{
   _client.connection().write(
         *output_buff,
         std::bind(
               &flight_vars_client::on_data_sent,
               this,
               std::placeholders::_1,
               std::placeholders::_2));
}

void
flight_vars_client::on_data_sent(
      const boost::system::error_code& ec,
      std::size_t bytes_read)
{
   // TODO: resolv how to report that request went wrong
}

}}} // namespace oac::fv::client
