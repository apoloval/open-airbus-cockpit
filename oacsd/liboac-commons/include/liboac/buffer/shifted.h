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

#ifndef OAC_BUFFER_SHIFTED_H
#define OAC_BUFFER_SHIFTED_H

#include "liboac/buffer/linear.h"

namespace oac { namespace buffer {

/**
 * Shifted buffer class. This class provides a decorator that shifts
 * the offset on read and write operations. The decorated buffer and
 * the shift are set in the constructor. After that, offsets in read
 * or write are shifted. E.g., for shift 0x5600, a read/write on offset
 * 0x5678 would be made on decorated buffer on 0x0078.
 */
template <typename Buffer>
class shifted_buffer :
      public linear_stream_buffer_base<shifted_buffer<Buffer>>
{
public:

   class factory
   {
   public:

      typedef shifted_buffer value_type;

      factory(
            const typename Buffer::factory_ptr& backed_buffer_fact,
            std::uint32_t shift)
         : _backed_buffer_fact(backed_buffer_fact), _shift(shift)
      {}

      inline virtual shifted_buffer* create_buffer() const
      {
         return new shifted_buffer(
               Buffer::ptr_type(_backed_buffer_fact->create_buffer()),
               _shift);
      }

   private:

      typename Buffer::factory_ptr _backed_buffer_fact;
      std::uint32_t _shift;
   };

   typedef std::shared_ptr<shifted_buffer> ptr_type;
   typedef factory factory_type;
   typedef std::shared_ptr<factory> factory_ptr;

   inline shifted_buffer(const typename Buffer::ptr_type& backed_buffer,
                         std::uint32_t offset_shift)
      : linear_stream_buffer_base<shifted_buffer<Buffer>>(this),
        _backed_buffer(backed_buffer),
        _backed_capacity(backed_buffer->capacity()),
        _offset_shift(offset_shift)
   {}

   std::size_t read(void* dest, std::size_t count)
   { return _backed_buffer->read(dest, count); }

   std::size_t write(const void* src, std::size_t count)
   { return _backed_buffer->write(src, count); }

   void flush()
   { _backed_buffer->flush(); }

   inline std::size_t capacity() const
   { return _backed_buffer->capacity(); }

   inline void read(void* dst, std::uint32_t offset, std::size_t length) const
   throw (buffer::index_out_of_bounds, io_exception)
   {
      check_bounds(offset, length);
      _backed_buffer->read(dst, shift(offset), length);
   }

   template <typename OutputStream>
   inline void read(
         OutputStream& dst, std::uint32_t offset, std::size_t length) const
   throw (buffer::index_out_of_bounds, io_exception)
   {
      check_bounds(offset, length);
      _backed_buffer->read(dst, shift(offset), length);
   }

   inline void write(const void* src, std::uint32_t offset, std::size_t length)
   throw (buffer::index_out_of_bounds, io_exception)
   {
      check_bounds(offset, length);
      _backed_buffer->write(src, shift(offset), length);
   }

   template <typename InputStream>
   inline std::size_t write(
         InputStream& src, std::uint32_t offset, std::size_t length)
   throw (buffer::index_out_of_bounds, io_exception)
   {
      check_bounds(offset, length);
      return _backed_buffer->write(src, shift(offset), length);
   }

   template <typename Buffer>
   inline void copy(const Buffer& src, std::uint32_t src_offset,
         std::uint32_t dst_offset, std::size_t length)
   throw (buffer::index_out_of_bounds, io_exception)
   {
      check_bounds(dst_offset, length);
      _backed_buffer->copy(src, src_offset, shift(dst_offset), length);
   }

private:

   typename Buffer::ptr_type _backed_buffer;
   std::size_t _backed_capacity;
   std::uint32_t _offset_shift;

   inline void check_bounds(std::uint32_t offset, std::uint32_t length) const
   throw (buffer::index_out_of_bounds)
   {
      if (offset < _offset_shift || shift(offset) + length > _backed_capacity)
         OAC_THROW_EXCEPTION(buffer::index_out_of_bounds(
               offset + length,
               0,
               _backed_capacity - 1));
   }

   inline std::uint32_t shift(std::uint32_t offset) const
   { return offset - _offset_shift; }
};

template <typename Buffer>
typename shifted_buffer<Buffer>::ptr_type shift_buffer(
      const typename Buffer::ptr_type& b,
      std::uint32_t shift)
{ return std::make_shared<shifted_buffer<Buffer>>(b, shift); }

template <typename BufferFactory>
typename shifted_buffer<typename BufferFactory::value_type>::factory_ptr
shift_factory(
      const std::shared_ptr<BufferFactory>& fact,
      std::uint32_t shift)
{
   return std::make_shared<
         shifted_buffer<typename BufferFactory::value_type>::factory>(
               fact, shift);
}

}} // namespace oac::buffer

#endif
