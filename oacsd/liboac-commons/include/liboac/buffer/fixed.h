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

#ifndef OAC_BUFFER_FIXED_H
#define OAC_BUFFER_FIXED_H

#include <cstdint>

#include "liboac/buffer/errors.h"
#include "liboac/io.h"

namespace oac { namespace buffer {

/**
 * Buffer of fixed capacity. This is buffer implementation of
 * fixed capacity set at object construction. It conforms Buffer concept but
 * not Stream. For streamed buffer please check linear_buffer or ring_buffer
 * classes.
 */
class fixed_buffer
{
public:

   /**
    * A factory class conforming to BufferFactory.
    */
   class factory
   {
   public:

      typedef fixed_buffer value_type;

      factory(size_t capacity);

      virtual fixed_buffer* create_buffer() const;

   private:

      size_t _capacity;
   };

   typedef std::shared_ptr<fixed_buffer> ptr_type;
   typedef factory factory_type;
   typedef std::shared_ptr<factory_type> factory_ptr;

   fixed_buffer(size_t capacity);

   fixed_buffer(const fixed_buffer& buff);

   fixed_buffer(fixed_buffer&& buff);

   ~fixed_buffer();

   std::size_t capacity() const;

   void read(void* dst, std::uint32_t offset, std::uint32_t length) const
         throw (buffer::index_out_of_bounds);

   template <typename OutputStream>
   void read_to(
         OutputStream& dst,
         std::uint32_t offset,
         std::uint32_t length) const
   throw (buffer::index_out_of_bounds, io_exception);

   void write(const void* src, std::uint32_t offset, std::uint32_t length)
         throw (buffer::index_out_of_bounds, io_exception);

   template <typename InputStream>
   std::size_t write_from(
         InputStream& src,
         std::uint32_t offset,
         std::uint32_t length)
   throw (buffer::index_out_of_bounds, io_exception);

   template <typename Buffer>
   void copy(
         const Buffer& src, std::uint32_t src_offset,
         std::uint32_t dst_offset, std::size_t length)
   throw (buffer::index_out_of_bounds);

   /**
    * Obtain a pointer to the raw data for this buffer.
    */
   const void* data() const;

   /**
    * Obtain a pointer to the raw data for this buffer.
    */
   void* data();

private:

   std::uint8_t* _data;
   size_t _capacity;
   std::uint32_t _read_index;
   std::uint32_t _write_index;

   void check_bounds(std::uint32_t offset, std::size_t length) const
         throw (buffer::index_out_of_bounds);
};

typedef std::shared_ptr<fixed_buffer> fixed_buffer_ptr;

}} // namespace oac::buffer

#include <liboac/buffer/fixed.inl>

#endif
