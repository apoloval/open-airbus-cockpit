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
std::string format(
      const char* fmt)
{ return fmt; }

template <typename T1>
std::string format(
      const char* fmt,
      const T1& v1)
{
   return str(boost::format(fmt) % v1);
}

template <typename T1, typename T2>
std::string format(
      const char* fmt,
      const T1& v1,
      const T2& v2)
{
   return str(boost::format(fmt) % v1 % v2);
}

template <typename T1, typename T2, typename T3>
std::string format(
      const char* fmt,
      const T1& v1,
      const T2& v2,
      const T3& v3)
{
   return str(boost::format(fmt) % v1 % v2 % v3);
}

template <typename T1, typename T2, typename T3, typename T4>
std::string format(
      const char* fmt,
      const T1& v1,
      const T2& v2,
      const T3& v3,
      const T4& v4)
{
   return str(boost::format(fmt) % v1 % v2 % v3 % v4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::string format(
      const char* fmt,
      const T1& v1,
      const T2& v2,
      const T3& v3,
      const T4& v4,
      const T5& v5)
{
   return str(boost::format(fmt) % v1 % v2 % v3 % v4 %v5);
}

} // namespace oac

#endif
