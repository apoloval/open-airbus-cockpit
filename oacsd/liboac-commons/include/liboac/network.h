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
#include "io.h"
#include "stream.h"
#include "worker.h"

namespace oac {

namespace network {

/**
 * An error while binding a socket.
 */
OAC_EXCEPTION_BEGIN(bind_error, io_exception)
   OAC_EXCEPTION_FIELD(port, std::uint16_t)
   OAC_EXCEPTION_MSG("cannot bind on port %d", port)
OAC_EXCEPTION_END()

/**
 * An error while connecting to a remove peer.
 */
OAC_EXCEPTION_BEGIN(connection_refused, io_exception)
   OAC_EXCEPTION_FIELD(remote_host, std::string)
   OAC_EXCEPTION_FIELD(remote_port, std::uint16_t)
   OAC_EXCEPTION_MSG(
         "connection refused to %s on port %d",
         remote_host,
         remote_port)
OAC_EXCEPTION_END()

typedef std::function<void(const io_exception&)> error_handler;

} // namespace network

/**
 * A synchronous TCP connection. This object is the result of creating a new
 * TCP connection, either after accepting a new client by a server or
 * encapsulated in a client.
 */
class tcp_connection
{
public:

   /**
    * The type of input stream used by TCP connections.
    */
   typedef sync_read_stream_adapter<
         boost::asio::ip::tcp::socket> input_stream;

   /**
    * The type of output stream used by TCP connections.
    */
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
   tcp_server(
         std::uint16_t port,
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

   void on_accept(const ptr<tcp_connection>& conn);
};

/**
 * A synchronous TCP client. It may be constructed indicating a hostname
 * and a port, and after successful connection it wraps a tcp_connection
 * object representing the communication with the other TCP peer.
 */
class tcp_client
{
public:

   /**
    * Create a new TCP client.
    *
    * @param hostname   The hostname of the remote peer
    * @param port       The port of the remote peer
    */
   tcp_client(
         const std::string& hostname,
         std::uint16_t port)
   throw (network::connection_refused);

   /**
    * Obtain the TCP connection corresponding to this client.
    */
   tcp_connection& connection();

   /**
    * A convenience function to obtain the input stream
    * from the TCP connection.
    */
   ptr<tcp_connection::input_stream> input();

   /**
    * A convenience function to obtain the output stream
    * from the TCP connection.
    */
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
         void(const async_tcp_connection::ptr_type&)> connection_handler;

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
         std::uint16_t port,
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
