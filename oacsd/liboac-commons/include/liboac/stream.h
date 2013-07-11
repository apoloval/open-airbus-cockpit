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

#ifndef OAC_STREAM_H
#define OAC_STREAM_H

#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "exception.h"
#include "lang-utils.h"

namespace oac {

/**
 * @concept InputStream
 *
 * A class which provides a member to read bytes from with the signature:
 *
 * std::size_t InputStream::read(
 *       void* dest, std::size_t count) throw (io_error);
 *
 * The read() operation is synchronous, so the caller shall be blocked if
 * there is no available data in the stream. When data is available, the bytes
 * are copied into dest buffer. read() may not read all of the requested number
 * of bytes if all them are not available yet. For that, use stream::read_all()
 * function instead. When the stream is closed by the other end, a call to
 * read() shall read the remaining bytes which were not consumed, if any.
 * After that, any subsequent call to read() shall return 0.
 *
 */

/**
 * @concept OutputStream
 *
 * A class which provides a member to write bytes to, with the signature:
 *
 * std::size_t OutputStream::write(
 *       const void* src, std::size_t count) throw (io_error);
 *
 * void OutputStream::flush();
 *
 * The write() operation is synchronous, so the caller shall be blocked if
 * it is not possible to write any byte yet. When data may be write, the bytes
 * are copied from src buffer. write() may not write all of the requested
 * number of bytes. For that, use stream::write_all() function instead. When
 * the stream is closed by the other end, a call to write() shall return 0.
 *
 * The flush() operation forces the sending of any pending byte that was not
 * sent to the device.
 */

namespace stream {

OAC_DECL_ERROR(eof_error, io_error);
OAC_DECL_ERROR(read_error, io_error);
OAC_DECL_ERROR(write_error, io_error);

/**
 * Read count bytes from the stream, waiting for new data to arrive if
 * there are no enough bytes. While read() returns even when less bytes
 * than requested have arrived, read_all() waits until every requested
 * byte is available. If the stream is closed before that, a eof_error
 * is thrown.
 *
 * @tparam InputStream A type conforming InputStream concept
 * @param buffer the buffer where store the read elements. It must
 *               have at least count allocated bytes
 * @param count the count of bytes to read
 */
template <typename InputStream>
inline void read_all(InputStream& s, void* dest, std::size_t count)
throw (io_error)
{
   auto p = (std::uint8_t*) dest;
   while (count)
   {
      auto nread = s.read(p, count);
      if (nread == 0)
         BOOST_THROW_EXCEPTION(eof_error());
      p += nread;
      count -= nread;
   }
}

/**
 * Read an object of type T from given InputStream object. The caller will
 * block until at least sizeof(T) bytes are available in the stream. If the
 * stream is closed before that, a eof_error is thrown.
 */
template <typename T, typename InputStream>
inline T read_as(InputStream& s)
throw (io_error)
{
   T r;
   read_all(s, &r, sizeof(T));
   return r;
}

/**
 * Read a string of the given length from the InputStream. The caller will
 * block until at requested bytes are available in the stream. If the
 * stream is closed before that, a eof_error is thrown.
 */
template <typename InputStream>
inline std::string read_as_string(InputStream& s, unsigned int len)
throw (io_error)
{
   char* buff = new char[len];
   read_all(s, buff, len);
   std::string r(buff, len);
   delete buff;
   return r;
}

/**
 * Read a line terminated with a '\n' symbol from the InputStream. If the
 * stream is closed before line termination is found, the characters read to
 * that point are returned.
 */
template <typename InputStream>
inline std::string read_line(InputStream& s)
{
   static const unsigned CHUNK_SIZE = 64;
   CHAR buff[CHUNK_SIZE];
   unsigned int i = 0;

   for (i = 0; i < CHUNK_SIZE; i++)
   {
      if (!s.read(&(buff[i]), 1) || buff[i] == '\n')
         return std::string(buff, i);
   }
   return std::string(buff, CHUNK_SIZE) + read_line(s);
}

/**
 * Write count bytes from src buffer into OutputStream object. This function
 * will block until all the bytes are written. If the other end of the stream
 * is closed before all bytes are written, an eof_error is thrown.
 */
template <typename OutputStream>
inline void write_all(OutputStream& s, const void* src, std::size_t count)
throw (io_error)
{
   auto p = (const std::uint8_t*) src;
   while (count)
   {
      auto nwrite = s.write(p, count);
      if (nwrite == 0)
         BOOST_THROW_EXCEPTION(eof_error());
      p += nwrite;
      count -= nwrite;
   }
}

/**
 * Write given string into the OutputStream. This just writes the characters
 * without any termination symbol. If the other end of the stream is closed
 * before all bytes are written, an eof_error is thrown.
 */
template <typename OutputStream>
inline void write_as_string(
      OutputStream& s, const std::string& str)
throw (io_error)
{ write_all(s, str.c_str(), str.length()); }

/**
 * Write given object of type T into the OutputStream. If the other end of the
 * stream is closed before all bytes are written, an eof_error is thrown.
 */
template <typename T, typename OutputStream>
inline void write_as(OutputStream& s, const T& t)
throw (io_error)
{ write_all(s, &t, sizeof(T)); }

} // namespace stream

/**
 * An input stream which adapts a Boost ASIO SyncReadStream object to
 * conform InputStream concept.
 */
template <typename SyncReadStream>
class sync_read_stream_adapter
{
public:

   inline sync_read_stream_adapter(const ptr<SyncReadStream>& adapted)
      : _adapted(adapted)
   {}

   inline size_t read(void* dest, size_t count)
   {
      try
      { return _adapted->read_some(boost::asio::buffer(dest, count)); }
      catch (boost::system::system_error& e)
      {
         if (e.code() == boost::asio::error::eof)
            return 0;
         else
            BOOST_THROW_EXCEPTION(
                     stream::read_error() <<
                     boost_system_error_info(e) <<
                     message_info(e.what()));
      }
   }

private:

   ptr<SyncReadStream> _adapted;
};

/**
 * An output stream which adapts a Boost ASIO SyncWriteStream object to
 * conform OutputStream concept.
 */
template <typename SyncWriteStream>
class sync_write_stream_adapter
{
public:

   inline sync_write_stream_adapter(const ptr<SyncWriteStream>& adapted)
      : _adapted(adapted)
   {}

   inline std::size_t write(const void* src, size_t count)
   {
      try { return _adapted->write_some(boost::asio::buffer(src, count)); }
      catch (boost::system::system_error& e)
      {
         if (e.code() == boost::asio::error::eof)
            return 0;
         else
            BOOST_THROW_EXCEPTION(
                     stream::write_error() <<
                     boost_system_error_info(e) <<
                     message_info(e.what()));
      }
   }

   inline void flush()
   { /* Boost doesn't require flushing the bytes. */ }

private:

   ptr<SyncWriteStream> _adapted;
};

namespace stream {

/**
 * Create a pointer to a new adapter compliant with InputStream concept from a
 * Boost SyncReadStream object.
 */
template <typename SyncReadStream>
inline ptr<sync_read_stream_adapter<SyncReadStream>> make_input_adapter(
      const ptr<SyncReadStream>& s)
{ return new sync_read_stream_adapter<SyncReadStream>(s); }

/**
 * Create a pointer to a new adapter compliant with OutputStream concept from a
 * Boost SyncWriteStream object.
 */
template <typename SyncWriteStream>
inline ptr<sync_write_stream_adapter<SyncWriteStream>> make_output_adapter(
  const ptr<SyncWriteStream>& s)
{ return new sync_write_stream_adapter<SyncWriteStream>(s); }

} // namespace stream

} // namespace oac

#endif
