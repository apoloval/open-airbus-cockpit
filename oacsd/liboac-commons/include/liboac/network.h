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

class async_tcp_connection :
      public shared_by_ptr<async_tcp_connection>,
      public std::enable_shared_from_this<async_tcp_connection> {
public:

   typedef boost::asio::ip::tcp::socket socket_type;
   typedef std::shared_ptr<socket_type> socket_ptr;

   class output_stream :
         public shared_by_ptr<output_stream>,
         public std::enable_shared_from_this<output_stream>
   {
   public:

      template <typename ErrorHandler>
      output_stream(
            const socket_ptr& socket,
            const ErrorHandler& ehandler)
         : _socket(socket), _ehandler(ehandler)
      {}

      output_stream(const socket_ptr& socket)
         : _socket(socket),
           _ehandler(
              std::bind(
                 &output_stream::ignore_error, this, std::placeholders::_1))
      {}

      std::size_t write(const void* src, std::size_t count) throw (io_error)
      {
         auto buff = std::make_shared<fixed_buffer>(count);
         buff->write(src, 0, count);
         boost::asio::async_write(
               *_socket,
               boost::asio::buffer(buff->data(), count),
               std::bind(
                     &output_stream::on_write,
                     shared_from_this(),
                     buff,
                     std::placeholders::_1,
                     std::placeholders::_2));
         return count;
      }

      void flush() {}

   private:

      socket_ptr _socket;
      std::function<void(const io_error& error)>_ehandler;

      void on_write(
            const std::shared_ptr<fixed_buffer>& buff,
            const boost::system::error_code& ec,
            std::size_t bytes_written)
      {

      }

      void ignore_error(const io_error&) {}
   };

   async_tcp_connection(boost::asio::io_service& io_service)
      : _socket(new socket_type(io_service))
   {}

   boost::asio::ip::tcp::socket& socket()
   { return *_socket; }

   template <typename ReadHandler>
   void read(const ReadHandler& handler, std::size_t buffer_size = 4096)
   {
      auto buff = std::make_shared<fixed_buffer>(buffer_size);
      read(buff, 0, handler);
   }

   template <typename ErrorHandler>
   output_stream::ptr_type output(const ErrorHandler& ehandler)
   { return output_stream::create(_socket, ehandler); }

   output_stream::ptr_type output()
   { return output_stream::create(_socket); }

private:

   typedef std::function<
         std::size_t(
               buffer_input_stream<fixed_buffer>& input,
               std::size_t nbytes)> read_handler;

   socket_ptr _socket;

   void read(
         const std::shared_ptr<fixed_buffer>& buffer,
         std::uint32_t index,
         const read_handler& handler)
   {
      auto buff_cap = buffer->capacity();
      auto data = ((std::uint8_t*) buffer->data()) + index;
      auto remaining = buff_cap - index;

      if (remaining)
      {
         _socket->async_read_some(
               boost::asio::buffer(data, remaining),
               std::bind(
                     &async_tcp_connection::on_read,
                     shared_from_this(),
                     buffer,
                     index,
                     handler,
                     std::placeholders::_1,
                     std::placeholders::_2));
      }
      else
      {
         /* Buffer exhausted. Allocate a new one and continue reading. */
         auto new_buff = std::make_shared<fixed_buffer>(buff_cap * 4);
         new_buff->copy(*buffer, 0, 0, buff_cap);
         read(new_buff, index, handler);
      }
   }

   void on_read(
         const std::shared_ptr<fixed_buffer>& buffer,
         std::uint32_t index,
         const read_handler& handler,
         const boost::system::error_code& ec,
         std::size_t nbytes)
   {
      auto new_index = index + nbytes;
      auto nconsumed = handler(
            *buffer::make_input_stream(buffer),
            new_index);
      std::shared_ptr<fixed_buffer> new_buff = buffer;
      if (nconsumed)
      {
         new_buff = std::make_shared<fixed_buffer>(buffer->capacity());
         new_buff->copy(*buffer, nconsumed, 0, new_index - nconsumed);
         new_index = 0;
      }
      // TODO: check error code
      if (!ec)
         read(new_buff, new_index, handler);
   }
};

template <typename ConnectionHandler>
class async_tcp_server
{
public:

   /**
    * Creates a new asynchronous TCP server on the given port.
    */
   async_tcp_server(std::uint16_t port, const ConnectionHandler& handler)
      : _acceptor(_io_service,
                  boost::asio::ip::tcp::endpoint(
                     boost::asio::ip::tcp::v4(), port)),
        _handler(handler)
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

   boost::asio::io_service _io_service;
   boost::asio::ip::tcp::acceptor _acceptor;
   boost::thread _bg_server;
   boost::condition_variable _is_started;
   ConnectionHandler _handler;

   struct buffers
   {
      fixed_buffer input_buffer;
      fixed_buffer output_buffer;

      inline buffers() : input_buffer(4096), output_buffer(4096) {}
   };

   std::map<async_tcp_connection, buffers> _buffers;

   void start_accept()
   {
      auto conn = async_tcp_connection::create(_io_service);
      _acceptor.async_accept(
               conn->socket(),
               std::bind(&async_tcp_server::on_accept, this, conn));
   }

   void on_accept(const async_tcp_connection::ptr_type& conn)
   {
      _handler(conn);
      start_accept();
   }
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
