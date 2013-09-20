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

#ifndef OAC_BUFFER_LINEAR_INL
#define OAC_BUFFER_LINEAR_INL

#include "liboac/buffer/linear.h"

namespace oac { namespace buffer {

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
          typename AsyncReadHandler>
void
linear_stream_buffer_base<Buffer>::async_write_some_from(
      AsyncReadStream& stream,
      AsyncReadHandler handler)
{
   auto on_read = buffer::make_io_handler(
         handler,
         [this](std::size_t bytes_read)
         {
            _write_index += bytes_read;
         });

   stream.async_read_some(asio_mutable_buffers(), on_read);
}

template <typename Buffer>
template <typename AsyncWriteStream,
          typename AsyncWriteHandler>
void
linear_stream_buffer_base<Buffer>::async_read_some_to(
      AsyncWriteStream& stream,
      AsyncWriteHandler handler)
{
   auto on_write = buffer::make_io_handler(
         handler,
         [this](std::size_t bytes_written)
         {
            _read_index += bytes_written;
         });
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
throw (buffer::index_out_of_bounds)
{ _fixed_buffer.read(dst, offset, length); }

template <typename OutputStream>
void
linear_buffer::read_to(
      OutputStream& dst, std::uint32_t offset, std::size_t length) const
throw (buffer::index_out_of_bounds, io_exception)
{ _fixed_buffer.read_to(dst, offset, length); }

inline void
linear_buffer::write(
      const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::index_out_of_bounds)
{ _fixed_buffer.write(src, offset, length); }

template <typename InputStream>
std::size_t
linear_buffer::write_from(
      InputStream& src, std::uint32_t offset, std::size_t length)
throw (buffer::index_out_of_bounds, io_exception)
{ return _fixed_buffer.write_from(src, offset, length); }

template <typename Buffer>
void
linear_buffer::copy(
      const Buffer& src, std::uint32_t src_offset,
      std::uint32_t dst_offset, std::size_t length)
throw (buffer::index_out_of_bounds)
{ _fixed_buffer.copy(src, src_offset, dst_offset, length); }

}} // namespace oac::buffer

#endif
