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

#ifndef OAC_BUFFER_FUNCTIONS_H
#define OAC_BUFFER_FUNCTIONS_H

#include <cstdint>

#include "liboac/buffer/errors.h"
#include "liboac/io.h"

namespace oac { namespace buffer {

template <typename T, typename Buffer>
T read_as(Buffer& b, std::uint32_t offset)
throw (index_out_of_bounds, io_exception)
{
   T result;
   b.read(&result, offset, sizeof(T));
   return result;
}

template <typename T, typename Buffer>
void write_as(Buffer& b, std::uint32_t offset, const T& t)
throw (index_out_of_bounds, io_exception)
{
   b.write(&t, offset, sizeof(T));
}

template <typename Buffer>
void fill(Buffer& b, std::uint8_t value)
throw (io_exception)
{
   for (std::uint32_t i = 0; i < b.capacity(); i++)
      b.write(&value, i, 1);
}

template <typename AsyncReadStream,
          typename StreamedBuffer,
          typename AsyncReadHandler>
void
async_read_some(
      AsyncReadStream& stream,
      StreamedBuffer& buffer,
      AsyncReadHandler handler)
{
   buffer.async_write_some_from(stream, handler);
}

template <typename AsyncWriteStream,
          typename StreamBuffer,
          typename AsyncWriteHandler>
void
async_write_some(
      AsyncWriteStream& stream,
      StreamBuffer& buffer,
      AsyncWriteHandler handler)
{
   buffer.async_read_some_to(stream, handler);
}

}} // namespace oac::buffer

#endif
