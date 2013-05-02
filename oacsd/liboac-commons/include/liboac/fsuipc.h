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
class fsuipc : public buffer
{
public:

   OAC_DECL_ERROR(init_error, connection_error);
   OAC_DECL_ERROR_INFO(error_code_info, DWORD);
   OAC_DECL_ERROR_INFO(error_msg_info, std::string);

   typedef DWORD Offset;

   class factory 
   {
   public:

      virtual fsuipc* create_fsuipc() throw (init_error) = 0;
   };

   virtual DWORD capacity() const
   { return 0xffff; }

   virtual void read(void* dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error);

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, write_error);

   virtual void read(output_stream& dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error);

   virtual DWORD write(input_stream& src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, write_error);

   virtual void copy(
         const buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length) throw (out_of_bounds_error, io_error);

protected:

   inline fsuipc() {}
};

class local_fsuipc : public fsuipc
{
public:

   class factory : public fsuipc::factory
   {
   public:
      
      virtual local_fsuipc* create_fsuipc() throw (init_error)
      { return new local_fsuipc(); }
   };

   local_fsuipc() throw (illegal_state_error);

   virtual ~local_fsuipc();

   virtual void read(void* dst, DWORD offset, DWORD length) const
         throw (out_of_bounds_error, read_error);

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (out_of_bounds_error, write_error);

   virtual void copy(
         const buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length) throw (out_of_bounds_error, io_error);

};

} // namespace oac

#endif
