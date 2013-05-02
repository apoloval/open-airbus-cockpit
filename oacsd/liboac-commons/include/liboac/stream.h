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

#include <Windows.h>

#include "exception.h"
#include "lang-utils.h"

namespace oac {

class buffer;

/**
 * Pure abstract class for a stream of data which may be read from.
 *
 */
class input_stream
{
public:

   OAC_DECL_ERROR(read_error, io_error);

   OAC_DECL_ERROR(eof_error, read_error);

   virtual ~input_stream() {}

   /**
    * Read count bytes from the stream and store them in given buffer.
    * This is a synchronous operation. If there are still no bytes in
    * the stream, the caller may be blocked until new data arrive. Then,
    * it's possible that there wasn't enough bytes to satisfy the requested
    * count, so the return value of the function would be less than count.
    * When stream closes, it shall return 0.
    *
    * @param buffer the buffer where store the read elements. It must
    *               have at least count allocated bytes
    * @param count the count of bytes to read
    * @return the number of bytes actually read (might be less than count, or
    *         0 when the stream is closed)
    */
   virtual DWORD read(void* buffer, DWORD count) throw (read_error) = 0;

   /**
    * Read one element of type T from the stream and return it. If the
    * stream is closed andthere are no enough bytes left to read an element
    * of type T, a eof_error is thrown.
    */
   template <typename T>
   inline T read_as() throw (read_error)
   {
      T r;
      if (read(&r, sizeof(T)) < sizeof(T))
         BOOST_THROW_EXCEPTION(eof_error());
      return r;
   }
};

/**
 * Pure abstract class for a stream of data which may be written to.
 */
class output_stream
{
public:

   OAC_DECL_ERROR(write_error, io_error);

   virtual ~output_stream() {}

   /**
    * Write count bytes obtained from given buffer into the stream. This
    * call may be blocking depending on the concrete stream implementation.
    *
    * @param buffer the buffer which contains the elements to be written
    * @param count the number of bytes from buffer to write
    */
   virtual void write(const void* buffer, DWORD count) throw (write_error) = 0;

   /**
    * Flush the bytes pending to be sent.
    */
   virtual void flush() = 0;

   template <typename T>
   inline void write_as(const T& t) throw (write_error)
   { write(&t, sizeof(T)); }

   template <>
   inline void write_as<std::string>(const std::string& str) throw (write_error)
   { write(str.c_str(), str.length()); }
};

class reader
{
public:

   reader(const ptr<input_stream>& input);

   std::string readLine();

private:

   ptr<input_stream> _input;
};

} // namespace oac

#endif
