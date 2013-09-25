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

#ifndef OAC_NETWORK_ASYNC_CONNECTION_H
#define OAC_NETWORK_ASYNC_CONNECTION_H

#include <future>

#include <boost/asio/ip/tcp.hpp>

#include <liboac/attempt.h>
#include <liboac/network/types.h>

namespace oac { namespace network {

typedef std::function<attempt<std::size_t>> async_read_handler;
typedef std::function<attempt<std::size_t>> async_write_handler;

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

   std::string local_to_string() const;

   std::string remote_to_string() const;

   /**
    * Read some bytes from this connection. After calling this function,
    * as soon as one or more bytes are available they will be stored in the
    * StreamBuffer object passed as argument. Then the given ReadHandler
    * will be invoked. This function is not blocking, so the control will
    * be immediately returned to the caller.
    *
    * @param buff    The stream-buffer where incoming data will be stored
    * @param handler The handler that will be invoked once incoming data
    *                is read. It conforms the AsyncReadHandler concept.
    */
   template <typename StreamBuffer, typename AsyncReadHandler>
   void read(StreamBuffer& buff, AsyncReadHandler handler);

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
    *                is written. It conforms the AsyncWriteHandler concept.
    */
   template <typename StreamBuffer, typename AsyncWriteHandler>
   void write(StreamBuffer& buff, AsyncWriteHandler handler);

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
         const attempt<std::size_t>& nbytes);
};

typedef std::shared_ptr<async_tcp_connection> async_tcp_connection_ptr;

}} // namespace oac::network

#include <liboac/network/async_connection.inl>

#endif
