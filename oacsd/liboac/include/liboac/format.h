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

#ifndef OAC_FORMAT_H
#define OAC_FORMAT_H

#include <boost/format.hpp>

// This file provides convenience functions to
// format strings based on Boost Format library

namespace oac {

inline
std::string format(boost::format& fmt)
{
   return str(fmt);
}

template <typename T, typename... Args>
std::string format(
      boost::format& fmt, const T& t, const Args&... args)
{
   return format(fmt % t, args...);
}

template <typename... Args>
std::string format(
      const char* fmt, const Args&... args)
{
   return format(boost::format(fmt), args...);
}

} // namespace oac

#endif
