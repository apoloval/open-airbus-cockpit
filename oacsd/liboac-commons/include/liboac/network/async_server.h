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

#ifndef OAC_NETWORK_ASYNC_SERVER_H
#define OAC_NETWORK_ASYNC_SERVER_H

#include <boost/thread/thread.hpp>

#include <liboac/network/async_connection.h>

namespace oac { namespace network {

/**
 * An asynchronous TCP server. This class provides an async TCP server
 * based on Boost ASIO library. It provides some glue code to make it
 * easier to manage incoming connections.
 *
 * Important note! This class operates on a boost::asio::io_service object
 * passed as argument to its constructor. It has no control over the lifecycle
 * of either the IO service object or the thread executing on its event loop.
 * Therefore, before destroying the async_tcp_server object, you must be sure
 * that IO service is stopped and the thread executing its event loop is not
 * processing any request. Otherwise the internal handlers of async_tcp_server
 * may execute on an already destroyed object.
 */
class async_tcp_server
{
public:

   /**
    * A handler able to process new TCP connections once they are received.
    */
   typedef std::function<
         void(const async_tcp_connection_ptr&)> connection_handler;

   /**
    * Creates a new asynchronous TCP server.
    *
    * @param port       The TCP port the server will be bounded
    * @param handler    The handler to be invoked when a new connection arrives
    * @param io_srv     The Boost IO service to use for handling IO operations
    * @param ehandler   The handler to be invoked when an error occurs while
    *                   waiting for a connection
    */
   async_tcp_server(
         network::tcp_port port,
         const connection_handler& handler,
         const std::shared_ptr<boost::asio::io_service>& io_srv,
         const network::error_handler& ehandler)
   throw (network::bind_error);

   /**
    * Obtain the IO service used by this server.
    */
   const boost::asio::io_service& io_service() const;

   /**
    * Obtain the IO service used by this server.
    */
   boost::asio::io_service& io_service();

private:

   std::shared_ptr<boost::asio::io_service> _io_service;
   boost::asio::ip::tcp::acceptor _acceptor;
   boost::thread _bg_server;
   boost::condition_variable _is_started;
   connection_handler _handler;
   network::error_handler _ehandler;

   void start_accept();

   void on_accept(const async_tcp_connection_ptr& conn);
};

}} // namespace oac::network

#include <liboac/network/async_server.inl>

#endif
