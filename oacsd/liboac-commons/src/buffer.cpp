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

FixedBuffer::FixedBuffer(size_t capacity) : _capacity(capacity)
{
   _data = new BYTE[_capacity];
   std::memset(_data, 0, _capacity);
}

FixedBuffer::~FixedBuffer()
{ delete _data; }

void
FixedBuffer::read(void* dst, DWORD offset, DWORD length) const 
throw (OutOfBoundsError)
{
   this->checkBounds(offset, length);
   std::memcpy(dst, &(_data[offset]), length);
}

void
FixedBuffer::read(OutputStream& dst, DWORD offset, DWORD length) const
throw (OutOfBoundsError, ReadError, OutputStream::WriteError)
{
   this->checkBounds(offset, length);
   dst.write(&(_data[offset]), length);
}

void
FixedBuffer::write(const void* src, DWORD offset, DWORD length) 
throw (OutOfBoundsError)
{
   this->checkBounds(offset, length);
   std::memcpy(&(_data[offset]), src, length);
}

void
FixedBuffer::write(InputStream& src, DWORD offset, DWORD length)
throw (OutOfBoundsError, InputStream::ReadError, WriteError)
{
   this->checkBounds(offset, length);
   src.read(&(_data[offset]), length);
}

void
FixedBuffer::copy(const Buffer& src, DWORD src_offset, 
      DWORD dst_offset, DWORD length) 
throw (OutOfBoundsError)
{
   this->checkBounds(dst_offset, length);
   src.read(&(_data[dst_offset]), src_offset, length);
}

void
FixedBuffer::checkBounds(DWORD offset, DWORD length) const
throw (OutOfBoundsError)
{
   if (offset + length > _capacity)
      BOOST_THROW_EXCEPTION(OutOfBoundsError() <<
            LowerBoundInfo(0) <<
            UpperBoundInfo(_capacity - 1) <<
            IndexInfo(offset + length));
}

ShiftedBuffer::ShiftedBuffer(
      const Ptr<Buffer>& backed_buffer,
      DWORD offset_shift) :
   _backed_buffer(backed_buffer),
   _backed_capacity(backed_buffer->capacity()),
   _offset_shift(offset_shift)
{}

void
ShiftedBuffer::read(void* dst, DWORD offset, DWORD length) const 
throw (OutOfBoundsError, ReadError)
{
   this->checkBounds(offset, length);
   _backed_buffer->read(dst, shift(offset), length);
}

void
ShiftedBuffer::read(OutputStream& dst, DWORD offset, DWORD length) const
throw (OutOfBoundsError, ReadError, OutputStream::WriteError)
{
   this->checkBounds(offset, length);
   _backed_buffer->read(dst, shift(offset), length);
}

void
ShiftedBuffer::write(const void* src, DWORD offset, DWORD length) 
throw (OutOfBoundsError, WriteError)
{ 
   this->checkBounds(offset, length);
   _backed_buffer->write(src, shift(offset), length);
}

void
ShiftedBuffer::write(InputStream& src, DWORD offset, DWORD length)
throw (OutOfBoundsError, InputStream::ReadError, WriteError)
{
   this->checkBounds(offset, length);
   _backed_buffer->write(src, shift(offset), length);
}

void
ShiftedBuffer::copy(const Buffer& src, DWORD src_offset, 
      DWORD dst_offset, DWORD length) 
throw (OutOfBoundsError, IOError)
{ 
   this->checkBounds(dst_offset, length);
   _backed_buffer->copy(src, src_offset, shift(dst_offset), length);
}

void
ShiftedBuffer::checkBounds(DWORD offset, DWORD length) const
throw (OutOfBoundsError)
{
   if (offset < _offset_shift || shift(offset) + length > _backed_capacity)
      THROW_ERROR(OutOfBoundsError() <<
            LowerBoundInfo(0) <<
            UpperBoundInfo(_backed_capacity - 1) <<
            IndexInfo(offset + length));
}

DoubleBuffer::DoubleBuffer(
      const Ptr<Buffer>& backed_buffer_0,
      const Ptr<Buffer>& backed_buffer_1) :
   _current_buffer(0)
{
   _backed_buffer[0] = Ptr<Buffer>(backed_buffer_0);
   _backed_buffer[1] = Ptr<Buffer>(backed_buffer_1);     
}

void
DoubleBuffer::read(void* dst, DWORD offset, DWORD length) const 
throw (OutOfBoundsError)
{ _backed_buffer[_current_buffer]->read(dst, offset, length); }

void
DoubleBuffer::read(OutputStream& dst, DWORD offset, DWORD length) const
throw (OutOfBoundsError, ReadError, OutputStream::WriteError)
{ _backed_buffer[_current_buffer]->read(dst, offset, length); }

void
DoubleBuffer::write(const void* src, DWORD offset, DWORD length) 
throw (OutOfBoundsError)
{ _backed_buffer[_current_buffer]->write(src, offset, length); }

void
DoubleBuffer::write(InputStream& src, DWORD offset, DWORD length)
throw (OutOfBoundsError)
{ _backed_buffer[_current_buffer]->write(src, offset, length); }

void
DoubleBuffer::copy(const Buffer& src, DWORD src_offset, 
      DWORD dst_offset, DWORD length)
throw (OutOfBoundsError)
{ _backed_buffer[_current_buffer]->copy(src, src_offset, dst_offset, length); }

void
DoubleBuffer::swap()
{ _current_buffer ^= 1; }

DWORD
BufferInputStream::read(void* buffer, DWORD count)
{
   auto remain = _buffer->capacity() - _index;
   if (count > remain)
      count = remain;
   _buffer->read(buffer, _index, count);
   _index += count;
   return count;
}

void
BufferOutputStream::write(const void* buffer, DWORD count)
throw (CapacityExhaustedError)
{
   auto remain = _buffer->capacity() - _index;
   if (count > remain)
      THROW_ERROR(CapacityExhaustedError() <<
                  RemainingBytesInfo(remain) << RequestedBytesInfo(count));
   _buffer->write(buffer, _index, count);
   _index += count;
}

} // namespace oac
