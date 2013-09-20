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

#ifndef OAC_NETWORK_CONNECTION_INL
#define OAC_NETWORK_CONNECTION_INL

#include <liboac/network/connection.h>

namespace oac { namespace network {

inline
tcp_connection::tcp_connection()
   : _socket(new boost::asio::ip::tcp::socket(_io_service))
{}

inline boost::asio::io_service&
tcp_connection::io_service()
{ return _io_service; }

inline boost::asio::ip::tcp::socket&
tcp_connection::socket()
{ return *_socket; }

inline tcp_connection::input_stream_ptr
tcp_connection::input()
{ return std::make_shared<input_stream>(_socket); }

inline tcp_connection::output_stream_ptr
tcp_connection::output()
{ return std::make_shared<output_stream>(_socket); }

}} // namespace oac::network

#endif
