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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_NETWORK_ASYNC_CLIENT_H
#define OAC_NETWORK_ASYNC_CLIENT_H

#include <boost/asio/io_service.hpp>

#include <liboac/network/async_connection.h>
#include <liboac/network/errors.h>
#include <liboac/network/types.h>

namespace oac { namespace network {

/**
 * An asynchronous TCP client. This class provides an async TCP client
 * based on Boost ASIO library. It provides some glue code to create a
 * async_tcp_connection communicating with a remote server.
 *
 * Important note! This class operates on a boost::asio::io_service object
 * passed as argument to its constructor. It has no control over the lifecycle
 * of either the IO service object or the thread executing on its event loop.
 * Therefore, before destroying the async_tcp_client object, you must be sure
 * that IO service is stopped and the thread executing its event loop is not
 * processing any request. Otherwise the internal handlers of async_tcp_client
 * may execute on an already destroyed object.
 */
class async_tcp_client
{
public:

   /**
    * Create a new async TCP client and connects to the server using the
    * given paratemers.
    *
    * @param hostname   The hostname of the remote server
    * @param port       The port of the remove server
    * @param io_srv     The IO service to use for async IO
    */
   async_tcp_client(
         const network::hostname& hostname,
         network::tcp_port port,
         const std::shared_ptr<boost::asio::io_service>& io_srv)
   throw (network::connection_refused);

   /**
    * Obtain the connection object of this client.
    */
   async_tcp_connection& connection()
   { return _connection; }

private:

   std::shared_ptr<boost::asio::io_service> _io_service;
   async_tcp_connection _connection;
};

}} // namespace oac::network

#include <liboac/network/async_client.inl>

#endif
