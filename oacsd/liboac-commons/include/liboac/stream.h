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

#include <Windows.h>

#include "exception.h"
#include "lang-utils.h"

namespace oac {

/**
 * @concept InputStream
 *
 * A class which provides a member to read bytes from with the signature:
 *
 * size_t InputStream::read(void* dest, size_t count);
 *
 * This operation is synchronous, so the caller may be blocked if no data
 * is available in the stream. It shall return 0 on any pending and subsequent
 * call if the stream is closed. It might occur that less bytes than requested
 * are read. In such a case, return value shall be less than count. For
 * blocking the caller until all bytes are available, use read_all() function
 * instead.
 */

/**
 * @concept OutputStream
 *
 * A class which provides a member to write bytes to, with the signature:
 *
 * void OutputStream::write(const void* src, size_t count);
 *
 * void flush();
 */

struct stream
{
   OAC_DECL_ERROR(read_error, io_error);
   OAC_DECL_ERROR(eof_error, read_error);

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
   inline static void read_all(InputStream& s, void* dest, std::size_t count)
   throw (read_error)
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

   template <typename T, typename InputStream>
   inline static T read_as(InputStream& s)
   throw (read_error)
   {
      T r;
      read_all(s, &r, sizeof(T));
      return r;
   }

   template <typename InputStream>
   inline static std::string read_as_string(InputStream& s, unsigned int len)
   throw (read_error)
   {
      char* buff = new char[len];
      read_all(s, buff, len);
      std::string r(buff, len);
      delete buff;
      return r;
   }

   template <typename InputStream>
   inline static std::string read_line(InputStream& s)
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

   template <typename OutputStream>
   inline static void write_as_string(
         OutputStream& s, const std::string& str)
   throw (write_error)
   { s.write(str.c_str(), str.length()); }

   template <typename T, typename OutputStream>
   inline static void write_as(OutputStream& s, const T& t)
   throw (write_error)
   { s.write(&t, sizeof(T)); }

};

} // namespace oac

#endif
