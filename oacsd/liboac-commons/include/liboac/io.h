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

#ifndef OAC_IO_H
#define OAC_IO_H

#include <boost/system/error_code.hpp>

#include "exception.h"

namespace oac {

/**
 * An exception reporting an error in a input/output operation.
 */
OAC_ABSTRACT_EXCEPTION(io_exception);

/**
 * An exception reporting an unexpected end of file while
 * executing an IO operation.
 */
OAC_EXCEPTION(eof_error, io_exception, "unexpected end of file");

/**
 * An exception reporting an error in a Boost ASIO operation.
 */
OAC_EXCEPTION_BEGIN(boost_asio_error, io_exception)
   OAC_EXCEPTION_FIELD(error_code, boost::system::error_code)
   OAC_EXCEPTION_MSG(
      "Boost ASIO operation returned with an error code %d (%s)",
      error_code.value(),
      error_code.message())
OAC_EXCEPTION_END()

} // namespace oac

#endif
