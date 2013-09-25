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

#ifndef OAC_NETWORK_ASYNC_SERVER_INL
#define OAC_NETWORK_ASYNC_SERVER_INL

#include <liboac/network/asio_utils.h>
#include <liboac/network/async_server.h>

namespace oac { namespace network {

inline
async_tcp_server::async_tcp_server(
      network::tcp_port port,
      const connection_handler& handler,
      const std::shared_ptr<boost::asio::io_service>& io_srv =
            std::make_shared<boost::asio::io_service>(),
      const network::error_handler& ehandler = network::error_handler())
throw (network::bind_error)
   : _acceptor(*io_srv),
     _handler(handler),
     _io_service(io_srv),
     _ehandler(ehandler)
{
   try
   {
      bind_port(_acceptor, port);
   }
   catch (io::boost_asio_error& e)
   {
      OAC_THROW_EXCEPTION(network::bind_error(port, e));
   }
   start_accept();
}

inline const boost::asio::io_service&
async_tcp_server::io_service() const
{ return *_io_service; }

inline boost::asio::io_service&
async_tcp_server::io_service()
{ return *_io_service; }

inline void
async_tcp_server::start_accept()
{
   auto conn = std::make_shared<async_tcp_connection>(*_io_service, _ehandler);
   _acceptor.async_accept(
            conn->socket(),
            std::bind(&async_tcp_server::on_accept, this, conn));
}

inline void
async_tcp_server::on_accept(
      const async_tcp_connection_ptr& conn)
{
   _handler(conn);
   start_accept();
}

}} // namespace oac::network

#endif
