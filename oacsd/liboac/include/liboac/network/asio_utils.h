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

#ifndef OAC_NETWORK_ASIO_UTILS_H
#define OAC_NETWORK_ASIO_UTILS_H

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>

#include <liboac/network/types.h>

namespace oac { namespace network {

inline void
bind_port(
      boost::asio::ip::tcp::acceptor& acceptor,
      network::tcp_port port)
throw (io::boost_asio_error)
{
   using namespace boost::asio;
   try
   {
      ip::tcp::endpoint ep(ip::tcp::v4(), port);
      acceptor.open(ep.protocol());
      acceptor.bind(ep);
      acceptor.listen();
   } catch (const boost::system::system_error& e)
   {
      OAC_THROW_EXCEPTION(io::boost_asio_error(e.code(), e));
   }
}

inline void
connect(
      const network::hostname& remote_host,
      network::tcp_port remote_port,
      boost::asio::io_service& io_srv,
      boost::asio::ip::tcp::socket& socket)
throw (io::boost_asio_error)
{
   using namespace boost::asio;
   try
   {
      ip::tcp::resolver resolver(io_srv);
      ip::tcp::resolver::query query(
            remote_host,
            boost::lexical_cast<std::string>(remote_port));
      boost::asio::connect(socket, resolver.resolve(query));
   }
   catch (const boost::system::system_error& e)
   {
      OAC_THROW_EXCEPTION(io::boost_asio_error(e.code(), e));
   }
}

}} // namespace oac::network

#endif
