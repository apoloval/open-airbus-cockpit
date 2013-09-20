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

#ifndef OAC_BUFFER_DOUBLE_H
#define OAC_BUFFER_DOUBLE_H

#include "liboac/buffer/linear.h"

namespace oac { namespace buffer {

/**
 * Double buffer class. This class decorates a couple of buffers,
 * making it possible to detect modifications among them.
 */
template <typename Buffer = linear_buffer>
class double_buffer :
      public linear_stream_buffer_base<double_buffer<Buffer>>
{
public:

   class factory
   {
   public:

      typedef double_buffer value_type;

      inline factory(const typename Buffer::factory_ptr& backed_buffer_fact) :
         _backed_buffer_fact(backed_buffer_fact)
      {}

      inline virtual double_buffer* create_buffer() const
      {
         return new double_buffer(
               Buffer::ptr_type(_backed_buffer_fact->create_buffer()),
               Buffer::ptr_type(_backed_buffer_fact->create_buffer()));
      }

   private:

      typename Buffer::factory_ptr _backed_buffer_fact;
      unsigned long _shift;
   };

   typedef std::shared_ptr<double_buffer> ptr_type;
   typedef factory factory_type;
   typedef std::shared_ptr<factory> factory_ptr;

   double_buffer(
         const typename Buffer::ptr_type& backed_buffer_0,
         const typename Buffer::ptr_type& backed_buffer_1) :
      linear_stream_buffer_base<double_buffer<Buffer>>(this),
      _current_buffer(0)
   {
      _backed_buffer[0] = backed_buffer_0;
      _backed_buffer[1] = backed_buffer_1;
   }

   inline std::size_t capacity() const
   { return _backed_buffer[_current_buffer]->capacity(); }

   inline void read(void* dst, std::uint32_t offset, std::size_t length) const
   throw (buffer::index_out_of_bounds)
   { _backed_buffer[_current_buffer]->read(dst, offset, length); }

   template <typename OutputStream>
   inline void read(OutputStream& dst,
                    std::uint32_t offset,
                    std::size_t length) const
   throw (buffer::index_out_of_bounds, io_exception)
   { _backed_buffer[_current_buffer]->read(dst, offset, length); }

   inline void write(const void* src, std::uint32_t offset, std::size_t length)
   throw (buffer::index_out_of_bounds)
   { _backed_buffer[_current_buffer]->write(src, offset, length); }

   template <typename InputStream>
   inline std::size_t write(InputStream& src,
                            std::uint32_t offset,
                            std::size_t length)
   throw (buffer::index_out_of_bounds)
   { return _backed_buffer[_current_buffer]->write(src, offset, length); }

   template <typename Buffer>
   void copy(const Buffer& src,
             std::uint32_t src_offset,
             std::uint32_t dst_offset,
             std::size_t length)
   throw (buffer::index_out_of_bounds)
   {
      _backed_buffer[_current_buffer]->copy(
               src, src_offset, dst_offset, length);
   }

   inline void swap()
   { _current_buffer ^= 1; }

   template <std::size_t length>
   inline bool is_modified(std::uint32_t offset)
   {
      std::uint8_t data0[length];
      std::uint8_t data1[length];
      _backed_buffer[0]->read(&data0, offset, length);
      _backed_buffer[1]->read(&data1, offset, length);
      return memcmp(data0, data1, length) != 0;
   }

   template <typename T>
   inline bool is_modified_as(std::uint32_t offset)
   { return is_modified<sizeof(T)>(offset); }

private:

   typename Buffer::ptr_type _backed_buffer[2];
   std::uint8_t _current_buffer;
};

template <typename Buffer>
typename double_buffer<Buffer>::ptr_type dup_buffers(
      const typename Buffer::ptr_type& b1,
      const typename Buffer::ptr_type& b2)
{ return std::make_shared<double_buffer<Buffer>>(b1, b2); }

template <typename BufferFactory>
typename double_buffer<typename BufferFactory::value_type>::factory_ptr
dup_factory(
      const std::shared_ptr<BufferFactory>& fact)
{
   return std::make_shared<
         double_buffer<typename BufferFactory::value_type>::factory>(fact);
}

}} // namespace oac::buffer

#endif
