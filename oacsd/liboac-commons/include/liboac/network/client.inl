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

#ifndef OAC_NETWORK_CLIENT_INL
#define OAC_NETWORK_CLIENT_INL

#include <liboac/network/asio_utils.h>
#include <liboac/network/client.h>

namespace oac { namespace network {

inline
tcp_client::tcp_client(
      const network::hostname& hostname,
      network::tcp_port port)
throw (network::connection_refused)
{
   try
   {
      _connection = std::make_shared<tcp_connection>();
      connect(
            hostname,
            port,
            _connection->io_service(),
            _connection->socket());
   }
   catch (io::boost_asio_error& e)
   {
      OAC_THROW_EXCEPTION(network::connection_refused(hostname, port, e));
   }
}

inline tcp_connection&
tcp_client::connection()
{ return *_connection; }

inline tcp_connection::input_stream_ptr
tcp_client::input()
{ return _connection->input(); }

inline tcp_connection::output_stream_ptr
tcp_client::output()
{ return _connection->output(); }

}} // namespace oac::network

#endif
