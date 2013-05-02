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
#include "stream.h"

namespace oac {

/**
 * Buffer class. This class provides an abstraction for a data buffer. 
 */
class buffer
{
public:

   OAC_DECL_ERROR(out_of_bounds_error, invalid_input_error);
   OAC_DECL_ERROR(read_error, io_error);
   OAC_DECL_ERROR(write_error, io_error);

   class factory
   {
   public:

      virtual ~factory() {}
      virtual buffer* create_buffer() const = 0;
   };

   virtual ~buffer() {}

   virtual DWORD capacity() const = 0;

   virtual void read(void* dst, DWORD offset, DWORD length) const 
         throw (out_of_bounds_error, read_error) = 0;

   virtual void read(output_stream& dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error, output_stream::write_error) = 0;

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, write_error) = 0;

   virtual DWORD write(input_stream& src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, input_stream::read_error, write_error) = 0;

   virtual void copy(
         const buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length) throw (out_of_bounds_error, io_error) = 0;

   template <typename T>
   inline T read_as(DWORD offset) const
   throw (out_of_bounds_error, read_error)
   {
      T result;
      this->read(&result, offset, sizeof(T));
      return result;
   }

   template <typename T>
   inline void write_as(DWORD offset, const T& t)
   throw (out_of_bounds_error, write_error)
   { this->write(&t, offset, sizeof(T)); }
};

/**
 * Buffer of fixed capacity. This is an actual implementation of buffer of
 * fixed capacity set at object construction. 
 */
class fixed_buffer : public buffer
{
public:

   class factory : public buffer::factory
   {
   public:

      inline factory(size_t capacity) : _capacity(capacity) {}
      inline virtual fixed_buffer* create_buffer() const
      { return new fixed_buffer(_capacity); }

   private:
      size_t _capacity;
   };

   fixed_buffer(size_t capacity);

   virtual ~fixed_buffer();

   inline virtual DWORD capacity() const
   { return _capacity; }

   virtual void read(void* dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error);

   virtual void read(output_stream& dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error, output_stream::write_error);

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, write_error);

   virtual DWORD write(input_stream& src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, input_stream::read_error, write_error);

   virtual void copy(const buffer& src, DWORD src_offset,
         DWORD dst_offset, DWORD length) throw (out_of_bounds_error);

private:

   BYTE *_data;
   size_t _capacity;

   void check_bounds(DWORD offset, DWORD length) const throw (out_of_bounds_error);
};

/**
 * Shifted buffer class. This class provides a decorator that shifts
 * the offset on read and write operations. The decorated buffer and
 * the shift are set in the constructor. After that, offsets in read 
 * or write are shifted. E.g., for shift 0x5600, a read/write on offset
 * 0x5678 would be made on decorated buffer on 0x0078. 
 */
class shifted_buffer : public buffer
{
public:

   class factory : public buffer::factory
   {
   public:

      inline factory(const ptr<buffer::factory>& backed_buffer_fact,
            DWORD shift) : 
         _backed_buffer_fact(backed_buffer_fact),
         _shift(shift)
      {}
      
      inline virtual shifted_buffer* create_buffer() const
      { 
         return new shifted_buffer(_backed_buffer_fact->create_buffer(), _shift);
      }
      
   private:

      ptr<buffer::factory> _backed_buffer_fact;
      DWORD _shift;
   };

   shifted_buffer(const ptr<buffer>& backed_buffer, DWORD offset_shift);

   virtual DWORD capacity() const
   { return _backed_buffer->capacity(); }

   virtual void read(void* dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error);

   virtual void read(output_stream& dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error, output_stream::write_error);

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, write_error);

   virtual DWORD write(input_stream& src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, input_stream::read_error, write_error);

   virtual void copy(const buffer& src, DWORD src_offset,
         DWORD dst_offset, DWORD length) throw (out_of_bounds_error, io_error);

private:

   ptr<buffer> _backed_buffer;
   DWORD _backed_capacity;
   DWORD _offset_shift;

   void check_bounds(DWORD offset, DWORD length) const throw (out_of_bounds_error);

   inline DWORD shift(DWORD offset) const
   { return offset - _offset_shift; }
};

/**
 * Double buffer class. This class decorates a couple of buffers, 
 * making it possible to detect modifications among them. 
 */
class double_buffer : public buffer
{
public:

   class factory : public buffer::factory
   {
   public:

      inline factory(const ptr<buffer::factory>& backed_buffer_fact) :
         _backed_buffer_fact(backed_buffer_fact)
      {}
      
      inline virtual double_buffer* create_buffer() const
      { 
         return new double_buffer(
               _backed_buffer_fact->create_buffer(),
               _backed_buffer_fact->create_buffer());
      }
      
   private:

      ptr<buffer::factory> _backed_buffer_fact;
      DWORD _shift;
   };

   double_buffer(const ptr<buffer>& backed_buffer_0,
         const ptr<buffer>& backed_buffer_1);

   virtual DWORD capacity() const
   { return _backed_buffer[_current_buffer]->capacity(); }

   virtual void read(void* dst, DWORD offset, DWORD length) const 
         throw (out_of_bounds_error, read_error);

   virtual void read(output_stream& dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error, output_stream::write_error);

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, write_error);

   virtual DWORD write(input_stream& src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, input_stream::read_error, write_error);

   virtual void copy(const buffer& src, DWORD src_offset,
         DWORD dst_offset, DWORD length) throw (out_of_bounds_error);

   void swap();

   template <DWORD length>
   inline bool is_modified(DWORD offset)
   {
      BYTE data0[length];
      BYTE data1[length];
      _backed_buffer[0]->read(&data0, offset, length);
      _backed_buffer[1]->read(&data1, offset, length);
      return memcmp(data0, data1, length) != 0;
   }

   template <typename T>
   inline bool is_modified_as(DWORD offset)
   { return this->is_modified<sizeof(T)>(offset); }

private:

   ptr<buffer> _backed_buffer[2];
   BYTE _current_buffer;
};

class buffer_input_stream : public input_stream {
public:

   inline buffer_input_stream(
         const ptr<buffer>& buffer) : _buffer(buffer), _index(0) {}

   virtual DWORD read(void* buffer, DWORD count);

   inline ptr<buffer> get_buffer() const { return _buffer; }

private:

   ptr<buffer> _buffer;
   DWORD _index;
};

class buffer_output_stream : public output_stream {
public:

   OAC_DECL_ERROR(capacity_exhausted_error, write_error);

   OAC_DECL_ERROR_INFO(remaining_bytes_info, DWORD);
   OAC_DECL_ERROR_INFO(requested_bytes_info, DWORD);

   inline buffer_output_stream(
         const ptr<buffer>& buffer) : _buffer(buffer), _index(0) {}

   virtual void write(
         const void* buffer, DWORD count) throw (capacity_exhausted_error);

   virtual void flush();

   inline ptr<buffer> get_buffer() const { return _buffer; }

private:

   ptr<buffer> _buffer;
   DWORD _index;

};

}; // namespace oac

#endif
