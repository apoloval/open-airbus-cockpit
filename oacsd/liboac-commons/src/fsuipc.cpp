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

class local_fsuipc_handler
{
public:

   inline static void init()
   throw (local_fsuipc::init_error)
   {
      if (!_singleton)
         _singleton = new local_fsuipc_handler();
   }

   inline static void reset()
   {
      _singleton.reset();
   }

   inline ~local_fsuipc_handler()
   { FSUIPC_Close(); }

private:

   static ptr<local_fsuipc_handler> _singleton;
   BYTE _buffer[LOCAL_FSUIPC_BUFFER_SIZE];

   inline local_fsuipc_handler()
   throw (local_fsuipc::init_error)
   {
      DWORD result;
      if (!FSUIPC_Open2(SIM_ANY, &result, _buffer, LOCAL_FSUIPC_BUFFER_SIZE))
         BOOST_THROW_EXCEPTION(
                  local_fsuipc::init_error() <<
                  message_info(GetResultMessage(result)));
   }

};

ptr<local_fsuipc_handler> local_fsuipc_handler::_singleton;

} // anonymous namespace

local_fsuipc::local_fsuipc()
   : linear_stream_buffer_base<local_fsuipc>(this)
{
   local_fsuipc_handler::init();
}

void
local_fsuipc::read(void* dst, std::uint32_t offset, std::size_t length) const
throw (buffer::out_of_bounds_error, buffer::read_error)
{
   DWORD error;
   if (FSUIPC_Read(offset, length, dst, &error))
      if (FSUIPC_Process(&error))
         return;
   BOOST_THROW_EXCEPTION(buffer::read_error() <<
         error_code_info(error) <<
         error_msg_info(GetResultMessage(error)));
}

void
local_fsuipc::write(const void* src, std::uint32_t offset, std::size_t length)
throw (buffer::out_of_bounds_error, buffer::write_error)
{
   DWORD error;
   if (FSUIPC_Write(offset, length, (void*) src, &error))
      if (FSUIPC_Process(&error))
         return;
   BOOST_THROW_EXCEPTION(buffer::write_error() <<
         error_code_info(error) <<
         error_msg_info(GetResultMessage(error)));
}

} // namespace oac
