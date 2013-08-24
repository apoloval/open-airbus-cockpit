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

#ifndef OAC_BUFFER_H
#define OAC_BUFFER_H

#pragma warning( disable : 4290 )

#include <list>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <boost/asio/buffer.hpp>
#include <boost/format.hpp>

#include "exception.h"
#include "io.h"
#include "lang-utils.h"
#include "stream.h"

namespace oac {

/**
 * @concept Buffer
 *
 * A class that may be used as data storage, allowing reading from and
 * writing to bytes. Buffer concept extends the InputStream and OutputStream
 * concepts, so it implements their members plus:
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
 * A class that may be used to create buffer objects under a common interface.
 * It provides the following members, where T is the type of buffer objects
 * it creates.
 *
 * typedef T value_type;
 *
 * T* create_buffer();
 *
 */

/**
 * @concept StreamBuffer
 *
 * A class able to store data as Buffer but allows stream-like read and write
 * operations. It conforms the Buffer and Stream concepts, with the following
 * additional operations.
 *
 * std::size_t available_for_read() const;
 *
 * std::size_t available_for_write() const;
 *
 * boost::optional<std::uint32_t> mark() const;
 *
 * void set_mark();
 *
 * void unset_mark();
 *
 * void reset();
 *
 * template <typename AsyncReadStream,
 *           typename ReadHandler>
 * void async_write_some_from(AsyncReadStream& stream, ReadHandler handler);
 *
 * template <typename AsyncWriteStream,
 *           typename WriteHandler>
 * void async_read_some_to(AsyncWriteStream& stream, WriteHandler handler);
 *
 */

namespace buffer
{

   typedef std::function<void(
         const boost::system::error_code&,
         std::size_t)> async_io_handler;

   /**
    * An exception indicating a invalid index provided in a buffer operation.
    */
   OAC_EXCEPTION_BEGIN(index_out_of_bounds, exception)
      OAC_EXCEPTION_FIELD(lower_bound, int)
      OAC_EXCEPTION_FIELD(upper_bound, int)
      OAC_EXCEPTION_FIELD(index, int)
      OAC_EXCEPTION_MSG(
            "buffer access with invalid index %d for bounds [%d, %d]",
            index,
            lower_bound,
            upper_bound)
   OAC_EXCEPTION_END()

   template <typename T, typename Buffer>
   T read_as(
         Buffer& b, std::uint32_t offset)
         throw (index_out_of_bounds, io_exception);

   template <typename T, typename Buffer>
   void write_as(
         Buffer& b, std::uint32_t offset, const T& t)
         throw (index_out_of_bounds, io_exception);

   template <typename Buffer>
   void fill(Buffer& b, std::uint8_t value) throw (io_exception);

   template <typename AsyncReadStream,
             typename StreamBuffer,
             typename ReadHandler>
   void async_read_some(
         AsyncReadStream& stream,
         StreamBuffer& buffer,
         ReadHandler handler);

   template <typename AsyncWriteStream,
             typename StreamBuffer,
             typename WriteHandler>
   void async_write_some(
         AsyncWriteStream& stream,
         StreamBuffer& buffer,
         WriteHandler handler);
}

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

   void on_async_read(
         buffer::async_io_handler handler,
         const boost::system::error_code& ec,
         std::size_t nbytes);

   void on_async_write(
         buffer::async_io_handler handler,
         const boost::system::error_code& ec,
         std::size_t nbytes);

private:

   Buffer* _self;
   std::uint32_t _read_index;
   std::uint32_t _write_index;
   boost::optional<std::uint32_t> _mark;
};

template <typename Buffer>
class ring_stream_buffer_base
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
   void async_write_some_from(
         AsyncReadStream& stream,
         ReadHandler handler);

   template <typename AsyncWriteStream,
             typename WriteHandler>
   void async_read_some_to(
         AsyncWriteStream& stream,
         WriteHandler handler);

protected:

   ring_stream_buffer_base(Buffer* self);

private:

   bool is_broken() const;

   void inc_dist_from_mark(std::size_t amount);

   std::list<boost::asio::mutable_buffer> asio_mutable_buffers();

   std::list<boost::asio::mutable_buffer> asio_mutable_buffers(
         std::size_t nbytes);

   std::list<boost::asio::const_buffer> asio_const_buffers();

   std::list<boost::asio::const_buffer> asio_const_buffers(
         std::size_t nbytes);

   void on_async_read(
         buffer::async_io_handler handler,
         const boost::system::error_code& ec,
         std::size_t nbytes);

   void on_async_write(
         buffer::async_io_handler handler,
         const boost::system::error_code& ec,
         std::size_t nbytes);

   Buffer* _self;
   std::uint32_t _read_index;
   std::uint32_t _write_index;
   std::size_t _bytes_written;
   boost::optional<std::uint32_t> _mark;
   std::size_t _dist_from_mark;
};

/**
 * Buffer of fixed capacity. This is buffer implementation of
 * fixed capacity set at object construction. It conforms Buffer concept but
 * not Stream. For streamed buffer please check linear_buffer or ring_buffer
 * classes.
 */
class fixed_buffer : public shared_by_ptr<fixed_buffer>
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

   typedef factory factory_type;

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

class linear_buffer :
      public shared_by_ptr<linear_buffer>,
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

   typedef factory factory_type;

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

class ring_buffer :
      public shared_by_ptr<ring_buffer>,
      public ring_stream_buffer_base<fixed_buffer>
{
public:

   using ring_stream_buffer_base::read;
   using ring_stream_buffer_base::write;

   class factory
   {
   public:

      typedef ring_buffer value_type;

      factory(std::size_t capacity);

      virtual value_type* create_buffer() const;

   private:

      std::size_t _capacity;
   };

   typedef factory factory_type;

   ring_buffer(std::size_t capacity);

   ring_buffer(const ring_buffer& buff);

   ring_buffer(ring_buffer&& buff);

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
         const Buffer& src,
         std::uint32_t src_offset,
         std::uint32_t dst_offset,
         std::size_t length)
   throw (buffer::index_out_of_bounds);

private:

   fixed_buffer _fixed_buffer;
};

/**
 * Shifted buffer class. This class provides a decorator that shifts
 * the offset on read and write operations. The decorated buffer and
 * the shift are set in the constructor. After that, offsets in read 
 * or write are shifted. E.g., for shift 0x5600, a read/write on offset
 * 0x5678 would be made on decorated buffer on 0x0078. 
 */
template <typename Buffer>
class shifted_buffer :
      public shared_by_ptr<shifted_buffer<Buffer>>,
      public linear_stream_buffer_base<shifted_buffer<Buffer>>
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

   ptr<Buffer> _backed_buffer;
   std::size_t _backed_capacity;
   std::uint32_t _offset_shift;

   inline void check_bounds(std::uint32_t offset, std::uint32_t length) const
   throw (buffer::index_out_of_bounds)
   {
      if (offset < _offset_shift || shift(offset) + length > _backed_capacity)
         OAC_THROW_EXCEPTION(buffer::index_out_of_bounds()
               .with_lower_bound(0)
               .with_upper_bound(_backed_capacity - 1)
               .with_index(offset + length));
   }

   inline std::uint32_t shift(std::uint32_t offset) const
   { return offset - _offset_shift; }
};

/**
 * Double buffer class. This class decorates a couple of buffers, 
 * making it possible to detect modifications among them. 
 */
template <typename Buffer = linear_buffer>
class double_buffer :
      public shared_by_ptr<double_buffer<Buffer>>,
      public linear_stream_buffer_base<double_buffer<Buffer>>
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
      linear_stream_buffer_base<double_buffer<Buffer>>(this),
      _current_buffer(0)
   {
      _backed_buffer[0] = ptr<Buffer>(backed_buffer_0);
      _backed_buffer[1] = ptr<Buffer>(backed_buffer_1);
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

   ptr<Buffer> _backed_buffer[2];
   std::uint8_t _current_buffer;
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
}

} // namespace oac

#include "buffer.inl"

#endif
