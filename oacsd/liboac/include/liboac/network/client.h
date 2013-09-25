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

#ifndef OAC_NETWORK_CLIENT_H
#define OAC_NETWORK_CLIENT_H

#include <liboac/network/connection.h>

namespace oac { namespace network {

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

}} // namespace oac::network

#include <liboac/network/client.inl>

#endif
