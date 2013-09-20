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

#ifndef OAC_NETWORK_CONNECTION_H
#define OAC_NETWORK_CONNECTION_H

#include <boost/asio/ip/tcp.hpp>

#include <liboac/stream.h>

namespace oac { namespace network {

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

}} // namespace oac::network

#include <liboac/network/connection.inl>

#endif
