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

#ifndef OAC_BUFFER_FIXED_INL
#define OAC_BUFFER_FIXED_INL

#include <liboac/buffer/fixed.h>

namespace oac { namespace buffer {

inline
fixed_buffer::factory::factory(size_t capacity) : _capacity(capacity) {}

inline fixed_buffer*
fixed_buffer::factory::create_buffer() const
{ return new fixed_buffer(_capacity); }

inline
fixed_buffer::fixed_buffer(size_t capacity)
   : _capacity(capacity),
     _read_index(0),
     _write_index(0)
{
   _data = new std::uint8_t[_capacity];
   std::memset(_data, 0, _capacity);
}

inline
fixed_buffer::fixed_buffer(const fixed_buffer& buff)
   : _data(new std::uint8_t[buff._capacity]),
     _capacity(buff._capacity),
     _read_index(buff._read_index),
     _write_index(buff._write_index)
{
   copy(buff, 0, 0, _capacity);
}

inline
fixed_buffer::fixed_buffer(fixed_buffer&& buff)
   : _data(buff._data),
     _capacity(buff._capacity),
     _read_index(buff._read_index),
     _write_index(buff._write_index)
{
   buff._data = 0;
   buff._capacity = 0;
}

inline
fixed_buffer::~fixed_buffer()
{
   if (_data)
      delete _data;
}

inline std::size_t
fixed_buffer::capacity() const
{ return _capacity; }

inline void
fixed_buffer::read(void* dst, std::uint32_t offset, std::size_t length) const
throw (buffer::index_out_of_bounds)
{
   this->check_bounds(offset, length);
   std::memcpy(dst, &(_data[offset]), length);
}

template <typename OutputStream>
void
fixed_buffer::read_to(
      OutputStream& dst,
      std::uint32_t offset,
      std::uint32_t length) const
throw (buffer::index_out_of_bounds, io_exception)
{
   check_bounds(offset, length);
   dst.write(&(_data[offset]), length);
}

inline void
fixed_buffer::write(const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::index_out_of_bounds)
{
   this->check_bounds(offset, length);
   std::memcpy(&(_data[offset]), src, length);
}

template <typename InputStream>
std::size_t
fixed_buffer::write_from(
      InputStream& src,
      std::uint32_t offset,
      std::uint32_t length)
throw (buffer::index_out_of_bounds, io_exception)
{
   check_bounds(offset, length);
   return src.read(&(_data[offset]), length);
}

template <typename Buffer>
void
fixed_buffer::copy(const Buffer& src, std::uint32_t src_offset,
                 std::uint32_t dst_offset, std::size_t length)
throw (buffer::index_out_of_bounds)
{
   this->check_bounds(dst_offset, length);
   src.read(&(_data[dst_offset]), src_offset, length);
}

inline const void*
fixed_buffer::data() const
{ return _data; }

inline void*
fixed_buffer::data()
{ return _data; }

inline void
fixed_buffer::check_bounds(std::uint32_t offset, std::size_t length) const
throw (buffer::index_out_of_bounds)
{
   if (offset + length > _capacity)
      OAC_THROW_EXCEPTION(buffer::index_out_of_bounds(
            offset + length, 0, _capacity - 1));
}

}} // namespace oac::buffer

#endif
