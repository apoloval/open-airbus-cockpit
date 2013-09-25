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

#ifndef OAC_NETWORK_SERVER_H
#define OAC_NETWORK_SERVER_H

#include <liboac/network/connection.h>
#include <liboac/worker.h>

namespace oac { namespace network {

/**
 * A TCP server which submits each new connection to an Worker compliant
 * object.
 */
template <typename Worker>
class tcp_server
{
public:

   /**
    * Creates a new TCP server on the given port, using the given worker
    * to submit the incoming connections.
    */
   tcp_server(
         network::tcp_port port,
         const Worker& worker)
   throw (network::bind_error);

   /**
    * Run the server. It uses the current thread to execute the accept loop.
    * It stops running after a call to stop().
    */
   void run();

   /**
    * Run the server in background. It creates a background thread which
    * executes the accept loop. It returns the control once the thread is
    * sucessfuly created and accept loop is started. Any immediate subsequent
    * call to stop shall work.
    */
   void run_in_background();

   /**
    * Stop the server. If running in background, it waits for the background
    * thread to finish before returning.
    */
   void stop();

private:

   Worker _worker;
   boost::asio::io_service _io_service;
   boost::asio::ip::tcp::acceptor _acceptor;
   boost::thread _bg_server;
   boost::condition_variable _is_started;

   void start_accept();

   void on_accept(const tcp_connection_ptr& conn);
};

typedef std::function<void(const tcp_connection_ptr&)> connection_handler;
typedef thread_worker<
      network::connection_handler,
      tcp_connection_ptr> dedicated_thread_connection_handler;

/**
 * Create a new TCP server supported by a threaded worker.
 */
template <typename Worker>
std::shared_ptr<tcp_server<thread_worker<Worker, tcp_connection_ptr>>>
make_tcp_server(
      network::tcp_port port, const Worker& worker);

}} // namespace oac::network

#include <liboac/network/server.inl>

#endif
