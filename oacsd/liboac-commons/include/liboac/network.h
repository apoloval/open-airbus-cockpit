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

#ifndef OAC_NETWORK_H
#define OAC_NETWORK_H

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "stream.h"
#include "worker.h"

namespace oac {

namespace network {

OAC_DECL_ERROR(connection_closed_error, illegal_state_error);

} // namespace network

class tcp_connection
{
public:

   typedef sync_read_stream_adapter<boost::asio::ip::tcp::socket> input_stream;

   typedef sync_write_stream_adapter<boost::asio::ip::tcp::socket> output_stream;

   inline tcp_connection()
      : _socket(new boost::asio::ip::tcp::socket(_io_service))
   {}

   inline boost::asio::io_service& io_service()
   { return _io_service; }

   inline boost::asio::ip::tcp::socket& socket()
   { return *_socket; }

   inline ptr<input_stream> input()
   { return new input_stream(_socket); }

   inline ptr<output_stream> output()
   { return new output_stream(_socket); }

private:

   boost::asio::io_service _io_service;
   ptr<boost::asio::ip::tcp::socket> _socket;
};

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
   inline tcp_server(std::uint16_t port, const Worker& worker)
      : _worker(worker),
        _acceptor(_io_service,
                  boost::asio::ip::tcp::endpoint(
                     boost::asio::ip::tcp::v4(), port))
   {}

   /**
    * Run the server. It uses the current thread to execute the accept loop.
    * It stops running after a call to stop().
    */
   inline void run()
   {
      _io_service.reset();
      start_accept();
      _is_started.notify_all();
      _io_service.run();
   }

   /**
    * Run the server in background. It creates a background thread which
    * executes the accept loop. It returns the control once the thread is
    * sucessfuly created and accept loop is started. Any immediate subsequent
    * call to stop shall work.
    */
   inline void run_in_background()
   {
      _bg_server = boost::thread([this]() { run(); });
      {
         boost::mutex mutex;
         boost::unique_lock<boost::mutex> lock(mutex);
         _is_started.wait(lock);
      }
   }

   /**
    * Stop the server. If running in background, it waits for the background
    * thread to finish before returning.
    */
   inline void stop()
   {
      _io_service.stop();
      _bg_server.join(); // if not in background, returns immediately
   }

private:

   Worker _worker;
   boost::asio::io_service _io_service;
   boost::asio::ip::tcp::acceptor _acceptor;
   boost::thread _bg_server;
   boost::condition_variable _is_started;

   void start_accept()
   {
      if (_io_service.stopped())
         return;
      auto conn = make_ptr(new tcp_connection());
      _acceptor.async_accept(
               conn->socket(), std::bind(&tcp_server::on_accept, this, conn));
   }

   void on_accept(const ptr<tcp_connection>& conn)
   {
      _worker(conn);
      start_accept();
   }
};

class tcp_client
{
public:

   inline tcp_client(const std::string& hostname, std::uint16_t port)
   {
      using namespace boost::asio;

      _connection = new tcp_connection();
      ip::tcp::resolver resolver(_connection->io_service());
      ip::tcp::resolver::query query(
            hostname, boost::lexical_cast<std::string>(port));
      connect(_connection->socket(), resolver.resolve(query));
   }

   inline tcp_connection& connection() throw (network::connection_closed_error)
   { return *_connection; }

   inline ptr<tcp_connection::input_stream> input()
   { return _connection->input(); }

   inline ptr<tcp_connection::output_stream> output()
   { return _connection->output(); }

private:

   ptr<tcp_connection> _connection;
};

namespace network {

typedef std::function<void(const ptr<tcp_connection>&)> connection_handler;
typedef thread_worker<network::connection_handler,
                      ptr<tcp_connection>> dedicated_thread_connection_handler;

/**
 * Create a new TCP server supported by a threaded worker.
 */
template <typename Worker>
ptr<tcp_server<thread_worker<Worker, ptr<tcp_connection>>>> make_tcp_server(
      std::uint16_t port, const Worker& worker)
{
   return new tcp_server<thread_worker<Worker, ptr<tcp_connection>>>(
         port, worker);
}

} // namespace network

} // namespace oac

#endif // OAC_NETWORK_H
