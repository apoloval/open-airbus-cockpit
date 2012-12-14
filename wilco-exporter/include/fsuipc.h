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

#ifndef OAC_WE_FSUIPC_H
#define OAC_WE_FSUIPC_H

#pragma warning( disable : 4290 )

#include <Windows.h>

#include <Boost/format.hpp>
#include <FSUIPC_User.h>

#include "exception.h"

#define FSUIPC_DEFAULT_BUFFER_SIZE 1024

namespace oac {

const char* GetMessageForFSUIPCResult(DWORD result);

class FSUIPC {
public:

   typedef DWORD Offset;

   DECL_RUNTIME_ERROR(IOException);
   DECL_RUNTIME_ERROR(StateException);

   template <typename T>
   T read(Offset offset) const throw (IOException)
   {
      DWORD result;
      T t;
      if (FSUIPC_Read(offset, sizeof(T), &t, &result))
         if (FSUIPC_Process(&result))
            return t;      
      throw IOException(IOErrorMessage("reading data", result));
   }

   template <typename T>
   void readLazy(Offset offset, T& t) const throw (IOException)
   {
      DWORD result;
      if (!FSUIPC_Read(offset, sizeof(T), &t, &result))
         throw IOException(IOErrorMessage("reading data lazily", result));
   }

   template <typename T>
   void write(Offset offset, const T& t) throw (IOException)
   {
      DWORD result;
      if (FSUIPC_Write(offset, sizeof(T), (void*) &t, &result))
         if (FSUIPC_Process(&result))
            return;
      throw IOException(IOErrorMessage("writing data", result));
   }

   template <typename T>
   void writeLazy(Offset offset, const T& t) throw (IOException)
   {
      DWORD result;
      if (!FSUIPC_Write(offset, sizeof(T), (void*) &t, &result))
         throw IOException(IOErrorMessage("writing data lazily", result));
   }

   inline void flush(void) throw (IOException)
   {
      DWORD result;
      if (!FSUIPC_Process(&result))
         throw IOException(IOErrorMessage("flushing data", result));
   }

protected:

   inline FSUIPC() {}

private:

   std::string IOErrorMessage(const std::string& action, DWORD result)
   {
      return str(boost::format("IO error while %s: %s") % 
         action.c_str() % GetMessageForFSUIPCResult(result));
   }

};

template <size_t buffer_size = FSUIPC_DEFAULT_BUFFER_SIZE>
class LocalFSUIPC : public FSUIPC {
public:

   inline LocalFSUIPC() throw (StateException)
   {
      DWORD result;
      if (!FSUIPC_Open2(SIM_ANY, &result, _buffer, buffer_size))
         throw StateException(str(boost::format(
            "cannot open FSUIPC: %s") % GetMessageForFSUIPCResult(result)));      
   }

   inline ~LocalFSUIPC()
   { FSUIPC_Close(); }

   BYTE _buffer[buffer_size];
};

}; // namespace oac

#endif
