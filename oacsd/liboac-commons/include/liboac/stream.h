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

/**
 * Pure abstract class for a stream of data which may be read from.
 *
 */
class InputStream
{
public:

   DECL_ERROR(ReadError, IOError);

   DECL_ERROR(EndOfFileError, ReadError);

   virtual ~InputStream() {}

   /**
    * Read count bytes from the stream and store them in given buffer.
    * This is a synchronous operation. If there are not enough bytes in
    * the stream to satisfy the count, the caller may be blocked until
    * remaining bytes are available, or the stream is closed (which means
    * that return value is less than count).
    *
    * @param buffer the buffer where store the read elements. It must
    *               have at least count allocated bytes
    * @param count the count of bytes to read
    * @return the number of bytes actually read
    */
   virtual DWORD read(void* buffer, DWORD count) throw (ReadError) = 0;

   /**
    * Read one element of type T from the stream and return it. If the
    * stream is closed andthere are no enough bytes left to read an element
    * of type T, a EndOfFileError is thrown.
    */
   template <typename T>
   inline T readAs() throw (ReadError)
   {
      T r;
      if (read(&r, sizeof(T)) < sizeof(T))
         THROW_ERROR(EndOfFileError());
      return r;
   }
};

/**
 * Pure abstract class for a stream of data which may be written to.
 */
class OutputStream
{
public:

   DECL_ERROR(WriteError, IOError);

   virtual ~OutputStream() {}

   /**
    * Write count bytes obtained from given buffer into the stream. This
    * call may be blocking depending on the concrete stream implementation.
    *
    * @param buffer the buffer which contains the elements to be written
    * @param count the number of bytes from buffer to write
    */
   virtual void write(const void* buffer, DWORD count) throw (WriteError) = 0;

   template <typename T>
   inline void writeAs(const T& t) throw (WriteError)
   {
      write(&t, sizeof(T));
   }
};

} // namespace oac

#endif
