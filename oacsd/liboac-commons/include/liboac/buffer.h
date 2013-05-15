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

#ifndef OAC_BUFFER_H
#define OAC_BUFFER_H

#pragma warning( disable : 4290 )

#include <Windows.h>

#include <Boost/format.hpp>

#include "exception.h"
#include "lang-utils.h"
#include "stream.h"

namespace oac {

/**
 * @concept Buffer
 *
 * A class that may be used as data storage, allowing reading from and
 * writing to bytes. It should provide the following members.
 *
 * typedef BufferFactory factory_type;
 *
 * std::size_t Buffer::capacity() const;
 *
 * void Buffer::read(void* dst, std::uint32_t offset, std::size_t length) const
 *       throw (out_of_bounds_error);
 *
 * template <typename OutputStream>
 * void Buffer::read_to(
 *    OutputStream& dst, std::uint32_t offset, std::size_t length) const
 *       throw (out_of_bounds_error, read_error, stream::write_error);
 *
 * void Buffer::write(const void* src, std::uint32_t offset, std::size_t length)
 *       throw (out_of_bounds_error, write_error);
 *
 * template <typename InputStream>
 * std::size_t Buffer::write_from(
 *       InputStream& src, std::uint32_t offset, std::size_t length)
 *       throw (out_of_bounds_error, stream::read_error, write_error);
 *
 * template <typename Buffer>
 * void copy(const Buffer& src, std::uint32_t src_offset,
 *           std::uint32_t dst_offset, std::size_t length)
 *    throw (out_of_bounds_error, read_error, write_error);
 */

/**
 * @concept BufferFactory
 *
 * A class that may be used to create buffer objects. It provides the
 * following members, where T is the type of buffer objects it creates.
 *
 * typedef T value_type;
 *
 * T* create_buffer();
 *
 */

namespace buffer
{

   OAC_DECL_ERROR(out_of_bounds_error, invalid_input_error);
   OAC_DECL_ERROR(read_error, io_error);
   OAC_DECL_ERROR(write_error, io_error);

   template <typename T, typename Buffer>
   inline static T read_as(Buffer& b, std::uint32_t offset)
   throw (out_of_bounds_error, read_error)
   {
      T result;
      b.read(&result, offset, sizeof(T));
      return result;
   }

   template <typename T, typename Buffer>
   inline static void write_as(Buffer& b, std::uint32_t offset, const T& t)
   throw (out_of_bounds_error, write_error)
   {
      b.write(&t, offset, sizeof(T));
   }

   template <typename Buffer>
   inline static void fill(Buffer& b, std::uint8_t value)
   throw (write_error)
   {
      for (std::uint32_t i = 0; i < b.capacity(); i++)
         b.write(&value, i, 1);
   }
}

/**
 * Buffer of fixed capacity. This is buffer implementation of
 * fixed capacity set at object construction. It conforms Buffer concept.
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

      inline factory(size_t capacity) : _capacity(capacity) {}
      inline virtual fixed_buffer* create_buffer() const
      { return new fixed_buffer(_capacity); }

   private:
      size_t _capacity;
   };

   typedef factory factory_type;

   fixed_buffer(size_t capacity);

   ~fixed_buffer();

   inline std::size_t capacity() const
   { return _capacity; }

   void read(void* dst, std::uint32_t offset, std::uint32_t length) const
         throw (buffer::out_of_bounds_error);

   template <typename OutputStream>
   inline void read_to(
         OutputStream& dst,
         std::uint32_t offset,
         std::uint32_t length) const
   throw (buffer::out_of_bounds_error, buffer::read_error, stream::write_error)
   {
      check_bounds(offset, length);
      dst.write(&(_data[offset]), length);
   }

   void write(const void* src, std::uint32_t offset, std::uint32_t length)
         throw (buffer::out_of_bounds_error, buffer::write_error);

   template <typename InputStream>
   inline std::size_t write_from(
         InputStream& src,
         std::uint32_t offset,
         std::uint32_t length)
   throw (buffer::out_of_bounds_error, stream::read_error, buffer::write_error)
   {
      check_bounds(offset, length);
      return src.read(&(_data[offset]), length);
   }

   template <typename Buffer>
   inline void copy(const Buffer& src, std::uint32_t src_offset,
                    std::uint32_t dst_offset, std::size_t length)
   throw (buffer::out_of_bounds_error)
   {
      this->check_bounds(dst_offset, length);
      src.read(&(_data[dst_offset]), src_offset, length);
   }

private:

   BYTE* _data;
   size_t _capacity;

   void check_bounds(std::uint32_t offset, std::size_t length) const
         throw (buffer::out_of_bounds_error);
};

/**
 * Shifted buffer class. This class provides a decorator that shifts
 * the offset on read and write operations. The decorated buffer and
 * the shift are set in the constructor. After that, offsets in read 
 * or write are shifted. E.g., for shift 0x5600, a read/write on offset
 * 0x5678 would be made on decorated buffer on 0x0078. 
 */
template <typename Buffer>
class shifted_buffer
{
public:

   class factory
   {
   public:

      typedef shifted_buffer value_type;

      inline factory(const ptr<typename Buffer::factory_type>& backed_buffer_fact,
                     std::uint32_t shift)
         : _backed_buffer_fact(backed_buffer_fact), _shift(shift)
      {}
      
      inline virtual shifted_buffer* create_buffer() const
      { 
         return new shifted_buffer(_backed_buffer_fact->create_buffer(), _shift);
      }
      
   private:

      ptr<typename Buffer::factory_type> _backed_buffer_fact;
      std::uint32_t _shift;
   };

   typedef factory factory_type;

   inline shifted_buffer(const ptr<Buffer>& backed_buffer,
                         std::uint32_t offset_shift)
      : _backed_buffer(backed_buffer),
        _backed_capacity(backed_buffer->capacity()),
        _offset_shift(offset_shift)
   {}

   inline std::size_t capacity() const
   { return _backed_buffer->capacity(); }

   inline void read(void* dst, std::uint32_t offset, std::size_t length) const
   throw (buffer::out_of_bounds_error, buffer::read_error)
   {
      check_bounds(offset, length);
      _backed_buffer->read(dst, shift(offset), length);
   }

   template <typename OutputStream>
   inline void read(
         OutputStream& dst, std::uint32_t offset, std::size_t length) const
   throw (buffer::out_of_bounds_error, buffer::read_error, stream::write_error)
   {
      check_bounds(offset, length);
      _backed_buffer->read(dst, shift(offset), length);
   }

   inline void write(const void* src, std::uint32_t offset, std::size_t length)
   throw (buffer::out_of_bounds_error, buffer::write_error)
   {
      check_bounds(offset, length);
      _backed_buffer->write(src, shift(offset), length);
   }

   template <typename InputStream>
   inline std::size_t write(
         InputStream& src, std::uint32_t offset, std::size_t length)
   throw (buffer::out_of_bounds_error, stream::read_error, buffer::write_error)
   {
      check_bounds(offset, length);
      return _backed_buffer->write(src, shift(offset), length);
   }

   template <typename Buffer>
   inline void copy(const Buffer& src, std::uint32_t src_offset,
         std::uint32_t dst_offset, std::size_t length)
   throw (buffer::out_of_bounds_error, buffer::read_error, buffer::write_error)
   {
      check_bounds(dst_offset, length);
      _backed_buffer->copy(src, src_offset, shift(dst_offset), length);
   }

private:

   ptr<Buffer> _backed_buffer;
   std::size_t _backed_capacity;
   std::uint32_t _offset_shift;

   inline void check_bounds(std::uint32_t offset, std::uint32_t length) const
   throw (buffer::out_of_bounds_error)
   {
      if (offset < _offset_shift || shift(offset) + length > _backed_capacity)
         BOOST_THROW_EXCEPTION(buffer::out_of_bounds_error() <<
               lower_bound_info(0) <<
               upper_bound_info(_backed_capacity - 1) <<
               index_info(offset + length));
   }

   inline std::uint32_t shift(std::uint32_t offset) const
   { return offset - _offset_shift; }
};

/**
 * Double buffer class. This class decorates a couple of buffers, 
 * making it possible to detect modifications among them. 
 */
template <typename Buffer = fixed_buffer>
class double_buffer
{
public:

   class factory
   {
   public:

      typedef double_buffer value_type;

      inline factory(const ptr<typename Buffer::factory_type>& backed_buffer_fact) :
         _backed_buffer_fact(backed_buffer_fact)
      {}
      
      inline virtual double_buffer* create_buffer() const
      { 
         return new double_buffer(
               _backed_buffer_fact->create_buffer(),
               _backed_buffer_fact->create_buffer());
      }
      
   private:

      ptr<typename Buffer::factory_type> _backed_buffer_fact;
      DWORD _shift;
   };

   typedef factory factory_type;

   inline double_buffer(
         const ptr<Buffer>& backed_buffer_0,
         const ptr<Buffer>& backed_buffer_1) :
      _current_buffer(0)
   {
      _backed_buffer[0] = ptr<Buffer>(backed_buffer_0);
      _backed_buffer[1] = ptr<Buffer>(backed_buffer_1);
   }

   inline std::size_t capacity() const
   { return _backed_buffer[_current_buffer]->capacity(); }

   inline void read(void* dst, std::uint32_t offset, std::size_t length) const
   throw (buffer::out_of_bounds_error)
   { _backed_buffer[_current_buffer]->read(dst, offset, length); }

   template <typename OutputStream>
   inline void read(OutputStream& dst,
                    std::uint32_t offset,
                    std::size_t length) const
   throw (buffer::out_of_bounds_error, buffer::read_error, stream::write_error)
   { _backed_buffer[_current_buffer]->read(dst, offset, length); }

   inline void write(const void* src, std::uint32_t offset, std::size_t length)
   throw (buffer::out_of_bounds_error)
   { _backed_buffer[_current_buffer]->write(src, offset, length); }

   template <typename InputStream>
   inline std::size_t write(InputStream& src,
                            std::uint32_t offset,
                            std::size_t length)
   throw (buffer::out_of_bounds_error)
   { return _backed_buffer[_current_buffer]->write(src, offset, length); }

   template <typename Buffer>
   void copy(const Buffer& src,
             std::uint32_t src_offset,
             std::uint32_t dst_offset,
             std::size_t length)
   throw (buffer::out_of_bounds_error)
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

   ptr<Buffer> _backed_buffer[2];
   BYTE _current_buffer;
};

template <typename Buffer>
class buffer_input_stream {
public:

   inline buffer_input_stream(
         const ptr<Buffer>& buffer) : _buffer(buffer), _index(0) {}

   inline std::size_t read(void* buffer, std::size_t count)
   {
      auto remain = _buffer->capacity() - _index;
      if (count > remain)
         count = remain;
      _buffer->read(buffer, _index, count);
      _index += count;
      return count;
   }

   inline ptr<Buffer> get_buffer() const { return _buffer; }

private:

   ptr<Buffer> _buffer;
   DWORD _index;
};

template <typename Buffer>
class buffer_output_stream {
public:

   inline buffer_output_stream(
         const ptr<Buffer>& buffer) : _buffer(buffer), _index(0) {}

   inline std::size_t write(const void* buffer, std::size_t count)
   {
      auto remain = _buffer->capacity() - _index;
      auto nwrite = (count > remain) ? remain : count;
      _buffer->write(buffer, _index, nwrite);
      _index += nwrite;
      return nwrite;
   }

   inline void flush()
   { /* Well, it's a buffer. Nothing to be done here */ }

   inline ptr<Buffer> get_buffer() const { return _buffer; }

private:

   ptr<Buffer> _buffer;
   DWORD _index;

};

namespace buffer {

   template <typename Buffer>
   inline ptr<shifted_buffer<Buffer>> shift_buffer(
         const ptr<Buffer>& b, std::uint32_t shift)
   { return new shifted_buffer<Buffer>(b, shift); }

   template <typename BufferFactory>
   inline ptr<typename shifted_buffer<typename BufferFactory::value_type>::factory> shift_factory(
         const ptr<BufferFactory>& fact,
         std::uint32_t shift)
   {
      return new shifted_buffer<typename BufferFactory::value_type>::factory(
            fact, shift);
   }

   template <typename Buffer>
   inline ptr<double_buffer<Buffer>> dup_buffers(
         const ptr<Buffer>& b1, const ptr<Buffer>& b2)
   { return new double_buffer<Buffer>(b1, b2); }

   template <typename BufferFactory>
   inline ptr<typename double_buffer<typename BufferFactory::value_type>::factory> dup_factory(
         const ptr<BufferFactory>& fact)
   {
      return new double_buffer<typename BufferFactory::value_type>::factory(fact);
   }

   template <typename Buffer>
   inline ptr<buffer_input_stream<Buffer>> make_input_stream(
         const ptr<Buffer>& b)
   { return new buffer_input_stream<Buffer>(b); }

   template <typename Buffer>
   inline ptr<buffer_output_stream<Buffer>> make_output_stream(
         const ptr<Buffer>& b)
   { return new buffer_output_stream<Buffer>(b); }
}

} // namespace oac

#endif
