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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_ENDIAN_H
#define OAC_ENDIAN_H

#include <cstdint>

#include <boost/detail/endian.hpp>

namespace oac {

template <typename T>
T endian_swap(T v);

template <>
inline std::uint16_t endian_swap<std::uint16_t>(std::uint16_t v)
{
   return (v >> 8) | (v << 8);
}

template <>
inline std::uint32_t endian_swap<std::uint32_t>(std::uint32_t v)
{
   return (v >> 24) |
          ((v << 8) & 0x00ff0000) |
          ((v >> 8) & 0x0000ff00) |
          (v << 24);
}

template <>
inline std::int16_t endian_swap<std::int16_t>(std::int16_t v)
{
   return (v >> 8) | (v << 8);
}

template <>
inline std::int32_t endian_swap<std::int32_t>(std::int32_t v)
{
   return (v >> 24) |
          ((v << 8) & 0x00ff0000) |
          ((v >> 8) & 0x0000ff00) |
          (v << 24);
}

template <typename T>
inline T native_to_big(T v)
{
#if defined(BOOST_BIG_ENDIAN)
   return v;
#else
   return endian_swap(v);
#endif
}

template <typename T>
inline T native_to_little(T v)
{
#if defined(BOOST_LITTLE_ENDIAN)
   return v;
#else
   return endian_swap(v);
#endif
}

template <typename T>
inline T little_to_native(T v)
{
#if defined(BOOST_LITTLE_ENDIAN)
   return v;
#else
   return endian_swap(v);
#endif
}

template <typename T>
inline T big_to_native(T v)
{
#if defined(BOOST_BIG_ENDIAN)
   return v;
#else
   return endian_swap(v);
#endif
}

} // namespace oac

#endif
