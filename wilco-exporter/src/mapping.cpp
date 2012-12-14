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

struct EFISControlPanelAndFCU {

   inline static void ExportButtonLights(
         const WilcoCockpit::FCU& fcu, 
         const WilcoCockpit::EFISControlPanel& efis,
         const WilcoCockpit::MCPSwitches& mcp_sw,
         FSUIPC& fsuipc)
   {
      BYTE lights[] = 
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

      fsuipc.write<decltype(lights)>(0x560E, lights);
   }

   inline static void ExportModeButtonsAndSwitches(
         const WilcoCockpit::EFISControlPanel& efis,
         FSUIPC& fsuipc)
   {
      fsuipc.write<BYTE>(0x561C, efis.baro_fmt);
      fsuipc.write<BYTE>(0x561F, efis.baro_mode);
   }

   inline static void Export(const WilcoCockpit& cockpit, FSUIPC& fsuipc) {
      WilcoCockpit::FCU fcu;
      WilcoCockpit::EFISControlPanel efis;
      auto mcp_sw = cockpit.getMCPSwitches();
      cockpit.getFCU(fcu);
      cockpit.getEFISControlPanel(efis);

      ExportButtonLights(fcu, efis, mcp_sw, fsuipc);
      ExportModeButtonsAndSwitches(efis, fsuipc);
   }
};

}; // anonymous namespace

void
ExportState(const WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   ExportModuleState(fsuipc);
   EFISControlPanelAndFCU::Export(cockpit, fsuipc);
   fsuipc.write<BYTE>(0x56D3, cockpit.getGpuSwitch());
}

}}; // namespace oac::we
