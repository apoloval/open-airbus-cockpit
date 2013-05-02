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

namespace {

}; // anonymous namespace

fixed_buffer::fixed_buffer(size_t capacity) : _capacity(capacity)
{
   _data = new BYTE[_capacity];
   std::memset(_data, 0, _capacity);
}

fixed_buffer::~fixed_buffer()
{ delete _data; }

void
fixed_buffer::read(void* dst, DWORD offset, DWORD length) const 
throw (out_of_bounds_error)
{
   this->check_bounds(offset, length);
   std::memcpy(dst, &(_data[offset]), length);
}

void
fixed_buffer::read(output_stream& dst, DWORD offset, DWORD length) const
throw (out_of_bounds_error, read_error, output_stream::write_error)
{
   this->check_bounds(offset, length);
   dst.write(&(_data[offset]), length);
}

void
fixed_buffer::write(const void* src, DWORD offset, DWORD length) 
throw (out_of_bounds_error)
{
   this->check_bounds(offset, length);
   std::memcpy(&(_data[offset]), src, length);
}

DWORD
fixed_buffer::write(input_stream& src, DWORD offset, DWORD length)
throw (out_of_bounds_error, input_stream::read_error, write_error)
{
   this->check_bounds(offset, length);
   return src.read(&(_data[offset]), length);
}

void
fixed_buffer::copy(const buffer& src, DWORD src_offset, 
      DWORD dst_offset, DWORD length) 
throw (out_of_bounds_error)
{
   this->check_bounds(dst_offset, length);
   src.read(&(_data[dst_offset]), src_offset, length);
}

void
fixed_buffer::check_bounds(DWORD offset, DWORD length) const
throw (out_of_bounds_error)
{
   if (offset + length > _capacity)
      BOOST_THROW_EXCEPTION(out_of_bounds_error() <<
            lower_bound_info(0) <<
            upper_bound_info(_capacity - 1) <<
            index_info(offset + length));
}

shifted_buffer::shifted_buffer(
      const ptr<buffer>& backed_buffer,
      DWORD offset_shift) :
   _backed_buffer(backed_buffer),
   _backed_capacity(backed_buffer->capacity()),
   _offset_shift(offset_shift)
{}

void
shifted_buffer::read(void* dst, DWORD offset, DWORD length) const 
throw (out_of_bounds_error, read_error)
{
   this->check_bounds(offset, length);
   _backed_buffer->read(dst, shift(offset), length);
}

void
shifted_buffer::read(output_stream& dst, DWORD offset, DWORD length) const
throw (out_of_bounds_error, read_error, output_stream::write_error)
{
   this->check_bounds(offset, length);
   _backed_buffer->read(dst, shift(offset), length);
}

void
shifted_buffer::write(const void* src, DWORD offset, DWORD length) 
throw (out_of_bounds_error, write_error)
{ 
   this->check_bounds(offset, length);
   _backed_buffer->write(src, shift(offset), length);
}

DWORD
shifted_buffer::write(input_stream& src, DWORD offset, DWORD length)
throw (out_of_bounds_error, input_stream::read_error, write_error)
{
   this->check_bounds(offset, length);
   return _backed_buffer->write(src, shift(offset), length);
}

void
shifted_buffer::copy(const buffer& src, DWORD src_offset, 
      DWORD dst_offset, DWORD length) 
throw (out_of_bounds_error, io_error)
{ 
   this->check_bounds(dst_offset, length);
   _backed_buffer->copy(src, src_offset, shift(dst_offset), length);
}

void
shifted_buffer::check_bounds(DWORD offset, DWORD length) const
throw (out_of_bounds_error)
{
   if (offset < _offset_shift || shift(offset) + length > _backed_capacity)
      BOOST_THROW_EXCEPTION(out_of_bounds_error() <<
            lower_bound_info(0) <<
            upper_bound_info(_backed_capacity - 1) <<
            index_info(offset + length));
}

double_buffer::double_buffer(
      const ptr<buffer>& backed_buffer_0,
      const ptr<buffer>& backed_buffer_1) :
   _current_buffer(0)
{
   _backed_buffer[0] = ptr<buffer>(backed_buffer_0);
   _backed_buffer[1] = ptr<buffer>(backed_buffer_1);
}

void
double_buffer::read(void* dst, DWORD offset, DWORD length) const 
throw (out_of_bounds_error)
{ _backed_buffer[_current_buffer]->read(dst, offset, length); }

void
double_buffer::read(output_stream& dst, DWORD offset, DWORD length) const
throw (out_of_bounds_error, read_error, output_stream::write_error)
{ _backed_buffer[_current_buffer]->read(dst, offset, length); }

void
double_buffer::write(const void* src, DWORD offset, DWORD length) 
throw (out_of_bounds_error)
{ _backed_buffer[_current_buffer]->write(src, offset, length); }

DWORD
double_buffer::write(input_stream& src, DWORD offset, DWORD length)
throw (out_of_bounds_error)
{ return _backed_buffer[_current_buffer]->write(src, offset, length); }

void
double_buffer::copy(const buffer& src, DWORD src_offset, 
      DWORD dst_offset, DWORD length)
throw (out_of_bounds_error)
{ _backed_buffer[_current_buffer]->copy(src, src_offset, dst_offset, length); }

void
double_buffer::swap()
{ _current_buffer ^= 1; }

DWORD
buffer_input_stream::read(void* buffer, DWORD count)
{
   auto remain = _buffer->capacity() - _index;
   if (count > remain)
      count = remain;
   _buffer->read(buffer, _index, count);
   _index += count;
   return count;
}

void
buffer_output_stream::write(const void* buffer, DWORD count)
throw (capacity_exhausted_error)
{
   auto remain = _buffer->capacity() - _index;
   if (count > remain)
      BOOST_THROW_EXCEPTION(capacity_exhausted_error() <<
                  remaining_bytes_info(remain) << requested_bytes_info(count));
   _buffer->write(buffer, _index, count);
   _index += count;
}

void
buffer_output_stream::flush()
{
   // Well, it's a buffer. Nothing to be done here.
}

} // namespace oac
