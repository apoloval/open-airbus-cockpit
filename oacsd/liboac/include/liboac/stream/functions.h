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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_STREAM_FUNCTIONS_H
#define OAC_STREAM_FUNCTIONS_H

#include <cstdint>

#include <liboac/exception.h>
#include <liboac/io.h>

namespace oac { namespace stream {

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
throw (io_exception)
{
   auto p = (std::uint8_t*) dest;
   while (count)
   {
      auto nread = s.read(p, count);
      if (nread == 0)
         OAC_THROW_EXCEPTION(io::eof_error());
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
throw (io_exception)
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
throw (io_exception)
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
   char buff[CHUNK_SIZE];
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
throw (io_exception)
{
   auto p = (const std::uint8_t*) src;
   while (count)
   {
      auto nwrite = s.write(p, count);
      if (nwrite == 0)
         OAC_THROW_EXCEPTION(io::eof_error());
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
throw (io_exception)
{ write_all(s, str.c_str(), str.length()); }

/**
 * Write given object of type T into the OutputStream. If the other end of the
 * stream is closed before all bytes are written, an eof_error is thrown.
 */
template <typename T, typename OutputStream>
inline void write_as(OutputStream& s, const T& t)
throw (io_exception)
{ write_all(s, &t, sizeof(T)); }

}} // namespace oac::stream

#endif
