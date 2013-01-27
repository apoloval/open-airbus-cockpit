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

#include <liboac/logging.h>

#include "wilco-internal.h"

namespace oac { namespace we {

namespace {

const DllInfo DLL_INFO[] =
{
   { TEXT("A320CFM_FeelThere.DLL"),
      {
         VirtualAddress(0x10000000), // Image base
         VirtualAddress(0x10008BD0), // Wilco_PushMCPButton (Constraint)
         VirtualAddress(0x10008C30), // Wilco_PushMCPButton (Waypoint)
         VirtualAddress(0x10008C90), // Wilco_PushMCPButton (VORD)
         VirtualAddress(0x10008CF0), // Wilco_PushMCPButton (NDB)
         VirtualAddress(0x10008D50), // Wilco_PushMCPButton (Airport)
         VirtualAddress(0x1001D560), // Wilco_IsApuAvailable()
         VirtualAddress(0x1001F170), // Wilco_GetFADECMode()
         VirtualAddress(0x1002C920), // Wilco_SendCommand()
         VirtualAddress(0x100D4A78), // ND Range
         VirtualAddress(0x100D4A7C), // ND Mode
         VirtualAddress(0x100D4A80), // MCP Nav Left
         VirtualAddress(0x100D4A84), // MCP Nav Right
         VirtualAddress(0x1012A940), // FADEC data
         VirtualAddress(0x1012A95C), // Baro format
         VirtualAddress(0x1012A960), // Baro STD
         VirtualAddress(0x1012A964), // FCU Speed display
         VirtualAddress(0x1012A9AC), // MCP Constraint
         VirtualAddress(0x1012A9B0), // MCP Waypoint
         VirtualAddress(0x1012A9B4), // MCP VORD
         VirtualAddress(0x1012A9B8), // MCP NDB
         VirtualAddress(0x1012A9BC), // MCP Airport
         VirtualAddress(0x1012A9C0), // ILS Switch
         VirtualAddress(0x1012AA08), // FMGC data
         VirtualAddress(0x1012AA10), // FCU data
         VirtualAddress(0x1012AA14), // FBW data
         VirtualAddress(0x1012AA18), // Head panel data
         VirtualAddress(0x1012AA1C), // GPWS data
         VirtualAddress(0x1012AA20), // Pedestal data
         VirtualAddress(0x1012AA28), // APU data
         VirtualAddress(0x1012AA38), // GPU data
      },
   },
};

} // anonymous namespace

FunctionName FunctionNames::GET_INTERNAL_DATA = "GetInternalData";
FunctionName FunctionNames::GET_EXTENDED_DATA = "GetExtendedData";
FunctionName FunctionNames::RESET_INTERNAL_DATA = "ResetInternalData";
FunctionName FunctionNames::RESET_FLIGHT = "ResetFlight";

const DllInfo&
DllInfo::forAircraft(AircraftType aircraft)
{
   return DLL_INFO[aircraft];
}

HINSTANCE
LoadDLLForAircraft(AircraftType aircraft)
throw (InvalidInputException)
{
   auto dll_filename = DLL_INFO[aircraft].name.c_str();
   HINSTANCE lib = LoadLibrary(dll_filename);
   if (lib == NULL)
      LogAndThrow(FAIL, InvalidInputException(str(boost::format(
            "cannot load Wilco Airbus DLL file %s") % dll_filename)));
   return lib;
}

void
FreeDLL(HINSTANCE lib)
{
   if (!FreeLibrary(lib))
      Log(WARN, "cannot free DLL instance");
}

void
TrackChangesOnMemory(void* mem, size_t len) {
   static void* buf = nullptr;
   if (buf)
   {
      auto src = (DWORD*) mem;
      auto dst = (DWORD*) buf;
      for (unsigned int i = 0; i < len / size_t(4); i++)
      {
         if (src[i] != dst[i]) {
            Log(INFO, str(boost::format(
                  "word %d (+0x%X) changes from %d to %d")
                  % i % i % dst[i] % src[i]));
         }
      }
   }
   else
   {
      Log(INFO, str(boost::format("Tracking %d bytes on 0x%X")
            % len % mem));
      buf = new uint8_t[len];
   }
   memcpy(buf, mem, len);
}

}} // namespace oac::we

