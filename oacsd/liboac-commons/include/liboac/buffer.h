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

namespace oac {

/**
 * Buffer class. This class provides an abstraction for a data buffer. 
 */
class Buffer
{
public:

   DECL_ERROR(OutOfBoundsError, InvalidInputError);
   DECL_ERROR(ReadError, IOError);
   DECL_ERROR(WriteError, IOError);

   class Factory
   {
   public:

      virtual ~Factory() {}
      virtual Buffer* createBuffer() const = 0;
   };

   virtual ~Buffer() {}

   virtual DWORD capacity() const = 0;

   virtual void read(void* dst, DWORD offset, DWORD length) const 
         throw (OutOfBoundsError, ReadError) = 0;

   virtual void write(const void* src, DWORD offset, DWORD length) 
         throw (OutOfBoundsError, WriteError) = 0;

   virtual void copy(
         const Buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length) throw (OutOfBoundsError, IOError) = 0;

   template <typename T>
   inline T readAs(DWORD offset) const
   throw (OutOfBoundsError, ReadError)
   {
      T result;
      this->read(&result, offset, sizeof(T));
      return result;
   }

   template <typename T>
   inline void writeAs(DWORD offset, const T& t)
   throw (OutOfBoundsError, WriteError)
   { this->write(&t, offset, sizeof(T)); }
};

/**
 * Buffer of fixed capacity. This is an actual implementation of buffer of
 * fixed capacity set at object construction. 
 */
class FixedBuffer : public Buffer
{
public:

   class Factory : public Buffer::Factory
   {
   public:

      inline Factory(size_t capacity) : _capacity(capacity) {}
      inline virtual FixedBuffer* createBuffer() const
      { return new FixedBuffer(_capacity); }

   private:
      size_t _capacity;
   };

   FixedBuffer(size_t capacity);

   virtual ~FixedBuffer();

   inline virtual DWORD capacity() const
   { return _capacity; }

   virtual void read(void* dst, DWORD offset, DWORD length) const
         throw (OutOfBoundsError);

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (OutOfBoundsError);

   virtual void copy(const Buffer& src, DWORD src_offset,
         DWORD dst_offset, DWORD length) throw (OutOfBoundsError);

private:

   BYTE *_data;
   size_t _capacity;

   void checkBounds(DWORD offset, DWORD length) const throw (OutOfBoundsError);
};

/**
 * Shifted buffer class. This class provides a decorator that shifts
 * the offset on read and write operations. The decorated buffer and
 * the shift are set in the constructor. After that, offsets in read 
 * or write are shifted. E.g., for shift 0x5600, a read/write on offset
 * 0x5678 would be made on decorated buffer on 0x0078. 
 */
class ShiftedBuffer : public Buffer
{
public:

   class Factory : public Buffer::Factory
   {
   public:

      inline Factory(const Ptr<Buffer::Factory>& backed_buffer_fact,
            DWORD shift) : 
         _backed_buffer_fact(backed_buffer_fact),
         _shift(shift)
      {}
      
      inline virtual ShiftedBuffer* createBuffer() const
      { 
         return new ShiftedBuffer(_backed_buffer_fact->createBuffer(), _shift);
      }
      
   private:

      Ptr<Buffer::Factory> _backed_buffer_fact;
      DWORD _shift;
   };

   ShiftedBuffer(const Ptr<Buffer>& backed_buffer, DWORD offset_shift);

   virtual DWORD capacity() const
   { return _backed_buffer->capacity(); }

   virtual void read(void* dst, DWORD offset, DWORD length) const
         throw (OutOfBoundsError, ReadError);

   virtual void write(const void* src, DWORD offset, DWORD length) 
         throw (OutOfBoundsError, WriteError);

   virtual void copy(const Buffer& src, DWORD src_offset, 
         DWORD dst_offset, DWORD length) throw (OutOfBoundsError, IOError);

private:

   Ptr<Buffer> _backed_buffer;
   DWORD _backed_capacity;
   DWORD _offset_shift;

   void checkBounds(DWORD offset, DWORD length) const throw (OutOfBoundsError);

   inline DWORD shift(DWORD offset) const
   { return offset - _offset_shift; }
};

/**
 * Double buffer class. This class decorates a couple of buffers, 
 * making it possible to detect modifications among them. 
 */
class DoubleBuffer : public Buffer
{
public:

   class Factory : public Buffer::Factory
   {
   public:

      inline Factory(const Ptr<Buffer::Factory>& backed_buffer_fact) : 
         _backed_buffer_fact(backed_buffer_fact)
      {}
      
      inline virtual DoubleBuffer* createBuffer() const
      { 
         return new DoubleBuffer(
               _backed_buffer_fact->createBuffer(), 
               _backed_buffer_fact->createBuffer());
      }
      
   private:

      Ptr<Buffer::Factory> _backed_buffer_fact;
      DWORD _shift;
   };

   DoubleBuffer(const Ptr<Buffer>& backed_buffer_0, 
         const Ptr<Buffer>& backed_buffer_1);

   virtual DWORD capacity() const
   { return _backed_buffer[_current_buffer]->capacity(); }

   virtual void read(void* dst, DWORD offset, DWORD length) const 
         throw (OutOfBoundsError, ReadError);

   virtual void write(const void* src, DWORD offset, DWORD length) 
         throw (OutOfBoundsError, WriteError);

   virtual void copy(const Buffer& src, DWORD src_offset, 
         DWORD dst_offset, DWORD length) throw (OutOfBoundsError);

   void swap();

   template <DWORD length>
   inline bool isModified(DWORD offset)
   {
      BYTE data0[length];
      BYTE data1[length];
      _backed_buffer[0]->read(&data0, offset, length);
      _backed_buffer[1]->read(&data1, offset, length);
      return memcmp(data0, data1, length) != 0;
   }

   template <typename T>
   inline bool isModifiedAs(DWORD offset)
   { return this->isModified<sizeof(T)>(offset); }

private:

   Ptr<Buffer> _backed_buffer[2];
   BYTE _current_buffer;
};

}; // namespace oac

#endif
