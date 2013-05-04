/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
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

#include "buffer.h"
#include "logging.h"

namespace oac {

fixed_buffer::fixed_buffer(size_t capacity) : _capacity(capacity)
{
   _data = new BYTE[_capacity];
   std::memset(_data, 0, _capacity);
}

fixed_buffer::~fixed_buffer()
{ delete _data; }

void
fixed_buffer::read(void* dst, std::uint32_t offset, std::size_t length) const
throw (buffer::out_of_bounds_error)
{
   this->check_bounds(offset, length);
   std::memcpy(dst, &(_data[offset]), length);
}

void
fixed_buffer::write(const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::out_of_bounds_error)
{
   this->check_bounds(offset, length);
   std::memcpy(&(_data[offset]), src, length);
}

void
fixed_buffer::check_bounds(std::uint32_t offset, std::size_t length) const
throw (buffer::out_of_bounds_error)
{
   if (offset + length > _capacity)
      BOOST_THROW_EXCEPTION(buffer::out_of_bounds_error() <<
            lower_bound_info(0) <<
            upper_bound_info(_capacity - 1) <<
            index_info(offset + length));
}

} // namespace oac
