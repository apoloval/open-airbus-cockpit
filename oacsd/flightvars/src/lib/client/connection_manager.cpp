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

#include "client/connection_manager.h"

namespace oac { namespace fv { namespace client {

connection_manager::connection_manager(
      const std::string& client_name,
      const network::hostname& server_host,
      network::tcp_port server_port,
      const error_handler& ehandler)
throw (communication_error)
   : logger_component("connection_manager"),
     _error_handler(ehandler),
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

connection_manager::~connection_manager()
{
   log_info("Starting FlightVars client shutdown");
   stop_io_service_thread();
   close();
   log_info("FlightVars client shutdown completed");
}

void
connection_manager::submit(
      const subscription_request_ptr& req)
{
   log_info(
         "Requesting subscription for variable %s",
         var_to_string(req->var_id()));
   _io_service->post(std::bind(
         &connection_manager::on_subscription_requested,
         this,
         req));
}

void
connection_manager::submit(
      const unsubscription_request_ptr& req)
{
   log_info("Requesting unsubscription for %d", req->virtual_subs_id());
   _io_service->post(std::bind(
         &connection_manager::on_unsubscription_requested,
         this,
         req));
}

void
connection_manager::submit(
      const variable_update_request_ptr& req)
{
   log_info(
         "Sending a variable update for virtual subscription %d",
         req->virtual_subs_id());
   _io_service->post(std::bind(
         &connection_manager::on_variable_update_requested,
         this,
         req));
}

void
connection_manager::handshake(
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
            OAC_THROW_EXCEPTION(communication_error(e));
         }
         // Else: not enough bytes, read again
      }
      catch (const io_exception& e)
      {
         log_error(
               "IO error while reading response from the server:\n%s",
               e.report());
         OAC_THROW_EXCEPTION(communication_error(e));
      }
      catch (const proto::protocol_exception& e)
      {
         log_error(
               "protocol error while reading response from the server:\n%s",
               e.report());
         OAC_THROW_EXCEPTION(communication_error(e));
      }
   }
}

void
connection_manager::close()
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
connection_manager::start_receive()
{
   _client.connection().read(
         _input_buffer,
         std::bind(
               &connection_manager::on_message_received,
               this,
               std::placeholders::_1,
               std::placeholders::_2));
}

void
connection_manager::run_io_service_thread()
{
   _client_thread = std::thread([this](){
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
      log_info("Connection manager thread is done: exiting");
   });
}

void
connection_manager::stop_io_service_thread()
{
   log_info("Stopping IO service thread");
   _io_service->stop();
   if (_client_thread.joinable())
   {
      _client_thread.join();
   }
}

void
connection_manager::on_subscription_requested(
      const subscription_request_ptr& req)
{
   auto var_id = req->var_id();
   if (_db.entry_defined(var_id))
   {
      try
      {
         auto virt_subs_id = _db.add_virtual_subscription(
               var_id,
               req->handler());
         log_info(
               "Master subscription for variable %s found: "
               "binding new virtual subscription with ID %d ",
               var_to_string(var_id),
               virt_subs_id);
         req->set_result(virt_subs_id);
      }
      catch (const subscription_db::no_such_element_exception& e)
      {
         req->set_error(e);
      }
   }
   else
   {
      log_info(
            "No master subscription found for variable %s: "
            "requesting subscription to the server",
            var_to_string(var_id));
      _request_pool.insert(req);
      auto req = proto::subscription_request_message(
            get_var_group(var_id), get_var_name(var_id));
      send_message(req);
   }
}

void
connection_manager::on_unsubscription_requested(
      const unsubscription_request_ptr& req)
{
   auto virt_subs_id = req->virtual_subs_id();
   try
   {
      auto master_subs_id = _db.get_master_subscription_id(virt_subs_id);
      req->update_master_subs_id(master_subs_id);
      log_info("Removing subscription with virtual ID %d", virt_subs_id);

      if (_db.remove_virtual_subscription(virt_subs_id))
      {
         log_info(
               "No more virtual subscriptions for master %d: "
               "requesting unsubscription to server",
               master_subs_id);

         _request_pool.insert(req);

         auto req = proto::unsubscription_request_message(master_subs_id);
         send_message(req);
      }
      else
         req->set_result();
   }
   catch (const subscription_db::no_such_virtual_subscription_error& e)
   {
      log_warn(
            "No such virtual subscription %d was found in subscription DB",
            virt_subs_id);
      req->set_error(
            OAC_MAKE_EXCEPTION(
                  flight_vars::no_such_subscription_error(virt_subs_id, e)));
   }
}

void
connection_manager::on_variable_update_requested(
      const variable_update_request_ptr& req)
{
   auto virt_subs_id = req->virtual_subs_id();
   try
   {
      auto master_subs_id = _db.get_master_subscription_id(virt_subs_id);
      proto::var_update_message msg(master_subs_id, req->var_value());
      send_message(msg);
      req->set_result();
   }
   catch (const subscription_db::no_such_virtual_subscription_error& e)
   {
      log_warn(
            "Cannot send variable update for unknown virtual subscription %d",
            virt_subs_id);
      req->set_error(
            OAC_MAKE_EXCEPTION(
                  flight_vars::no_such_subscription_error(virt_subs_id, e)));
   }
}

void
connection_manager::on_message_received(
      const boost::system::error_code& ec,
      std::size_t bytes_read)
{
   // This function must only be invoked from the internal client thread.
   // Otherwise means that the client is closing and residual read handlers
   // are still present in the IO service. In such case we may harmless ignore
   // the handler. We determine whether this function is invoked from the
   // client thread or not checking the status of the client thread.
   if (_client_thread.get_id() == std::thread::id())
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
                     &connection_manager::on_subscription_reply_received,
                     this,
                     std::placeholders::_1));
         match |= proto::if_message_type<proto::unsubscription_reply_message>(
               msg,
               std::bind(
                     &connection_manager::on_unsubscription_reply_received,
                     this,
                     std::placeholders::_1));
         match |= proto::if_message_type<proto::var_update_message>(
               msg,
               std::bind(
                     &connection_manager::on_variable_update_received,
                     this,
                     std::placeholders::_1));
         if (!match)
            OAC_THROW_EXCEPTION(
                  proto::unexpected_message_error(
                        proto::get_message_type(msg)));
      }
      else
      {
         OAC_THROW_EXCEPTION(boost_asio_error(ec));
      }
   }
   catch (const eof_error&)
   {
      // Not enough bytes while deserialing message
      // Continue to read again
      _input_buffer.reset();
   }
   catch (const oac::exception& e)
   {
      log_error(
            "Unexpected error occurred while receiving a message:\n%s",
            e.report());

      auto comm_error = OAC_MAKE_EXCEPTION(communication_error(e));

      _request_pool.propagate_error(comm_error);
      if (_error_handler)
         _error_handler(comm_error);

      return;
   }
   start_receive();
}

void connection_manager::on_subscription_reply_received(
      const proto::subscription_reply_message& msg)
{
   auto var_id = make_var_id(msg.var_grp, msg.var_name);
   auto subs = _request_pool.pop_subscription_requests(var_id);
   if (subs.empty())
      OAC_THROW_EXCEPTION(
            proto::unexpected_message_error(
                  proto::message_type::SUBSCRIPTION_REP));
   switch (msg.st)
   {
      case proto::subscription_status::NO_SUCH_VAR:
         log_info(
               "Variable %s was not found in server: subscription rejected",
               var_to_string(var_id));
         for (auto& req : subs)
            req->set_error(
                  OAC_MAKE_EXCEPTION(
                        flight_vars::no_such_variable_error(var_id)));
         break;
      case proto::subscription_status::SUBSCRIBED:
         log_info(
               "Successful subscription reply message "
               "received for %s with master subscription ID %d",
               var_to_string(make_var_id(msg.var_grp, msg.var_name)),
               msg.subs_id);
         for (auto& req : subs)
         {
            try
            {
               auto virt_subs_id = _db.entry_defined(var_id) ?
                     _db.add_virtual_subscription(var_id, req->handler()) :
                     _db.create_entry(var_id, msg.subs_id, req->handler());
               req->set_result(virt_subs_id);
            }
            catch (const subscription_db::already_exists_exception& e)
            {
               OAC_THROW_EXCEPTION(communication_error(e));
            }
         }
         break;
      default:
         log_warn(
               "Unexpected subscription status returned by the server: %s",
               to_string(msg.st));
         OAC_THROW_EXCEPTION(
               proto::unexpected_message_error(
                     proto::message_type::SUBSCRIPTION_REQ));
   }
}

void
connection_manager::on_unsubscription_reply_received(
      const proto::unsubscription_reply_message& msg)
{
   auto master_subs_id = msg.subs_id;
   auto unsubs = _request_pool.pop_unsubscription_requests(master_subs_id);
   if (!unsubs.empty())
   {
      switch (msg.st)
      {
         case proto::subscription_status::NO_SUCH_SUBSCRIPTION:
            for (auto& req : unsubs)
            {
               auto error = OAC_MAKE_EXCEPTION(
                     flight_vars::no_such_subscription_error(msg.subs_id));
               req->set_error(error);
            }
            return;
         case proto::subscription_status::UNSUBSCRIBED:
            log_info(
                  "Successful unsubscription reply message "
                  "received with ID %d",
                  msg.subs_id);
            for (auto& req : unsubs)
               req->set_result();
            return;
         default:
            log_warn(
                  "Unexpected subscription status returned by the server: %s",
                  to_string(msg.st));
      }
   }
   OAC_THROW_EXCEPTION(
         proto::unexpected_message_error(
               proto::message_type::UNSUBSCRIPTION_REP));
}

void
connection_manager::on_variable_update_received(
      const proto::var_update_message& msg)
{
   try
   {
      _db.invoke_handlers(msg.subs_id, msg.var_value);
   }
   catch (const subscription_db::no_such_element_exception& e)
   {
      auto ce = OAC_MAKE_EXCEPTION(communication_error(e));
      log_error(
            "Variable update message received for unknown subscription %d\n%s",
            msg.subs_id,
            e.report());
      if (_error_handler)
         _error_handler(ce);
   }
}

template <typename Message>
void
connection_manager::send_message(
      const Message& msg)
{
   auto buff = std::make_shared<linear_buffer>(1024);
   proto::serialize<proto::binary_message_serializer>(msg, *buff);
   send_data(buff);
}

void
connection_manager::send_data(
      const std::shared_ptr<linear_buffer>& output_buff)
{
   _client.connection().write(
         *output_buff,
         std::bind(
               &connection_manager::on_data_sent,
               this,
               output_buff,
               std::placeholders::_1,
               std::placeholders::_2));
}

void
connection_manager::on_data_sent(
      const std::shared_ptr<linear_buffer>& output_buff,
      const boost::system::error_code& ec,
      std::size_t bytes_read)
{
   if (!!ec && _error_handler)
   {
      auto io_error = OAC_MAKE_EXCEPTION(boost_asio_error(ec));
      auto comm_error = OAC_MAKE_EXCEPTION(communication_error(io_error));
      _error_handler(comm_error);
   }
}

}}} // namespace oac::fv::client
