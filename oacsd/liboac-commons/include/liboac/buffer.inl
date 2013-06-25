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

#ifndef OAC_BUFFER_INL
#define OAC_BUFFER_INL

#include <boost/asio/read.hpp>

#include "buffer.h"

namespace oac {

namespace buffer {

template <typename T, typename Buffer>
T read_as(Buffer& b, std::uint32_t offset)
throw (out_of_bounds_error, read_error)
{
   T result;
   b.read(&result, offset, sizeof(T));
   return result;
}

template <typename T, typename Buffer>
void write_as(Buffer& b, std::uint32_t offset, const T& t)
throw (out_of_bounds_error, write_error)
{
   b.write(&t, offset, sizeof(T));
}

template <typename Buffer>
void fill(Buffer& b, std::uint8_t value)
throw (write_error)
{
   for (std::uint32_t i = 0; i < b.capacity(); i++)
      b.write(&value, i, 1);
}

template <typename AsyncReadStream,
          typename StreamedBuffer,
          typename ReadHandler>
void
async_read_some(
      AsyncReadStream& stream,
      StreamedBuffer& buffer,
      ReadHandler handler)
{
   buffer.async_write_some_from(stream, handler);
}

template <typename AsyncWriteStream,
          typename StreamBuffer,
          typename WriteHandler>
void
async_write_some(
      AsyncWriteStream& stream,
      StreamBuffer& buffer,
      WriteHandler handler)
{
   buffer.async_read_some_to(stream, handler);
}


} // namespace buffer

template <typename Buffer>
std::size_t
linear_stream_buffer_base<Buffer>::read(
      void* dest, std::size_t count)
{
   auto remain = _write_index - _read_index;
   auto nread = (remain > count) ? count : remain;
   _self->read(dest, _read_index, nread);
   _read_index += nread;
   return nread;
}

template <typename Buffer>
std::size_t
linear_stream_buffer_base<Buffer>::write(
      const void* src, std::size_t count)
{
   auto remain = _self->capacity() - _write_index;
   auto nwrite = (remain > count) ? count : remain;
   _self-> write(src, _write_index, nwrite);
   _write_index += nwrite;
   return nwrite;
}

template <typename Buffer>
void
linear_stream_buffer_base<Buffer>::flush()
{
   // Nothing to be done for flushing an in-memory buffer
}

template <typename Buffer>
std::size_t
linear_stream_buffer_base<Buffer>::available_for_read() const
{ return _write_index - _read_index; }

template <typename Buffer>
std::size_t
linear_stream_buffer_base<Buffer>::available_for_write() const
{ return _self->capacity() - _write_index; }

template <typename Buffer>
boost::optional<std::uint32_t>
linear_stream_buffer_base<Buffer>::mark() const
{
   return _mark;
}

template <typename Buffer>
void
linear_stream_buffer_base<Buffer>::set_mark()
{
   _mark = _read_index;
}

template <typename Buffer>
void
linear_stream_buffer_base<Buffer>::unset_mark()
{
   _mark.reset();
}

template <typename Buffer>
void
linear_stream_buffer_base<Buffer>::reset()
{
   if (_mark)
   {
      _read_index = *_mark;
   }
}

template <typename Buffer>
template <typename AsyncReadStream,
          typename ReadHandler>
void
linear_stream_buffer_base<Buffer>::async_write_some_from(
      AsyncReadStream& stream,
      ReadHandler handler)
{
   buffer::async_io_handler on_read(
            std::bind(
               &linear_stream_buffer_base<Buffer>::on_async_read,
               this,
               buffer::async_io_handler(handler),
               std::placeholders::_1,
               std::placeholders::_2));
   stream.async_read_some(asio_mutable_buffers(), on_read);
}

template <typename Buffer>
template <typename AsyncWriteStream,
          typename WriteHandler>
void
linear_stream_buffer_base<Buffer>::async_read_some_to(
      AsyncWriteStream& stream,
      WriteHandler handler)
{
   buffer::async_io_handler on_write(
            std::bind(
               &linear_stream_buffer_base<Buffer>::on_async_write,
               this,
               buffer::async_io_handler(handler),
               std::placeholders::_1,
               std::placeholders::_2));
   stream.async_write_some(asio_const_buffers(), on_write);
}

template <typename Buffer>
linear_stream_buffer_base<Buffer>::linear_stream_buffer_base(
      Buffer* self)
   : _self(self),
     _read_index(0),
     _write_index(0)
{}

template <typename Buffer>
linear_stream_buffer_base<Buffer>::linear_stream_buffer_base(
      const linear_stream_buffer_base& base, Buffer* self)
   : _self(self),
     _read_index(base._read_index),
     _write_index(base._write_index)
{}

template <typename Buffer>
linear_stream_buffer_base<Buffer>::linear_stream_buffer_base(
      linear_stream_buffer_base&& base, Buffer* self)
   : _self(self),
     _read_index(base._read_index),
     _write_index(base._write_index)
{
   base._read_index = 0;
   base._write_index = 0;
}

template <typename Buffer>
std::list<boost::asio::mutable_buffer>
linear_stream_buffer_base<Buffer>::asio_mutable_buffers()
{ return asio_mutable_buffers(available_for_write()); }

template <typename Buffer>
std::list<boost::asio::mutable_buffer>
linear_stream_buffer_base<Buffer>::asio_mutable_buffers(std::size_t nbytes)
{
   auto available = available_for_write();
   if (nbytes > available)
      nbytes = available;
   std::list<boost::asio::mutable_buffer> result;
   auto data = ((std::uint8_t*) _self->data()) + _write_index;
   result.push_back(boost::asio::buffer(data, nbytes));
   return result;
}

template <typename Buffer>
std::list<boost::asio::const_buffer>
linear_stream_buffer_base<Buffer>::asio_const_buffers()
{ return asio_const_buffers(available_for_read()); }

template <typename Buffer>
std::list<boost::asio::const_buffer>
linear_stream_buffer_base<Buffer>::asio_const_buffers(std::size_t nbytes)
{
   auto available = available_for_read();
   if (nbytes > available)
      nbytes = available;
   std::list<boost::asio::const_buffer> result;
   auto data = ((std::uint8_t*) _self->data()) + _read_index;
   result.push_back(boost::asio::buffer(data, nbytes));
   return result;
}

template <typename Buffer>
void
linear_stream_buffer_base<Buffer>::on_async_read(
      buffer::async_io_handler handler,
      const boost::system::error_code& ec,
      std::size_t nbytes)
{
   _read_index += nbytes;
   handler(ec, nbytes);
}

template <typename Buffer>
void
linear_stream_buffer_base<Buffer>::on_async_write(
      buffer::async_io_handler handler,
      const boost::system::error_code& ec,
      std::size_t nbytes)
{
   _write_index += nbytes;
   handler(ec, nbytes);
}



template <typename Buffer>
std::size_t
ring_stream_buffer_base<Buffer>::read(
      void* dest, std::size_t count)
{
   count = std::min(count, available_for_read());
   if (is_broken())
   {
      auto rem = _self->capacity() - _read_index;
      auto final_count = std::min(count, rem);
      _self->read(dest, _read_index, final_count);
      _bytes_written -= final_count;
      inc_dist_from_mark(final_count);
      if (count < rem)
      {
         _read_index += count;
         return count;
      }
      else
      {
         _read_index = 0;
         return rem + read((((std::uint8_t*) dest) + rem), count - rem);
      }
   }
   else
   {
      _self->read(dest, _read_index, count);
      _read_index += count;
      _bytes_written -= count;
      inc_dist_from_mark(count);
      return count;
   }
}

template <typename Buffer>
std::size_t
ring_stream_buffer_base<Buffer>::write(
      const void* src, std::size_t count)
{
   count = std::min(count, available_for_write());
   if (is_broken())
   {
      _self->write(src, _write_index, count);
      _write_index += count;
      _bytes_written += count;
      return count;
   }
   else
   {
      auto rem = _self->capacity() - _write_index;
      auto final_count = std::min(count, rem);
      _self->write(src, _write_index, final_count);
      _bytes_written += final_count;
      if (count < rem)
      {
         _write_index += count;
         return count;
      }
      else
      {
         _write_index = 0;
         return rem + write((((std::uint8_t*) src) + rem), count - rem);
      }
   }
}

template <typename Buffer>
void
ring_stream_buffer_base<Buffer>::flush()
{
   // As in-memory buffer, nothing to do
}

template <typename Buffer>
std::size_t
ring_stream_buffer_base<Buffer>::available_for_read() const
{
   return _bytes_written;
}

template <typename Buffer>
std::size_t
ring_stream_buffer_base<Buffer>::available_for_write() const
{
   return _self->capacity() - _bytes_written - _dist_from_mark;
}

template <typename Buffer>
boost::optional<std::uint32_t>
ring_stream_buffer_base<Buffer>::mark() const
{
   return _mark;
}

template <typename Buffer>
void
ring_stream_buffer_base<Buffer>::set_mark()
{
   _mark = _read_index;
}

template <typename Buffer>
void
ring_stream_buffer_base<Buffer>::unset_mark()
{
   _mark.reset();
   _dist_from_mark = 0;
}

template <typename Buffer>
void
ring_stream_buffer_base<Buffer>::reset()
{
   if (_mark)
   {
      _bytes_written += _dist_from_mark;
      _dist_from_mark = 0;
      _read_index = *_mark;
   }
}

template <typename Buffer>
template <typename AsyncReadStream,
          typename ReadHandler>
void
ring_stream_buffer_base<Buffer>::async_write_some_from(
      AsyncReadStream& stream,
      ReadHandler handler)
{
   buffer::async_io_handler on_read(
         std::bind(
               &ring_stream_buffer_base<Buffer>::on_async_read,
               this,
               buffer::async_io_handler(handler),
               std::placeholders::_1,
               std::placeholders::_2));
   stream.async_read_some(
         asio_mutable_buffers(),
         on_read);
}

template <typename Buffer>
template <typename AsyncWriteStream,
          typename WriteHandler>
void
ring_stream_buffer_base<Buffer>::async_read_some_to(
      AsyncWriteStream& stream,
      WriteHandler handler)
{
   buffer::async_io_handler on_write(
         std::bind(
            &ring_stream_buffer_base<Buffer>::on_async_write,
            this,
            buffer::async_io_handler(handler),
            std::placeholders::_1,
            std::placeholders::_2));
   stream.async_write_some(
         asio_const_buffers(),
         on_write);
}

template <typename Buffer>
ring_stream_buffer_base<Buffer>::ring_stream_buffer_base(
      Buffer* self)
   : _self(self),
     _read_index(0),
     _write_index(0),
     _bytes_written(0),
     _mark(),
     _dist_from_mark(0)
{}

template <typename Buffer>
bool
ring_stream_buffer_base<Buffer>::is_broken() const
{
   return (_read_index > _write_index);
}

template <typename Buffer>
void
ring_stream_buffer_base<Buffer>::inc_dist_from_mark(
      std::size_t amount)
{
   if (_mark)
      _dist_from_mark += amount;
}

template <typename Buffer>
std::list<boost::asio::mutable_buffer>
ring_stream_buffer_base<Buffer>::asio_mutable_buffers()
{
   return asio_mutable_buffers(available_for_write());
}

template <typename Buffer>
std::list<boost::asio::mutable_buffer>
ring_stream_buffer_base<Buffer>::asio_mutable_buffers(std::size_t nbytes)
{
   std::list<boost::asio::mutable_buffer> result;
   nbytes = std::min(nbytes, available_for_write());

   if (is_broken())
   {
      auto chunk_len = std::min(nbytes, _read_index - _write_index);
      auto data = ((std::uint8_t*) _self->data()) + _write_index;
      result.push_back(boost::asio::buffer(data, chunk_len));
   }
   else
   {
      auto chunk_len = std::min(nbytes, _self->capacity() - _write_index);
      auto data = ((std::uint8_t*) _self->data()) + _write_index;
      result.push_back(boost::asio::buffer(data, chunk_len));

      nbytes -= chunk_len;
      chunk_len = std::min(nbytes, _read_index);
      result.push_back(boost::asio::buffer(_self->data(), chunk_len));
   }
   return result;
}

template <typename Buffer>
std::list<boost::asio::const_buffer>
ring_stream_buffer_base<Buffer>::asio_const_buffers()
{
   return asio_const_buffers(available_for_write());
}

template <typename Buffer>
std::list<boost::asio::const_buffer>
ring_stream_buffer_base<Buffer>::asio_const_buffers(std::size_t nbytes)
{
   std::list<boost::asio::const_buffer> result;
   nbytes = std::min(nbytes, available_for_write());

   if (is_broken())
   {
      auto chunk_len = std::min(nbytes, _self->capacity() - _read_index);
      auto data = ((std::uint8_t*) _self->data()) + _read_index;
      result.push_back(boost::asio::buffer(data, chunk_len));

      nbytes -= chunk_len;
      chunk_len = std::min(nbytes, _write_index);
      result.push_back(boost::asio::buffer(_self->data(), chunk_len));
   }
   else
   {
      auto chunk_len = std::min(nbytes, _write_index - _read_index);
      auto data = ((std::uint8_t*) _self->data()) + _read_index;
      result.push_back(boost::asio::buffer(data, chunk_len));
   }
   return result;
}

template <typename Buffer>
void
ring_stream_buffer_base<Buffer>::on_async_read(
      buffer::async_io_handler handler,
      const boost::system::error_code& ec,
      std::size_t nbytes)
{
   _write_index = (_write_index + nbytes) % _self->capacity();
   _bytes_written += nbytes;
   handler(ec, nbytes);
}

template <typename Buffer>
void
ring_stream_buffer_base<Buffer>::on_async_write(
      buffer::async_io_handler handler,
      const boost::system::error_code& ec,
      std::size_t nbytes)
{
   _read_index = (_read_index + nbytes) % _self->capacity();
   inc_dist_from_mark(nbytes);
   _bytes_written -= nbytes;
   handler(ec, nbytes);
}




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
throw (buffer::out_of_bounds_error)
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
throw (buffer::out_of_bounds_error, buffer::read_error, stream::write_error)
{
   check_bounds(offset, length);
   dst.write(&(_data[offset]), length);
}

inline void
fixed_buffer::write(const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::out_of_bounds_error)
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
throw (buffer::out_of_bounds_error, stream::read_error, buffer::write_error)
{
   check_bounds(offset, length);
   return src.read(&(_data[offset]), length);
}

template <typename Buffer>
void
fixed_buffer::copy(const Buffer& src, std::uint32_t src_offset,
                 std::uint32_t dst_offset, std::size_t length)
throw (buffer::out_of_bounds_error)
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
throw (buffer::out_of_bounds_error)
{
   if (offset + length > _capacity)
      BOOST_THROW_EXCEPTION(buffer::out_of_bounds_error() <<
            lower_bound_info(0) <<
            upper_bound_info(_capacity - 1) <<
            index_info(offset + length));
}



inline
linear_buffer::factory::factory(size_t capacity) : _capacity(capacity) {}

inline linear_buffer::factory::value_type*
linear_buffer::factory::create_buffer() const
{ return new linear_buffer(_capacity); }

inline
linear_buffer::linear_buffer(std::size_t capacity)
   : linear_stream_buffer_base<fixed_buffer>(&_fixed_buffer),
     _fixed_buffer(capacity)
{}

inline
linear_buffer::linear_buffer(const linear_buffer& buff)
   : linear_stream_buffer_base<fixed_buffer>(buff),
     _fixed_buffer(buff._fixed_buffer)
{}

inline
linear_buffer::linear_buffer(linear_buffer&& buff)
   : linear_stream_buffer_base<fixed_buffer>(buff),
     _fixed_buffer(buff._fixed_buffer)
{}

inline std::size_t
linear_buffer::capacity() const
{ return _fixed_buffer.capacity(); }

inline void
linear_buffer::read(void* dst, std::uint32_t offset, std::size_t length) const
throw (buffer::out_of_bounds_error)
{ _fixed_buffer.read(dst, offset, length); }

template <typename OutputStream>
void
linear_buffer::read_to(
      OutputStream& dst, std::uint32_t offset, std::size_t length) const
throw (buffer::out_of_bounds_error, stream::write_error)
{ _fixed_buffer.read_to(dst, offset, length); }

inline void
linear_buffer::write(
      const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::out_of_bounds_error)
{ _fixed_buffer.write(src, offset, length); }

template <typename InputStream>
std::size_t
linear_buffer::write_from(
      InputStream& src, std::uint32_t offset, std::size_t length)
throw (buffer::out_of_bounds_error, stream::read_error)
{ return _fixed_buffer.write_from(src, offset, length); }

template <typename Buffer>
void
linear_buffer::copy(
      const Buffer& src, std::uint32_t src_offset,
      std::uint32_t dst_offset, std::size_t length)
throw (buffer::out_of_bounds_error)
{ _fixed_buffer.copy(src, src_offset, dst_offset, length); }

inline
ring_buffer::factory::factory(std::size_t capacity)
   : _capacity(capacity)
{}

inline ring_buffer::factory::value_type*
ring_buffer::factory::create_buffer() const
{ return new ring_buffer(_capacity); }

inline
ring_buffer::ring_buffer(std::size_t capacity)
   : ring_stream_buffer_base(&_fixed_buffer),
     _fixed_buffer(capacity)
{}

inline
ring_buffer::ring_buffer(const ring_buffer& buff)
   : ring_stream_buffer_base(&_fixed_buffer),
     _fixed_buffer(buff._fixed_buffer)
{}

inline
ring_buffer::ring_buffer(ring_buffer&& buff)
   : ring_stream_buffer_base(&_fixed_buffer),
     _fixed_buffer(buff._fixed_buffer)
{}

inline std::size_t
ring_buffer::capacity() const
{ return _fixed_buffer.capacity(); }

inline void
ring_buffer::read(
      void* dst, std::uint32_t offset, std::size_t length) const
throw (buffer::out_of_bounds_error)
{ _fixed_buffer.read(dst, offset, length); }

template <typename OutputStream>
void
ring_buffer::read_to(
      OutputStream& dst, std::uint32_t offset, std::size_t length) const
throw (buffer::out_of_bounds_error, stream::write_error)
{ _fixed_buffer.read_to(dst, offset, length); }

inline void
ring_buffer::write(
      const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::out_of_bounds_error)
{ _fixed_buffer.write(src, offset, length); }

template <typename InputStream>
std::size_t
ring_buffer::write_from(
      InputStream& src, std::uint32_t offset, std::size_t length)
throw (buffer::out_of_bounds_error, stream::read_error)
{ return _fixed_buffer.write_from(src, offset, length); }

template <typename Buffer>
void
ring_buffer::copy(
      const Buffer& src, std::uint32_t src_offset,
      std::uint32_t dst_offset, std::size_t length)
throw (buffer::out_of_bounds_error)
{ _fixed_buffer.copy(src, src_offset, dst_offset, length); }

} // namespace oac

#endif
