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

const int flight_vars_server::DEFAULT_PORT(8642);

flight_vars_server::flight_vars_server(
      const ptr<flight_vars>& delegate, int port)
   : _delegate(delegate),
     _io_service(),
     _acceptor(_io_service,
               boost::asio::ip::tcp::endpoint(
                     boost::asio::ip::tcp::v4(),
                     port))
{
   log(log_level::INFO,
       boost::format("@server; Initializing on port %d") % port);
   if (!_delegate)
      _delegate = flight_vars_core::instance();
}

void
flight_vars_server::run()
{
   accept_connection();
   _io_service.run();
}

void
flight_vars_server::accept_connection()
{
   std::shared_ptr<connection> conn(new connection(_io_service, _delegate));
   _acceptor.async_accept(
            conn->get_socket(),
            std::bind(&flight_vars_server::on_connection_accepted, this, conn));
}

void
flight_vars_server::on_connection_accepted(const ptr<connection>& conn)
{
   conn->start();
   accept_connection();
}

flight_vars_server::connection::connection(
      boost::asio::io_service &io_service,
      const ptr<oac::fv::flight_vars> &delegate)
   : _socket(io_service), _delegate(delegate)
{
}

void
flight_vars_server::connection::start()
{
   log(log_level::INFO, "@server, received a new connection");

}

}} // namespace oac::fv
