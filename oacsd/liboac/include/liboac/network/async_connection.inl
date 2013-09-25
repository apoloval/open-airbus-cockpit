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

#ifndef OAC_NETWORK_ASYNC_CONNECTION_INL
#define OAC_NETWORK_ASYNC_CONNECTION_INL

#include <liboac/buffer/functions.h>
#include <liboac/network/async_connection.h>

namespace oac { namespace network {

inline
async_tcp_connection::async_tcp_connection(
      boost::asio::io_service& io_service,
      const network::error_handler& ehandler)
   : _socket(new socket_type(io_service))
{}

inline
async_tcp_connection::async_tcp_connection(
      boost::asio::io_service& io_service)
   : _socket(new socket_type(io_service))
{}

inline boost::asio::ip::tcp::socket&
async_tcp_connection::socket()
{ return *_socket; }

inline std::string
async_tcp_connection::local_to_string() const
{
   auto ep = _socket->remote_endpoint();
   return format(
         "%s:%d",
         ep.address().to_string(),
         ep.port());
}

inline std::string
async_tcp_connection::remote_to_string() const
{
   auto ep = _socket->local_endpoint();
   return format(
         "%s:%d",
         ep.address().to_string(),
         ep.port());
}

template <typename StreamBuffer, typename AsyncReadHandler>
void
async_tcp_connection::read(
      StreamBuffer& buff,
      AsyncReadHandler handler)
{
   buffer::async_read_some(*_socket, buff, handler);
}

template <typename StreamBuffer>
std::future<std::size_t>
async_tcp_connection::read(StreamBuffer& buff)
{
   auto promise = std::make_shared<std::promise<std::size_t>>();
   auto fut = promise->get_future();

   read(
         buff,
         std::bind(
               &async_tcp_connection::on_io_completed_with_promise,
               promise,
               std::placeholders::_1));
   return fut;
}

template <typename StreamBuffer, typename AsyncWriteHandler>
void
async_tcp_connection::write(
      StreamBuffer& buff,
      AsyncWriteHandler handler)
{
   buffer::async_write_some(*_socket, buff, handler);
}

template <typename StreamBuffer>
std::future<std::size_t>
async_tcp_connection::write(StreamBuffer& buff)
{
   auto promise = std::make_shared<std::promise<std::size_t>>();
   auto fut = promise->get_future();

   write(
         buff,
         std::bind(
               &async_tcp_connection::on_io_completed_with_promise,
               promise,
               std::placeholders::_1));
   return fut;
}

inline
void
async_tcp_connection::on_io_completed_with_promise(
      const std::shared_ptr<std::promise<std::size_t>>& promise,
      const attempt<std::size_t>& nbytes)
{
   try
   {
      promise->set_value(nbytes.get_value());
   }
   catch (...)
   {
      promise->set_exception(std::current_exception());
   }
}

}} // namespace oac::network

#endif
