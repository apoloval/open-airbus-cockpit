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

#include "logging.h"
#include "mapping.h"

namespace oac { namespace we {

namespace {

inline void ExportModuleState(FSUIPC& fsuipc)
{
   fsuipc.write<BYTE>(0x5600, 1);   
}

struct EFISControlPanelAndFCU {

   inline static void Import(
      WilcoCockpit& cockpit,
      FSUIPC& fsuipc)
   {
      EFISControlPanel& efis_ctrl_panel = cockpit.getEFISControlPanel();
      ImportPushButtons(fsuipc, efis_ctrl_panel);
   }

   inline static void Export(const WilcoCockpit& cockpit, FSUIPC& fsuipc) {
      FCU fcu;
      const EFISControlPanel& efis = cockpit.getEFISControlPanel();
      cockpit.getFCU(fcu);

      ExportButtonLights(fcu, efis, fsuipc);
      ExportModeButtonsAndSwitches(fcu, efis, fsuipc);
   }

private:

   inline static void ImportPushButtons(      
      FSUIPC& fsuipc, EFISControlPanel& efis_ctrl_panel)
   {
      ImportFDButton(fsuipc);
      ImportILSButton(fsuipc, efis_ctrl_panel);
      ImportMCPSwitches(fsuipc, efis_ctrl_panel);
   }

   inline static void ImportFDButton(FSUIPC& fsuipc)
   {
      auto push_fd = fsuipc.read<BYTE>(0x5601);
      if (push_fd)
      {
         auto current_fd = fsuipc.read<DWORD>(0x2EE0);
         fsuipc.write<DWORD>(0x2EE0, !current_fd);
         fsuipc.write<BYTE>(0x5601, 0);
      }
   }

   inline static void ImportILSButton(
         FSUIPC& fsuipc, EFISControlPanel& efis_ctrl_panel)
   {
      auto push_ils = fsuipc.read<BYTE>(0x5602);
      if (push_ils)
      {
         efis_ctrl_panel.toggleILSButton();
         fsuipc.write<BYTE>(0x5602, 0);
      }
   }

   inline static void ImportMCPSwitches(
         FSUIPC& fsuipc, EFISControlPanel& efis_ctrl_panel)
   {
      ImportMCPSwitch<0x5603, MCP_CONSTRAINT>(fsuipc, efis_ctrl_panel);
      ImportMCPSwitch<0x5604, MCP_WAYPOINT>(fsuipc, efis_ctrl_panel);
      ImportMCPSwitch<0x5605, MCP_VORD>(fsuipc, efis_ctrl_panel);
      ImportMCPSwitch<0x5606, MCP_NDB>(fsuipc, efis_ctrl_panel);
      ImportMCPSwitch<0x5607, MCP_AIRPORT>(fsuipc, efis_ctrl_panel);
   }

   template <DWORD offset, MCPSwitch sw>
   inline static void ImportMCPSwitch(
         FSUIPC& fsuipc, EFISControlPanel& efis_ctrl_panel)
   {
      auto push = fsuipc.read<BYTE>(offset);
      if (push)
      {
         efis_ctrl_panel.pushMCPSwitch(sw);
         fsuipc.write<BYTE>(offset, 0);
      }
   }

   inline static void ExportButtonLights(
         const FCU& fcu, 
         const EFISControlPanel& efis,
         FSUIPC& fsuipc)
   {
      BYTE lights[] = 
      {
         efis.getILSButton(),
         efis.getMCPSwitch(MCP_CONSTRAINT),
         efis.getMCPSwitch(MCP_WAYPOINT),
         efis.getMCPSwitch(MCP_VORD),
         efis.getMCPSwitch(MCP_NDB),
         efis.getMCPSwitch(MCP_AIRPORT),
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
         const FCU& fcu, 
         const EFISControlPanel& efis,
         FSUIPC& fsuipc)
   {
      fsuipc.write<BYTE>(0x561C, efis.getBarometricFormat());
      fsuipc.write<BYTE>(0x561F, efis.getBarometricMode());
      fsuipc.write<BYTE>(0x5623, efis.getNDModeSwitch());
      fsuipc.write<BYTE>(0x5624, efis. getNDRangeSwitch());
      fsuipc.write<BYTE>(0x5625, efis.getNDNav1ModeSwitch());
      fsuipc.write<BYTE>(0x5626, efis.getNDNav2ModeSwitch());

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
};

}; // anonymous namespace

void
ImportState(WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   EFISControlPanelAndFCU::Import(cockpit, fsuipc);
}

void
ExportState(const WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   ExportModuleState(fsuipc);
   EFISControlPanelAndFCU::Export(cockpit, fsuipc);
   fsuipc.write<BYTE>(0x56D3, cockpit.getGpuSwitch());
}

}}; // namespace oac::we
