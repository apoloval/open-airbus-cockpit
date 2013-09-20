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

#ifndef OAC_BUFFER_LINEAR_H
#define OAC_BUFFER_LINEAR_H

#include <list>

#include <boost/asio/buffer.hpp>

#include "liboac/buffer/errors.h"
#include "liboac/buffer/fixed.h"

namespace oac { namespace buffer {

template <typename Buffer>
class linear_stream_buffer_base
{
public:

   std::size_t read(void* dest, std::size_t count);

   std::size_t write(const void* src, std::size_t count);

   void flush();

   std::size_t available_for_read() const;

   std::size_t available_for_write() const;

   boost::optional<std::uint32_t> mark() const;

   void set_mark();

   void unset_mark();

   void reset();

   template <typename AsyncReadStream,
             typename ReadHandler>
   void async_write_some_from(AsyncReadStream& stream, ReadHandler handler);

   template <typename AsyncWriteStream,
             typename WriteHandler>
   void async_read_some_to(AsyncWriteStream& stream, WriteHandler handler);

protected:

   linear_stream_buffer_base(Buffer* self);

   linear_stream_buffer_base(
         const linear_stream_buffer_base& base, Buffer* self);

   linear_stream_buffer_base(
         linear_stream_buffer_base&& base, Buffer* self);

   std::list<boost::asio::mutable_buffer> asio_mutable_buffers();

   std::list<boost::asio::mutable_buffer> asio_mutable_buffers(
         std::size_t nbytes);

   std::list<boost::asio::const_buffer> asio_const_buffers();

   std::list<boost::asio::const_buffer> asio_const_buffers(
         std::size_t nbytes);

private:

   Buffer* _self;
   std::uint32_t _read_index;
   std::uint32_t _write_index;
   boost::optional<std::uint32_t> _mark;
};



class linear_buffer :
      public linear_stream_buffer_base<fixed_buffer>
{
public:

   using linear_stream_buffer_base<fixed_buffer>::read;
   using linear_stream_buffer_base<fixed_buffer>::write;

   class factory
   {
   public:

      typedef linear_buffer value_type;

      factory(size_t capacity);

      virtual value_type* create_buffer() const;

   private:

      std::size_t _capacity;
   };

   typedef std::shared_ptr<linear_buffer> ptr_type;
   typedef factory factory_type;
   typedef std::shared_ptr<factory_type> factory_ptr;

   /**
    * Create a new linear buffer with given capacity.
    */
   linear_buffer(std::size_t capacity);

   linear_buffer(const linear_buffer& buff);

   linear_buffer(linear_buffer&& buff);

   std::size_t capacity() const;

   void read(void* dst, std::uint32_t offset, std::size_t length) const
         throw (buffer::index_out_of_bounds);

   template <typename OutputStream>
   void read_to(
         OutputStream& dst, std::uint32_t offset, std::size_t length) const
         throw (buffer::index_out_of_bounds, io_exception);

   void write(
         const void* src, std::uint32_t offset, std::size_t length)
         throw (buffer::index_out_of_bounds);

   template <typename InputStream>
   std::size_t write_from(
         InputStream& src, std::uint32_t offset, std::size_t length)
         throw (buffer::index_out_of_bounds, io_exception);

   template <typename Buffer>
   void copy(
         const Buffer& src, std::uint32_t src_offset,
         std::uint32_t dst_offset, std::size_t length)
         throw (buffer::index_out_of_bounds);

private:

   fixed_buffer _fixed_buffer;
};

typedef std::shared_ptr<linear_buffer> linear_buffer_ptr;

}} // namespace oac::buffer

#include "buffer/linear.inl"

#endif
