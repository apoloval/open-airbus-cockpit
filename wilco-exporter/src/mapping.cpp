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

struct EFISControlPanelAndFlightControlUnit {

   inline static void Import(
      WilcoCockpit& cockpit,
      FSUIPC& fsuipc)
   {
      EFISControlPanel& efis_ctrl_panel = cockpit.getEFISControlPanel();
      FlightControlUnit& fcu = cockpit.getFlightControlUnit();
      ImportPushButtons(fsuipc, efis_ctrl_panel, fcu);
   }

   inline static void Export(const WilcoCockpit& cockpit, FSUIPC& fsuipc) {
      const FlightControlUnit& fcu = cockpit.getFlightControlUnit();
      const EFISControlPanel& efis = cockpit.getEFISControlPanel();

      ExportButtonLights(fcu, efis, fsuipc);
      ExportModeButtonsAndSwitches(fcu, efis, fsuipc);
   }

private:

   inline static void ImportPushButtons(      
      FSUIPC& fsuipc, 
      EFISControlPanel& efis_ctrl_panel,
      FlightControlUnit& fcu)
   {
      ImportBinarySwitch<0x5601>(fsuipc, [&fsuipc] () { 
               auto current_fd = fsuipc.read<DWORD>(0x2EE0);
               fsuipc.write<DWORD>(0x2EE0, !current_fd);
      });
      ImportBinarySwitch<0x5602>(fsuipc, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.toggleILSButton();
      });
      ImportBinarySwitch<0x5603>(fsuipc, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_CONSTRAINT);
      });
      ImportBinarySwitch<0x5604>(fsuipc, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_WAYPOINT);
      });
      ImportBinarySwitch<0x5605>(fsuipc, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_VORD);
      });
      ImportBinarySwitch<0x5606>(fsuipc, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_NDB);
      });
      ImportBinarySwitch<0x5607>(fsuipc, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_AIRPORT);
      });
      ImportBinarySwitch<0x5608>(fsuipc, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_LOC);
      });
      ImportBinarySwitch<0x5609>(fsuipc, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_AP1);
      });
      ImportBinarySwitch<0x560A>(fsuipc, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_AP2);
      });
      ImportBinarySwitch<0x560B>(fsuipc, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_ATHR);
      });
      ImportBinarySwitch<0x560C>(fsuipc, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_EXPE);
      });
      ImportBinarySwitch<0x560D>(fsuipc, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_APPR);
      });
   }

  template <DWORD offset>
   inline static void ImportBinarySwitch(
         FSUIPC& fsuipc,
         std::function<void(void)> action)
   {
      auto push = fsuipc.read<BYTE>(offset);
      if (push)
      {
         action();
         fsuipc.write<BYTE>(offset, 0);
      }
   }

   inline static void ExportButtonLights(
         const FlightControlUnit& fcu, 
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
         fcu.getSwitch(FlightControlUnit::SWITCH_LOC),
         fcu.getSwitch(FlightControlUnit::SWITCH_AP1),
         fcu.getSwitch(FlightControlUnit::SWITCH_AP2),
         fcu.getSwitch(FlightControlUnit::SWITCH_ATHR),
         fcu.getSwitch(FlightControlUnit::SWITCH_EXPE),
         fcu.getSwitch(FlightControlUnit::SWITCH_APPR),
      };

      fsuipc.write<decltype(lights)>(0x560E, lights);
   }

   inline static void ExportModeButtonsAndSwitches(
         const FlightControlUnit& fcu, 
         const EFISControlPanel& efis,
         FSUIPC& fsuipc)
   {
      fsuipc.write<BYTE>(0x561C, efis.getBarometricFormat());
      fsuipc.write<BYTE>(0x561F, efis.getBarometricMode());
      fsuipc.write<BYTE>(0x5623, efis.getNDModeSwitch());
      fsuipc.write<BYTE>(0x5624, efis. getNDRangeSwitch());
      fsuipc.write<BYTE>(0x5625, efis.getNDNav1ModeSwitch());
      fsuipc.write<BYTE>(0x5626, efis.getNDNav2ModeSwitch());

      fsuipc.write<BYTE>(0x562F, fcu.getSpeedDisplayUnits());
      fsuipc.write<BYTE>(0x5630, fcu.getGuidanceDisplayMode());
      fsuipc.write<BYTE>(0x5631, fcu.getAltitudeDisplayUnits());
      fsuipc.write<BYTE>(0x5632, fcu.getSpeedMode());
      fsuipc.write<BYTE>(0x5633, fcu.getLateralMode());
      fsuipc.write<BYTE>(0x5634, fcu.getVerticalMode());

      fsuipc.write<DWORD>(0x5638, fsuipc.read<DWORD>(0x07E2));
      fsuipc.write<DWORD>(0x563C, 
         DWORD(floor(0.5 + fsuipc.read<DWORD>(0x07E8) / 655.36f)));
      fsuipc.write<DWORD>(0x5640, 
         DWORD(floor(0.5 + 360.0*fsuipc.read<DWORD>(0x07CC) / 65536.0f)));
      fsuipc.write<DWORD>(0x5644, DWORD(floor(0.5 + fcu.getTrackValue())));
      fsuipc.write<DWORD>(0x5648, DWORD(fcu.getTargetAltitude()));
      fsuipc.write<DWORD>(0x564C, DWORD(fcu.getVerticalSpeedValue()));
      fsuipc.write<DWORD>(0x5650, DWORD(floor(0.5 + 100*fcu.getFPAValue())));
   }
};

}; // anonymous namespace

void
ImportState(WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   EFISControlPanelAndFlightControlUnit::Import(cockpit, fsuipc);
}

void
ExportState(const WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   ExportModuleState(fsuipc);
   EFISControlPanelAndFlightControlUnit::Export(cockpit, fsuipc);
   fsuipc.write<BYTE>(0x56D3, cockpit.getGpuSwitch());
}

}}; // namespace oac::we
