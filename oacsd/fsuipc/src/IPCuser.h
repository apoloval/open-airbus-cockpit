/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2002-2012 Peter Dowson, Pelle Liljendal and Chris Brett
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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#include <windows.h>

#include "FSUIPC_User.h"

#define FS6IPC_MSGNAME1      "FsasmLib:IPC" 

#define FS6IPC_MESSAGE_SUCCESS 1
#define FS6IPC_MESSAGE_FAILURE 0

// IPC message types
#define FS6IPC_READSTATEDATA_ID    1
#define FS6IPC_WRITESTATEDATA_ID   2

// read request structure
typedef struct tagFS6IPC_READSTATEDATA_HDR
{
  DWORD dwId;       // FS6IPC_READSTATEDATA_ID
  DWORD dwOffset;   // state table offset
  DWORD nBytes;     // number of bytes of state data to read
  void* pDest;      // destination buffer for data (client use only)
} FS6IPC_READSTATEDATA_HDR;

// write request structure
typedef struct tagFS6IPC_WRITESTATEDATA_HDR
{
  DWORD dwId;       // FS6IPC_WRITESTATEDATA_ID
  DWORD dwOffset;   // state table offset
  DWORD nBytes;     // number of bytes of state data to write
} FS6IPC_WRITESTATEDATA_HDR;
