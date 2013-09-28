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

#ifndef OAC_BUFFER_RING_INL
#define OAC_BUFFER_RING_INL

#include <liboac/buffer/asio_handler.h>
#include <liboac/buffer/ring.h>

namespace oac { namespace buffer {

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
          typename AsyncReadHandler>
void
ring_stream_buffer_base<Buffer>::async_write_some_from(
      AsyncReadStream& stream,
      AsyncReadHandler handler)
{
   auto on_read = buffer::make_io_handler(
         handler,
         [this](std::size_t bytes_read)
         {
            _write_index = (_write_index + bytes_read) % _self->capacity();
            _bytes_written += bytes_read;
         });
   stream.async_read_some(asio_mutable_buffers(), on_read);
}

template <typename Buffer>
template <typename AsyncWriteStream,
          typename AsyncWriteHandler>
void
ring_stream_buffer_base<Buffer>::async_read_some_to(
      AsyncWriteStream& stream,
      AsyncWriteHandler handler)
{
   auto on_write = buffer::make_io_handler(
         handler,
         [this](std::size_t bytes_read)
         {
            _read_index = (_read_index + bytes_read) % _self->capacity();
            inc_dist_from_mark(bytes_read);
            _bytes_written -= bytes_read;
         });
   stream.async_write_some(asio_const_buffers(), on_write);
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
throw (buffer::index_out_of_bounds)
{ _fixed_buffer.read(dst, offset, length); }

template <typename OutputStream>
void
ring_buffer::read_to(
      OutputStream& dst, std::uint32_t offset, std::size_t length) const
throw (buffer::index_out_of_bounds, io_exception)
{ _fixed_buffer.read_to(dst, offset, length); }

inline void
ring_buffer::write(
      const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::index_out_of_bounds)
{ _fixed_buffer.write(src, offset, length); }

template <typename InputStream>
std::size_t
ring_buffer::write_from(
      InputStream& src, std::uint32_t offset, std::size_t length)
throw (buffer::index_out_of_bounds, io_exception)
{ return _fixed_buffer.write_from(src, offset, length); }

template <typename Buffer>
void
ring_buffer::copy(
      const Buffer& src, std::uint32_t src_offset,
      std::uint32_t dst_offset, std::size_t length)
throw (buffer::index_out_of_bounds)
{ _fixed_buffer.copy(src, src_offset, dst_offset, length); }

}} // namespace oac::buffer

#endif
