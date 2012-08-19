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

#ifndef OACSP_H
#define OACSP_H

#ifndef ARDUINO
#include <stdint.h>

namespace oac {

typedef uint8_t byte;
typedef uint16_t word;   
#endif

struct PingCommand
{
   byte type;
   byte deviceType;
   byte majorVersion;
   byte minorVersion;
   byte padding[12];
};

struct ResetCommand
{
   byte type;
   byte padding[15];
};

struct WriteVarCommand
{
   byte type;
   word offset;
   word data;
   byte padding[11];
};

struct NotifyEventCommand
{
   byte type;
   word event;
   word param1;
   word param2;
   word param3;
   word param4;
   byte padding[5];
};

union Command
{
   byte                 type;
   PingCommand          ping;
   ResetCommand         reset;
   WriteVarCommand      writeVar;
   NotifyEventCommand   notifyEvent;
};

enum CommandType
{
   CMD_PING          = 0x01,
   CMD_PONG          = 0x02,
   CMD_RESET         = 0x03,
   CMD_WRITE_VAR     = 0x04,
   CMD_NOTIFY_EVENT  = 0x05,
};

enum DeviceType
{
   DEV_OAC_SERVER    = 0x01,
   DEV_FCU           = 0x10,
};

enum VarOffset
{
   VAR_FCU_STATUS    = 0x0100,
   VAR_FCU_SEL_SPD   = 0x0102,
   VAR_FCU_SEL_HDG   = 0x0104,
   VAR_FCU_TGT_ALT   = 0x0106,
   VAR_FCU_SEL_VS    = 0x0108,
};

enum EventType
{
   EVT_FCU_FD_BTN             = 0x0100,
   EVT_FCU_ILS_BTN            = 0x0101,
   EVT_FCU_LOC_BTN            = 0x0102,
   EVT_FCU_AP1_BTN            = 0x0103,
   EVT_FCU_AP2_BTN            = 0x0104,
   EVT_FCU_ATHR_BTN           = 0x0105,
   EVT_FCU_EXP_BTN            = 0x0106,
   EVT_FCU_APPR_BTN           = 0x0107,
   EVT_FCU_SEL_SPD_BTN        = 0x0108,
   EVT_FCU_SEL_SPD_ROT        = 0x0109,
   EVT_FCU_SEL_HDG_BTN        = 0x010a,
   EVT_FCU_SEL_HDG_ROT        = 0x010b,
   EVT_FCU_ALT_BTN            = 0x010c,
   EVT_FCU_ALT_ROT            = 0x010d,
   EVT_FCU_SEL_VS_BTN         = 0x010e,
   EVT_FCU_SEL_VS_ROT         = 0x010f,
   EVT_FCU_SPD_DISP_BTN       = 0x0110,
   EVT_FCU_LAT_VER_DISP_BTN   = 0x0111,
};

enum FCUStatusMask
{
   MASK_FCU_FD             = 0x0001,
   MASK_FCU_AP1            = 0x0002,
   MASK_FCU_AP2            = 0x0004,
   MASK_FCU_LOC            = 0x0008,
   MASK_FCU_ILS            = 0x0010,
   MASK_FCU_ATHR           = 0x0020,
   MASK_FCU_EXP            = 0x0040,
   MASK_FCU_APPR           = 0x0080,
   MASK_FCU_SPD_DISP       = 0x0100,
   MASK_FCU_LAT_VER_DISP   = 0x0200,
   MASK_FCU_SPD_MOD        = 0x0400,
   MASK_FCU_HDG_MOD        = 0x0800,
   MASK_FCU_VS_MOD         = 0x1000,
};

#ifndef ARDUINO
}; // namespace oac
#endif


#endif
