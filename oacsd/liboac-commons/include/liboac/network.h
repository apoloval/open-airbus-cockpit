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

#include <future>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "buffer.h"
#include "io.h"
#include "stream.h"
#include "worker.h"

namespace oac {

namespace network {

/**
 * A TCP protocol port.
 */
typedef std::uint16_t tcp_port;

/**
 * A hostname.
 */
typedef std::string hostname;

/**
 * An error while binding a socket.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(bind_error, io_exception,
   ("cannot bind on port %d", port),
   (port, std::uint16_t));

/**
 * An error while connecting to a remote peer.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(connection_refused, io_exception,
   (
      "connection refused to %s on port %d",
      remote_host,
      remote_port
   ),
   (remote_host, std::string),
   (remote_port, std::uint16_t));

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

   typedef std::shared_ptr<input_stream> input_stream_ptr;

   /**
    * The type of output stream used by TCP connections.
    */
   typedef sync_write_stream_adapter<
         boost::asio::ip::tcp::socket> output_stream;

   typedef std::shared_ptr<output_stream> output_stream_ptr;

   tcp_connection();

   boost::asio::io_service& io_service();

   boost::asio::ip::tcp::socket& socket();

   input_stream_ptr input();

   output_stream_ptr output();

private:

   boost::asio::io_service _io_service;
   std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
};

typedef std::shared_ptr<tcp_connection> tcp_connection_ptr;

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
         const network::hostname& hostname,
         network::tcp_port port)
   throw (network::connection_refused);

   /**
    * Obtain the TCP connection corresponding to this client.
    */
   tcp_connection& connection();

   /**
    * A convenience function to obtain the input stream
    * from the TCP connection.
    */
   tcp_connection::input_stream_ptr input();

   /**
    * A convenience function to obtain the output stream
    * from the TCP connection.
    */
   tcp_connection::output_stream_ptr output();

private:

   tcp_connection_ptr _connection;
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
      public std::enable_shared_from_this<async_tcp_connection> {
public:  

   typedef boost::asio::ip::tcp::socket socket_type;
   typedef std::shared_ptr<socket_type> socket_ptr;     

   async_tcp_connection(
         boost::asio::io_service& io_service,
         const network::error_handler& ehandler);

   async_tcp_connection(boost::asio::io_service& io_service);

   boost::asio::ip::tcp::socket& socket();

   /**
    * Read some bytes from this connection. After calling this function,
    * as soon as one or more bytes are available they will be stored in the
    * StreamBuffer object passed as argument. Then the given ReadHandler
    * will be invoked. This function is not blocking, so the control will
    * be immediately returned to the caller.
    *
    * @param buff    The stream-buffer where incoming data will be stored
    * @param handler The handler that will be invoked once incoming data
    *                is read. It conforms the ReadHandler concept of Boost ASIO
    *                library.
    */
   template <typename StreamBuffer, typename ReadHandler>
   void read(StreamBuffer& buff, ReadHandler handler);

   /**
    * Read some bytes from this connection. After calling this function,
    * as soon as one or more bytes are available they will be stored in the
    * StreamBuffer object passed as argument. The returned value represents
    * the future result of the read operation. This function is not blocking,
    * so the control will be immediately returned to the caller.
    *
    * @param buff The stream-buffer where incoming data will be stored
    * @return     The future object representing the number of bytes read
    *             from the connection, or io_error exception if failed
    */
   template <typename StreamBuffer>
   std::future<std::size_t> read(StreamBuffer& buff);

   /**
    * Write some bytes to this connection. After calling this function,
    * as may bytes as possible will be transfered from the given buffer
    * into this connection. Then the given WriteHandler will be invoked.
    * This function is not blocking, so the control will be immediately
    * returned to the caller.
    *
    * @param buff    The stream-buffer where outcoming data will be read
    * @param handler The handler that will be invoked once incoming data
    *                is written. It conforms the WriteHandler concept of
    *                Boost ASIO library.
    */
   template <typename StreamBuffer, typename WriteHandler>
   void write(StreamBuffer& buff, WriteHandler handler);

   /**
    * Write some bytes to this connection. After calling this function,
    * as may bytes as possible will be transfered from the given buffer
    * into this connection. The returned value represents the future result
    * of the write operation. This function is not blocking, so the control
    * will be immediately returned to the caller.
    *
    * @param buff The stream-buffer where outcoming data will be read
    * @return     The future object representing the number of bytes written
    *             to the connection, or io_error exception if failed
    */
   template <typename StreamBuffer>
   std::future<std::size_t> write(StreamBuffer& buff);

private:

   socket_ptr _socket;

   static void on_io_completed_with_promise(
         const std::shared_ptr<std::promise<std::size_t>>& promise,
         const boost::system::error_code& ec,
         std::size_t bytes_read);
};

typedef std::shared_ptr<async_tcp_connection> async_tcp_connection_ptr;

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

namespace network {

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

} // namespace network

} // namespace oac

#include "network.inl"

#endif // OAC_NETWORK_H
