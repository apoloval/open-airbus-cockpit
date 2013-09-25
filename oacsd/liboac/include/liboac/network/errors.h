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

#ifndef OAC_NETWORK_ERRORS_H
#define OAC_NETWORK_ERRORS_H

#include "liboac/exception.h"
#include "liboac/network/types.h"

namespace oac { namespace network {

/**
 * An error while binding a socket.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(bind_error, io_exception,
   ("cannot bind on port %d", port),
   (port, tcp_port));

/**
 * An error while connecting to a remote peer.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(connection_refused, io_exception,
   (
      "connection refused to %s on port %d",
      remote_host,
      remote_port
   ),
   (remote_host, hostname),
   (remote_port, tcp_port));

/**
 * An exception reporting a connection reset by remote peer while
 * executing an IO operation.
 */
OAC_DECL_EXCEPTION(
      connection_reset,
      io_exception,
      "connection reset by peer");


}} // namespace oac::network

#endif
