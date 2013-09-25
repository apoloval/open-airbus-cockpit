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

#ifndef OAC_FSUIPC_LOCAL_H
#define OAC_FSUIPC_LOCAL_H

#include <cstdint>

#include <liboac/buffer/linear.h>
#include <liboac/fsuipc/errors.h>
#include <liboac/fsuipc/offset.h>
#include <liboac/logging.h>

#ifndef LOCAL_FSUIPC_BUFFER_SIZE
#define LOCAL_FSUIPC_BUFFER_SIZE (65*1024) // 1KB more for headers
#endif

namespace oac { namespace fsuipc {

/**
 * A FsuipcUserAdapter which wraps the local FSUIPC functions from
 * ModuleUser.lib
 */
class local_user_adapter : public logger_component
{
public:

   local_user_adapter() throw (fsuipc_error);

   local_user_adapter(const local_user_adapter& adapter);

   ~local_user_adapter();

   void read(valued_offset& valued_offset)
   throw (fsuipc_error);

   void write(const valued_offset& valued_offset)
   throw (fsuipc_error);

   void process() throw (fsuipc_error);

private:

   static std::uint32_t _instance_count;
   static std::uint8_t _buffer[LOCAL_FSUIPC_BUFFER_SIZE];
};

/**
 * FSUIPC class. This class encapsulates the access to FSUIPC module. It
 * implements convenient wrappers to read from and write to FSUIPC offsets.
 */
class local_fsuipc :
      public buffer::linear_stream_buffer_base<local_fsuipc>
{
public:

   typedef std::shared_ptr<local_fsuipc> ptr_type;

   typedef std::uint32_t Offset;

   class factory
   {
   public:

      typedef local_fsuipc value_type;

      local_fsuipc* create_fsuipc() throw (fsuipc_error)
      { return new local_fsuipc(); }
   };

   typedef factory factory_type;
   typedef std::shared_ptr<factory_type> factory_ptr;

   local_fsuipc();

   virtual std::size_t capacity() const
   { return 0xffff; }

   void read(void* dst, std::uint32_t offset, std::size_t length) const
         throw (buffer::index_out_of_bounds, io_exception);

   void write(const void* src, std::uint32_t offset, std::size_t length)
         throw (buffer::index_out_of_bounds, io_exception);

   template <typename OutputStream>
   void read_to(OutputStream& dst,
                std::uint32_t offset,
                std::size_t length) const
   throw (buffer::index_out_of_bounds, io_exception)
   {
      linear_buffer tmp(length);
      tmp.copy(*this, offset, 0, length);
      tmp.read(dst, 0, length);
   }

   template <typename InputStream>
   std::size_t write_from(InputStream& src,
                          std::uint32_t offset,
                          std::size_t length)
   throw (buffer::index_out_of_bounds, io_exception)
   {
      linear_buffer tmp(length);
      auto nbytes = tmp.write(src, 0, length);
      this->copy(tmp, 0, offset, length);
      return nbytes;
   }

   template <typename Buffer>
   void copy(
         const Buffer& src,
         std::uint32_t src_offset,
         std::uint32_t dst_offset,
         std::size_t length)
   throw (buffer::index_out_of_bounds, io_exception)
   {
      std::uint8_t tmp[1024];
      auto sos = src_offset;
      auto dos = dst_offset;
      while (length)
      {
         auto l = length > 1024 ? 1024 : length;
         src.read(tmp, sos, l);
         write(tmp, dos, l);
         dos += l;
         sos += l;
         length -= l;
      }
   }
};

typedef local_fsuipc::ptr_type local_fsuipc_ptr;

}} // namespace oac::fsuipc

#endif
