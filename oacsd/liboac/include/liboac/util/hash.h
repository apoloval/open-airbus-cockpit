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

#ifndef OAC_UTIL_HASH_H
#define OAC_UTIL_HASH_H

#include <boost/optional.hpp>

namespace std {

/** A hash function for boost::optional objects. */
template <typename T>
struct hash<boost::optional<T>>
{
   typedef boost::optional<T> argument_type;
   typedef std::size_t value_type;

   value_type operator()(const argument_type& s) const
   {
      return s.is_initialized() ? std::hash<T>()(s.get()) : 0;
   }
};

} // namespace std

#endif
