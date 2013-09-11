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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_FV_CLIENT_ERRORS_H
#define OAC_FV_CLIENT_ERRORS_H

#include <liboac/exception.h>

namespace oac { namespace fv { namespace client {

/**
 * Exception thrown when a communication error occurs.
 */
OAC_EXCEPTION(
      communication_error,
      oac::exception,
      "something went wrong while communicating with FlightVars server");

/**
 * Exception thrown when on request timeout.
 */
OAC_EXCEPTION(
   request_timeout_error,
   oac::exception,
   "Time out while waiting for the result of the request");

}}} // namespace oac::fv::client

#endif
