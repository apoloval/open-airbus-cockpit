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

#ifndef OAC_FSUIPC_H
#define OAC_FSUIPC_H

#pragma warning( disable : 4290 )

#include <Windows.h>

#include <Boost/format.hpp>
#include <FSUIPC_User.h>

#include "buffer.h"
#include "exception.h"

#define FSUIPC_DEFAULT_BUFFER_SIZE 1024

namespace oac {

/**
 * FSUIPC class. This class encapsulates the access to FSUIPC module. It 
 * implements convenient wrappers to read from and write to FSUIPC offsets.
 */
class local_fsuipc :
      public shared_by_ptr<local_fsuipc>,
      public linear_stream_buffer_base<local_fsuipc>
{
public:

   OAC_DECL_ERROR(init_error, connection_error);
   OAC_DECL_ERROR_INFO(error_code_info, DWORD);
   OAC_DECL_ERROR_INFO(error_msg_info, std::string);

   typedef DWORD Offset;

   class factory 
   {
   public:

      typedef local_fsuipc value_type;

      local_fsuipc* create_fsuipc() throw (init_error)
      { return new local_fsuipc(); }
   };

   local_fsuipc();

   virtual DWORD capacity() const
   { return 0xffff; }

   void read(void* dst, std::uint32_t offset, std::size_t length) const
         throw (buffer::out_of_bounds_error, buffer::read_error);

   void write(const void* src, std::uint32_t offset, std::size_t length)
         throw (buffer::out_of_bounds_error, buffer::write_error);

   template <typename OutputStream>
   void read_to(OutputStream& dst,
                std::uint32_t offset,
                std::size_t length) const
   throw (buffer::out_of_bounds_error, buffer::read_error)
   {
      linear_buffer tmp(length);
      tmp.copy(*this, offset, 0, length);
      tmp.read(dst, 0, length);
   }

   template <typename InputStream>
   std::size_t write_from(InputStream& src,
                          std::uint32_t offset,
                          std::size_t length)
   throw (buffer::out_of_bounds_error, buffer::write_error)
   {
      linear_buffer tmp(length);
      auto nbytes = tmp.write(src, 0, length);
      this->copy(tmp, 0, offset, length);
      return nbytes;
   }

   template <typename Buffer>
   void copy(
         const Buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length)
   throw (buffer::out_of_bounds_error, buffer::read_error, buffer::write_error)
   {
      BYTE tmp[1024];
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

} // namespace oac

#endif
