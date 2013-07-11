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

#include "buffer.h"
#include "stream.h"
#include "worker.h"

namespace oac {

namespace network {

/**
 * An error while binding a socket.
 */
OAC_DECL_ERROR(bind_error, invalid_input_error);

/**
 * An attempt to execute an action on a closed connection.
 */
OAC_DECL_ERROR(connection_closed_error, illegal_state_error);

typedef std::function<void(const io_error&)> error_handler;

} // namespace network

class tcp_connection
{
public:

   typedef sync_read_stream_adapter<
         boost::asio::ip::tcp::socket> input_stream;

   typedef sync_write_stream_adapter<
         boost::asio::ip::tcp::socket> output_stream;

   tcp_connection();

   boost::asio::io_service& io_service();

   boost::asio::ip::tcp::socket& socket();

   ptr<input_stream> input();

   ptr<output_stream> output();

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
   tcp_server(std::uint16_t port,
              const Worker& worker) throw (network::bind_error);

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

   void on_accept(const ptr<tcp_connection>& conn);
};

class tcp_client
{
public:

   tcp_client(const std::string& hostname, std::uint16_t port);

   tcp_connection& connection() throw (network::connection_closed_error);

   ptr<tcp_connection::input_stream> input();

   ptr<tcp_connection::output_stream> output();

private:

   ptr<tcp_connection> _connection;
};

/**
 * An asynchronous TCP connection, which uses handlers to perform read and
 * write operations. The connection object wraps two streams, input_stream
 * and output_stream that behave according to InputStream and OutputStream
 * concepts, respectively. They maintaining an internal buffer to store the
 * data pending to receive/send. In case of input_stream, the programmer
 * have not to worry on buffering the input data until a new message is
 * received, since input_stream does that for him.
 */
class async_tcp_connection :
      public shared_by_ptr<async_tcp_connection>,
      public std::enable_shared_from_this<async_tcp_connection> {
public:  

   typedef boost::asio::ip::tcp::socket socket_type;
   typedef std::shared_ptr<socket_type> socket_ptr;   

   async_tcp_connection(
         boost::asio::io_service& io_service,
         const network::error_handler& ehandler);

   async_tcp_connection(boost::asio::io_service& io_service);

   boost::asio::ip::tcp::socket& socket();

   template <typename StreamBuffer, typename ReadHandler>
   void read(StreamBuffer& buff, ReadHandler handler);

   template <typename StreamBuffer, typename WriteHandler>
   void write(StreamBuffer& buff, WriteHandler handler);

private:

   socket_ptr _socket;
};

class async_tcp_server
{
public:

   /**
    * Creates a new asynchronous TCP server on the given port with given
    * connection handler and error handler.
    */
   template <typename ConnectionHandler, typename ErrorHandler>
   async_tcp_server(
         std::uint16_t port,
         const ConnectionHandler& handler,
         const ErrorHandler& ehandler)
   throw (network::bind_error);

   /**
    * Creates a new asynchronous TCP server on the given port with given
    * connection handler.
    */
   template <typename ConnectionHandler>
   async_tcp_server(
         std::uint16_t port,
         const ConnectionHandler& handler)
   throw (network::bind_error);

   ~async_tcp_server();

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

   typedef std::function<
         void(const async_tcp_connection::ptr_type&)> connection_handler;

   boost::asio::io_service _io_service;
   boost::asio::ip::tcp::acceptor _acceptor;
   boost::thread _bg_server;
   boost::condition_variable _is_started;
   connection_handler _handler;
   network::error_handler _ehandler;

   void start_accept();

   void on_accept(const async_tcp_connection::ptr_type& conn);
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
      std::uint16_t port, const Worker& worker);

} // namespace network

} // namespace oac

#include "network.inl"

#endif // OAC_NETWORK_H
