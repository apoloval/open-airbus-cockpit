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
}

const char*
GetMessageForFSUIPCResult(DWORD result)
{ 
   return (result < sizeof(fsuipc_error_msg) / sizeof(const char*))
         ? fsuipc_error_msg[result] : nullptr;
}

}; // namespace oac