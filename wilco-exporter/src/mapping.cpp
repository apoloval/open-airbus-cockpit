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

#include "mapping.h"

namespace oac { namespace we {

namespace {

inline void ExportModuleState(FSUIPC& fsuipc)
{
   fsuipc.write<BYTE>(0x5600, 1);   
}

inline void ExportEFISAndFCUButtonLights(
      const WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   WilcoCockpit::FCU fcu;
   cockpit.getFCU(fcu);

   WilcoCockpit::EFISControlPanel efis;
   cockpit.getEFISControlPanel(efis);

   auto mcp_sw = cockpit.getMCPSwitches();

   struct Lights
   {
      BYTE ils;
      BYTE cstr;
      BYTE wpt;
      BYTE vord;
      BYTE ndb;
      BYTE arpt;
      BYTE loc;
      BYTE ap1;
      BYTE ap2;
      BYTE athr;
      BYTE exped;
      BYTE appr;
   } lights = 
   {
      efis.ils,
      mcp_sw == WilcoCockpit::MCP_CONSTRAINT,
      mcp_sw == WilcoCockpit::MCP_WAYPOINT,
      mcp_sw == WilcoCockpit::MCP_VORD,
      mcp_sw == WilcoCockpit::MCP_NDB,
      mcp_sw == WilcoCockpit::MCP_AIRPORT,
      fcu.loc,
      fcu.ap1,
      fcu.ap2,
      fcu.athr,
      fcu.exp,
      fcu.appr,
   };

   fsuipc.write<Lights>(0x560E, lights);
}

}; // anonymous namespace

void
ExportState(const WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   ExportModuleState(fsuipc);
   ExportEFISAndFCUButtonLights(cockpit, fsuipc);
   fsuipc.write<BYTE>(0x56D3, cockpit.getGpuSwitch());
}

}}; // namespace oac::we
