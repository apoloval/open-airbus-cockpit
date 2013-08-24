/*
* This file is part of Open Airbus Cockpit
* Copyright (C) 2012, 2013 Alvaro Polo
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

const dll_info DLL_INFO[] =
{
   { TEXT("A320CFM_FeelThere.DLL"),
      {
         virtual_address(0x10000000), // Image base
         virtual_address(0x10008BD0), // wilco_push_mcp_button (Constraint)
         virtual_address(0x10008C30), // wilco_push_mcp_button (Waypoint)
         virtual_address(0x10008C90), // wilco_push_mcp_button (VORD)
         virtual_address(0x10008CF0), // wilco_push_mcp_button (NDB)
         virtual_address(0x10008D50), // wilco_push_mcp_button (Airport)
         virtual_address(0x1001D560), // wilco_is_apu_available()
         virtual_address(0x1001F170), // Wilco_GetFADECMode()
         virtual_address(0x1002C920), // wilco_send_command()
         virtual_address(0x100D4A78), // ND Range
         virtual_address(0x100D4A7C), // ND Mode
         virtual_address(0x100D4A80), // MCP Nav Left
         virtual_address(0x100D4A84), // MCP Nav Right
         virtual_address(0x1012A940), // FADEC data
         virtual_address(0x1012A95C), // Baro format
         virtual_address(0x1012A960), // Baro STD
         virtual_address(0x1012A964), // FCU Speed display
         virtual_address(0x1012A9AC), // MCP Constraint
         virtual_address(0x1012A9B0), // MCP Waypoint
         virtual_address(0x1012A9B4), // MCP VORD
         virtual_address(0x1012A9B8), // MCP NDB
         virtual_address(0x1012A9BC), // MCP Airport
         virtual_address(0x1012A9C0), // ILS Switch
         virtual_address(0x1012AA08), // FMGC data
         virtual_address(0x1012AA10), // FCU data
         virtual_address(0x1012AA14), // FBW data
         virtual_address(0x1012AA18), // Head panel data
         virtual_address(0x1012AA1C), // GPWS data
         virtual_address(0x1012AA20), // Pedestal data
         virtual_address(0x1012AA28), // APU data
         virtual_address(0x1012AA38), // GPU data
      },
   },
};

} // anonymous namespace

function_name function_names::GET_INTERNAL_DATA = "GetInternalData";
function_name function_names::GET_EXTENDED_DATA = "GetExtendedData";
function_name function_names::RESET_INTERNAL_DATA = "ResetInternalData";
function_name function_names::RESET_FLIGHT = "ResetFlight";

const dll_info&
dll_info::for_aircraft(const aircraft& aircraft)
{
   return DLL_INFO[aircraft.type];
}

HINSTANCE
load_dll_for_aircraft(const aircraft& aircraft)
throw (dll_load_error)
{
   auto dll = dll_info::for_aircraft(aircraft);
   auto dll_filename = dll.name.c_str();
   HINSTANCE lib = LoadLibrary(dll_filename);
   if (lib == NULL)
      OAC_THROW_EXCEPTION(dll_load_error()
            .with_dll(dll.name));
   return lib;
}

void
free_dll(HINSTANCE lib)
{
   if (!FreeLibrary(lib))
      log("WilcoInternal-free_dll", WARN, "cannot free DLL instance");
}

void
track_changes_on_memory(void* mem, size_t len) {
   static void* buf = nullptr;
   if (buf)
   {
      auto src = (DWORD*) mem;
      auto dst = (DWORD*) buf;
      for (unsigned int i = 0; i < len / size_t(4); i++)
      {
         if (src[i] != dst[i]) {
            log(
                  "WilcoInternal-track_changes_on_memory",
                  INFO,
                  str(boost::format(
                        "word %d (+0x%X) changes from %d to %d")
                        % i % i % dst[i] % src[i]));
         }
      }
   }
   else
   {
      log(
            "WilcoInternal-track_changes_on_memory",
            INFO,
            str(boost::format("Tracking %d bytes on 0x%X") % len % mem));
      buf = new uint8_t[len];
   }
   memcpy(buf, mem, len);
}

}} // namespace oac::we

