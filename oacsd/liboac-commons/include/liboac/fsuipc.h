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
class FSUIPC : public Buffer
{
public:

   typedef DWORD Offset;

   class Factory 
   {
   public:

      virtual FSUIPC* createFSUIPC() throw (IllegalStateException) = 0;
   };

   virtual DWORD capacity() const
   { return 0xffff; }

   virtual void read(void* dst, DWORD offset, DWORD length) const 
         throw (OutOfBoundsException, IOException);

   virtual void write(const void* src, DWORD offset, DWORD length) 
         throw (OutOfBoundsException, IOException);

   virtual void copy(
         const Buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length) throw (OutOfBoundsException, IOException);

protected:

   inline FSUIPC() {}
};

class LocalFSUIPC : public FSUIPC
{
public:

   class Factory : public FSUIPC::Factory
   {
   public:
      
      virtual LocalFSUIPC* createFSUIPC() throw (IllegalStateException)
      { return new LocalFSUIPC(); }
   };

   LocalFSUIPC() throw (IllegalStateException);

   virtual ~LocalFSUIPC();

   virtual void read(void* dst, DWORD offset, DWORD length) const
         throw (OutOfBoundsException, IOException);

   virtual void write(const void* src, DWORD offset, DWORD length)
         throw (OutOfBoundsException, IOException);

   virtual void copy(
         const Buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length) throw (OutOfBoundsException, IOException);
};

}; // namespace oac

#endif