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

#include "client.h"

namespace oac { namespace fv {


flight_vars_client::flight_vars_client(
      const std::string& client_name,
      const network::hostname& server_host,
      network::tcp_port server_port,
      const error_handler& ehandler,
      const std::chrono::seconds& request_timeout)
throw (client::communication_error)
 : logger_component("flight_vars_client"),
   _conn_mngr(client_name, server_host, server_port, ehandler),
   _request_timeout(request_timeout)
{
}

flight_vars_client::~flight_vars_client()
{
}

subscription_id
flight_vars_client::subscribe(
      const variable_id& var,
      const var_update_handler& handler)
throw (no_such_variable_error)
{
   try
   {
      auto req = std::make_shared<client::subscription_request>(var, handler);
      _conn_mngr.submit(req);
      return req->get_result(_request_timeout);
   }
   catch (const client::request_timeout_error& e)
   {
      log_error(
         "Subscription request to variable %s timed out",
         var_to_string(var));
      OAC_THROW_EXCEPTION(client::communication_error()
            .with_cause(e));
   }
}

void
flight_vars_client::unsubscribe(
      const subscription_id& id)
throw (no_such_subscription_error)
{
   try
   {
      auto req = std::make_shared<client::unsubscription_request>(id);
      _conn_mngr.submit(req);
      return req->get_result(_request_timeout);
   }
   catch (const client::request_timeout_error& e)
   {
      log_error("Unsubscription request to %d timed out", id);
      OAC_THROW_EXCEPTION(client::communication_error()
            .with_cause(e));
   }
}

void
flight_vars_client::update(
      const subscription_id& subs_id,
      const variable_value& var_value)
throw (no_such_subscription_error, illegal_value_error)
{
   try
   {
      auto req = std::make_shared<client::variable_update_request>(
            subs_id,
            var_value);
      _conn_mngr.submit(req);
      return req->get_result(_request_timeout);
   }
   catch (const client::request_timeout_error& e)
   {
      log_error(
         "Variable update request for subscription %d timed out",
         subs_id);
      OAC_THROW_EXCEPTION(client::communication_error()
            .with_cause(e));
   }
}

}} // namespace oac::fv
