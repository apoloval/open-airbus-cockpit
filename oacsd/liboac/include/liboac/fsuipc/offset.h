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

#ifndef OAC_FSUIPC_OFFSET_H
#define OAC_FSUIPC_OFFSET_H

#include <cstdint>

namespace oac { namespace fsuipc {

/**
 * The address of a FSUIPC offset.
 */
typedef std::uint16_t offset_address;

/**
 * The length of a FSUIPC offset.
 */
enum offset_length
{
   OFFSET_LEN_BYTE   = 1,
   OFFSET_LEN_WORD   = 2,
   OFFSET_LEN_DWORD  = 4
};

/**
 * The value of a FSUIPC offset. It actually have storage enought to store
 * a value of a OFFSET_LEN_DWORD offset. For any other offset length, only
 * the corresponding bytes should be interpreted.
 */
typedef std::uint32_t offset_value;

/**
 * A FSUIPC offset, comprising its address and length.
 */
struct offset
{
   /**
    * A hash type for offset. This allow offset to be used
    * in hash-based STL collections.
    */
   struct hash
   {
      std::size_t operator()(const offset& offset) const
      { return offset.address * 4 + offset.length; }
   };

   offset_address address;
   offset_length length;

   offset(
         offset_address addr,
         offset_length len)
      : address(addr), length(len)
   {}

   bool operator == (const offset& offset) const
   { return address == offset.address && length == offset.length; }
};

/**
 * A FSUIPC valued object, comprising the offset itself with the corresponding
 * value.
 */
struct valued_offset : offset
{
   offset_value value;

   valued_offset(
         const offset& offset,
         offset_value val)
      : offset(offset), value(val)
   {}

   valued_offset(
         offset_address addr,
         offset_length len,
         offset_value val)
      : offset(addr, len), value(val)
   {}
};

}} // namespace oac::fsuipc

#endif
