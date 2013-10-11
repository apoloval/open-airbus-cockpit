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

#ifndef OAC_FV_CLIENT_H
#define OAC_FV_CLIENT_H

#include <flightvars/client/connection_manager.h>

namespace oac { namespace fv {

/**
 * A FlightVars object that connects to a remote server to serve variables.
 * This class provides a FlightVars-like object that connects to a remote
 * server via TCP protocol.
 */
class flight_vars_client : public flight_vars, public logger_component
{
public:

   // A handler for errors occurred in asynchronous operations.
   typedef std::function<
         void(const client::communication_error&)> error_handler;

   flight_vars_client(
         const std::string& client_name,
         const network::hostname& server_host,
         network::tcp_port server_port,
         const error_handler& ehandler =
               error_handler(),
         const std::chrono::seconds& request_timeout =
               std::chrono::seconds(60))
   throw (client::communication_error);

   virtual ~flight_vars_client();

   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler)
   throw (no_such_variable_error);

   virtual void unsubscribe(
         const subscription_id& id)
   throw (no_such_subscription_error);

   virtual void update(
         const subscription_id& subs_id,
         const variable_value& var_value)
   throw (no_such_subscription_error, illegal_value_error);

   std::future<void> disconnection()
   { return _conn_mngr.disconnection(); }

private:

   client::connection_manager _conn_mngr;
   std::chrono::seconds _request_timeout;
};

}} // namespace oac::fv

#endif
