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

#ifndef OAC_BUFFER_ERRORS_H
#define OAC_BUFFER_ERRORS_H

#include "liboac/exception.h"

namespace oac { namespace buffer {

/**
 * An exception indicating a invalid index provided in a buffer operation.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(index_out_of_bounds, oac::exception,
   (
         "buffer access with invalid index %d for bounds [%d, %d]",
         index,
         lower_bound,
         upper_bound
   ),
   (index, int),
   (lower_bound, int),
   (upper_bound, int));

}} // namespace oac::buffer

#endif
