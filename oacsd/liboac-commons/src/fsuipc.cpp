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

#include <sstream>

#include <Boost/format.hpp>

#include "FSUIPC.h"
#include "logging.h"

#ifndef LOCAL_FSUIPC_BUFFER_SIZE
#define LOCAL_FSUIPC_BUFFER_SIZE (65*1024) // 1KB more for headers
#endif

namespace oac {

namespace {

const char* fsuipc_error_msg[] =
{
   "no error",
   "attempt to Open when already Open",
   "cannot link to FSUIPC or WideClient",
   "failed to register common message with Windows",
   "failed to create Atom for mapping filename",
   "failed to create a file mapping object",
   "failed to open a view to the file map",
   "incorrect version of FSUIPC, or not FSUIPC",
   "sim is not version requested",
   "call cannot execute, link not Open",
   "call cannot execute, no requests accumulated",
   "IPC timed out all retries",
   "IPC sendmessage failed all retries",
   "IPC request contains bad data",
   "maybe running on WideClient, but FS not running "
      "on Server, or wrong FSUIPC",
   "read or write request cannot be added, memory for Process is full",
};

const char*
GetResultMessage(DWORD result)
{
   return (result < sizeof(fsuipc_error_msg) / sizeof(const char*))
         ? fsuipc_error_msg[result] : nullptr;
}

std::string
IOErrorMessage(const std::string& action, DWORD result)
{
   return str(boost::format("IO error while %s: %s") %
      action.c_str() % GetResultMessage(result));
}

class LocalFSUIPCHandler
{
public:

   inline static void Init()
   throw (fsuipc::init_error)
   {
      if (!_singleton)
         _singleton = new LocalFSUIPCHandler();
   }

   inline static void Reset()
   {
      _singleton.reset();
   }

   inline ~LocalFSUIPCHandler()
   { FSUIPC_Close(); }

private:

   static ptr<LocalFSUIPCHandler> _singleton;
   BYTE _buffer[LOCAL_FSUIPC_BUFFER_SIZE];

   inline LocalFSUIPCHandler()
   throw (fsuipc::init_error)
   {
      DWORD result;
      if (!FSUIPC_Open2(SIM_ANY, &result, _buffer, LOCAL_FSUIPC_BUFFER_SIZE))
         BOOST_THROW_EXCEPTION(
                  fsuipc::init_error() <<
                  message_info(GetResultMessage(result)));
   }

};

ptr<LocalFSUIPCHandler> LocalFSUIPCHandler::_singleton;

} // anonymous namespace

void
fsuipc::read(void* dst, DWORD offset, DWORD length) const 
throw (out_of_bounds_error, read_error)
{
   DWORD error;
   if (FSUIPC_Read(offset, length, dst, &error))
      if (FSUIPC_Process(&error))
         return;
   BOOST_THROW_EXCEPTION(read_error() <<
         error_code_info(error) <<
         error_msg_info(GetResultMessage(error)));
}

void
fsuipc::read(output_stream& dst, DWORD offset, DWORD length) const
throw (out_of_bounds_error, read_error)
{
   fixed_buffer tmp(length);
   tmp.copy(*this, offset, 0, length);
   tmp.read(dst, 0, length);
}

void
fsuipc::write(const void* src, DWORD offset, DWORD length) 
throw (out_of_bounds_error, write_error)
{
   DWORD error;
   if (FSUIPC_Write(offset, length, (void*) src, &error))
      if (FSUIPC_Process(&error))
         return;
   BOOST_THROW_EXCEPTION(write_error() <<
         error_code_info(error) <<
         error_msg_info(GetResultMessage(error)));
}

DWORD
fsuipc::write(input_stream& src, DWORD offset, DWORD length)
throw (out_of_bounds_error, write_error)
{
   fixed_buffer tmp(length);
   auto nbytes = tmp.write(src, 0, length);
   this->copy(tmp, 0, offset, length);
   return nbytes;
}

void
fsuipc::copy(const buffer& src, DWORD src_offset, 
      DWORD dst_offset, DWORD length) 
throw (out_of_bounds_error, io_error)
{
   BYTE tmp[1024];
   auto sos = src_offset;
   auto dos = dst_offset;
   while (length)
   {
      auto l = length > 1024 ? 1024 : length;
      src.read(tmp, sos, l);
      this->write(tmp, dos, l);
      dos += l;
      sos += l;
      length -= l;
   }
}

local_fsuipc::local_fsuipc()
throw (illegal_state_error)
{ LocalFSUIPCHandler::Init(); }

local_fsuipc::~local_fsuipc()
{}

void
local_fsuipc::read(void* dst, DWORD offset, DWORD length) const
throw (out_of_bounds_error, read_error)
{
   try
   {
      fsuipc::read(dst, offset, length);
   } catch (std::exception&)
   {
      LocalFSUIPCHandler::Reset();
      throw;
   }
}

void
local_fsuipc::write(const void* src, DWORD offset, DWORD length)
throw (out_of_bounds_error, write_error)
{
   try
   {
      fsuipc::write(src, offset, length);
   } catch (std::exception&)
   {
      LocalFSUIPCHandler::Reset();
      throw;
   }
}

void
local_fsuipc::copy(const buffer& src, DWORD src_offset,
                  DWORD dst_offset, DWORD length)
throw (out_of_bounds_error, io_error)
{
   try
   {
      fsuipc::copy(src, src_offset, dst_offset, length);
   } catch (io_error&)
   {
      LocalFSUIPCHandler::Reset();
      throw;
   }
}


} // namespace oac
