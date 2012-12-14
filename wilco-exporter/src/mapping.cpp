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

#include <cmath>

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
         const WilcoCockpit::FCU& fcu, 
         const WilcoCockpit::EFISControlPanel& efis,
         FSUIPC& fsuipc)
   {
      fsuipc.write<BYTE>(0x561C, efis.baro_fmt);
      fsuipc.write<BYTE>(0x561F, efis.baro_mode);
      fsuipc.write<BYTE>(0x5623, efis.nd_mode);
      fsuipc.write<BYTE>(0x5624, efis.nd_range);
      fsuipc.write<BYTE>(0x5625, efis.bearing_1);
      fsuipc.write<BYTE>(0x5626, efis.bearing_2);

      fsuipc.write<BYTE>(0x562F, fcu.spd_dsp_mod);
      fsuipc.write<BYTE>(0x5630, fcu.lat_ver_dsp_mod);
      fsuipc.write<BYTE>(0x5631, fcu.alt_dsp_mod);
      fsuipc.write<BYTE>(0x5632, fcu.spd_mngt_mod);
      fsuipc.write<BYTE>(0x5633, fcu.hdg_mngt_mod);
      fsuipc.write<BYTE>(0x5634, fcu.vs_mngt_mod);

      fsuipc.write<DWORD>(0x5638, fsuipc.read<DWORD>(0x07E2));
      fsuipc.write<DWORD>(0x563C, 
         DWORD(floor(0.5 + fsuipc.read<DWORD>(0x07E8) / 655.36f)));
      fsuipc.write<DWORD>(0x5640, 
         DWORD(floor(0.5 + 360.0*fsuipc.read<DWORD>(0x07CC) / 65536.0f)));
      fsuipc.write<DWORD>(0x5644, DWORD(floor(0.5 + fcu.sel_track)));
      fsuipc.write<DWORD>(0x5648, DWORD(fcu.sel_alt));
      fsuipc.write<DWORD>(0x564C, DWORD(fcu.sel_vs));
      fsuipc.write<DWORD>(0x5650, DWORD(floor(0.5 + 100*fcu.sel_fpa)));
   }

   inline static void Export(const WilcoCockpit& cockpit, FSUIPC& fsuipc) {
      WilcoCockpit::FCU fcu;
      WilcoCockpit::EFISControlPanel efis;
      auto mcp_sw = cockpit.getMCPSwitches();
      cockpit.getFCU(fcu);
      cockpit.getEFISControlPanel(efis);

      ExportButtonLights(fcu, efis, mcp_sw, fsuipc);
      ExportModeButtonsAndSwitches(fcu, efis, fsuipc);
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
