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

inline void ExportModuleState(DoubleBuffer& buffer)
{
   buffer.writeAs<BYTE>(0x5600, 1);
}

struct EFISControlPanelAndFlightControlUnit {

   inline static void Import(
         FSUIPC& fsuipc,
         WilcoCockpit& cockpit,
         DoubleBuffer& buffer)
   {
      EFISControlPanel& efis_ctrl_panel = cockpit.getEFISControlPanel();
      FlightControlUnit& fcu = cockpit.getFlightControlUnit();
      ImportPushButtons(buffer, fsuipc, efis_ctrl_panel, fcu);
      ImportModeButtonAndSwitches(buffer, efis_ctrl_panel, fcu);
   }

   inline static void Export(
         const FSUIPC& fsuipc,
         const WilcoCockpit& cockpit, 
         DoubleBuffer& buffer)
   {
      const FlightControlUnit& fcu = cockpit.getFlightControlUnit();
      const EFISControlPanel& efis = cockpit.getEFISControlPanel();

      ExportButtonLights(fcu, efis, buffer);
      ExportModeButtonsAndSwitches(fsuipc, fcu, efis, buffer);
   }

private:

   inline static void ImportPushButtons(      
         DoubleBuffer& buffer, 
         FSUIPC& fsuipc,
         EFISControlPanel& efis_ctrl_panel,
         FlightControlUnit& fcu)
   {
      ImportBinarySwitch<0x5601>(buffer, [&fsuipc] () { 
               auto current_fd = fsuipc.readAs<DWORD>(0x2EE0);
               fsuipc.writeAs<DWORD>(0x2EE0, !current_fd);
      });
      ImportBinarySwitch<0x5602>(buffer, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushILSButton();
      });
      ImportBinarySwitch<0x5603>(buffer, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_CONSTRAINT);
      });
      ImportBinarySwitch<0x5604>(buffer, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_WAYPOINT);
      });
      ImportBinarySwitch<0x5605>(buffer, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_VORD);
      });
      ImportBinarySwitch<0x5606>(buffer, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_NDB);
      });
      ImportBinarySwitch<0x5607>(buffer, [&efis_ctrl_panel] () { 
               efis_ctrl_panel.pushMCPSwitch(MCP_AIRPORT);
      });
      ImportBinarySwitch<0x5608>(buffer, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_LOC);
      });
      ImportBinarySwitch<0x5609>(buffer, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_AP1);
      });
      ImportBinarySwitch<0x560A>(buffer, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_AP2);
      });
      ImportBinarySwitch<0x560B>(buffer, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_ATHR);
      });
      ImportBinarySwitch<0x560C>(buffer, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_EXPE);
      });
      ImportBinarySwitch<0x560D>(buffer, [&fcu] () { 
               fcu.pushSwitch(FlightControlUnit::SWITCH_APPR);
      });
   }

   inline static void ImportModeButtonAndSwitches(
         DoubleBuffer& buffer, 
         EFISControlPanel& efis,
         FlightControlUnit& fcu)
   {
      auto prev_baro = efis.getBarometricMode();

      ImportBinarySwitch<0x561A>(buffer, [&efis] () { 
               efis.setBarometricFormat(BARO_FMT_IN_HG);
      });
      ImportBinarySwitch<0x561B>(buffer, [&efis] () { 
               efis.setBarometricFormat(BARO_FMT_H_PA);
      });
      ImportBinarySwitch<0x561D>(buffer, [&efis] () { 
               efis.setBarometricMode(BARO_SELECTED);
      });
      ImportBinarySwitch<0x561E>(buffer, [&efis] () { 
               efis.setBarometricMode(BARO_STANDARD);
      });
      if (prev_baro == efis.getBarometricMode())
         ImportSwitchValue<0x561F>(buffer, prev_baro, [&efis] (BYTE new_value) {
            efis.setBarometricMode(BarometricMode(new_value));
         });
      ImportBinarySwitch<0x5620>(buffer, [&fcu] () { 
               fcu.pushSpeedUnitsButton();
      });      
      ImportBinarySwitch<0x5621>(buffer, [&fcu] () { 
               fcu.pushGuidanceModeButton();
      });
      ImportBinarySwitch<0x5622>(buffer, [&fcu] () { 
               fcu.pushAltitudeUnitsButton();
      });
      ImportSwitchValue<0x5623>(buffer, efis.getNDModeSwitch(), 
         [&efis] (BYTE new_value) {
            efis.setNDModeSwitch(NDModeSwitch(new_value));
         });
      ImportSwitchValue<0x5624>(buffer, efis.getNDRangeSwitch(), 
         [&efis] (BYTE new_value) {
            efis.setNDRangeSwitch(NDRangeSwitch(new_value));
         });
   }

   template <DWORD offset>
   inline static void ImportBinarySwitch(
         DoubleBuffer& buffer,
         std::function<void(void)> action)
   {
      auto push = buffer.readAs<BYTE>(offset);
      if (push)
      {
         action();
         buffer.writeAs<BYTE>(offset, 0);
      }
   }

   template <DWORD offset>
   inline static void ImportSwitchValue(
         DoubleBuffer& buffer,
         BYTE old_value,
         std::function<void(BYTE new_value)> action)
   {
      if (buffer.isModifiedAs<BYTE>(offset))
         action(buffer.readAs<BYTE>(offset));
   }

   inline static void ExportButtonLights(
         const FlightControlUnit& fcu, 
         const EFISControlPanel& efis,
         DoubleBuffer& buffer)
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

      buffer.writeAs<decltype(lights)>(0x560E, lights);
   }

   inline static void ExportModeButtonsAndSwitches(
         const FSUIPC& fsuipc,
         const FlightControlUnit& fcu, 
         const EFISControlPanel& efis,
         DoubleBuffer& buffer)
   {
      buffer.writeAs<BYTE>(0x561C, efis.getBarometricFormat());
      buffer.writeAs<BYTE>(0x561F, efis.getBarometricMode());
      buffer.writeAs<BYTE>(0x5623, efis.getNDModeSwitch());
      buffer.writeAs<BYTE>(0x5624, efis.getNDRangeSwitch());
      buffer.writeAs<BYTE>(0x5625, efis.getNDNav1ModeSwitch());
      buffer.writeAs<BYTE>(0x5626, efis.getNDNav2ModeSwitch());

      buffer.writeAs<BYTE>(0x562F, fcu.getSpeedDisplayUnits());
      buffer.writeAs<BYTE>(0x5630, fcu.getGuidanceDisplayMode());
      buffer.writeAs<BYTE>(0x5631, fcu.getAltitudeDisplayUnits());
      buffer.writeAs<BYTE>(0x5632, fcu.getSpeedMode());
      buffer.writeAs<BYTE>(0x5633, fcu.getLateralMode());
      buffer.writeAs<BYTE>(0x5634, fcu.getVerticalMode());

      buffer.writeAs<DWORD>(0x5638, fsuipc.readAs<DWORD>(0x07E2));
      buffer.writeAs<DWORD>(0x563C, 
         DWORD(floor(0.5 + fsuipc.readAs<DWORD>(0x07E8) / 655.36f)));
      buffer.writeAs<DWORD>(0x5640, 
         DWORD(floor(0.5 + 360.0*fsuipc.readAs<DWORD>(0x07CC) / 65536.0f)));
      buffer.writeAs<DWORD>(0x5644, DWORD(floor(0.5 + fcu.getTrackValue())));
      buffer.writeAs<DWORD>(0x5648, DWORD(fcu.getTargetAltitude()));
      buffer.writeAs<DWORD>(0x564C, DWORD(fcu.getVerticalSpeedValue()));
      buffer.writeAs<DWORD>(0x5650, DWORD(floor(0.5 + 100*fcu.getFPAValue())));
   }
};

}; // anonymous namespace

Mapper::Mapper()
{
}

void
Mapper::importState(WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   this->syncBuffer(fsuipc);
   EFISControlPanelAndFlightControlUnit::Import(fsuipc, cockpit, *_buffer);
}

void
Mapper::exportState(const WilcoCockpit& cockpit, FSUIPC& fsuipc)
{
   if (!_buffer)
      this->initBuffer();
   ExportModuleState(*_buffer);
   EFISControlPanelAndFlightControlUnit::Export(fsuipc, cockpit, *_buffer);
   _buffer->writeAs<BYTE>(0x56D3, cockpit.getGpuSwitch());
   fsuipc.copy(*_buffer, 0x5600, 0x5600, 512);
}

void
Mapper::initBuffer()
{
   Ptr<DoubleBuffer::Factory> fact = new DoubleBuffer::Factory(
         new ShiftedBuffer::Factory(new FixedBuffer::Factory(512), 0x5600));
   _buffer = fact->createBuffer();
}

void
Mapper::syncBuffer(FSUIPC& fsuipc)
{
   if (!_buffer)
   {
      this->initBuffer();
      _buffer->copy(fsuipc, 0x5600, 0x5600, 512);
   }
   _buffer->swap();
   _buffer->copy(fsuipc, 0x5600, 0x5600, 512);
}

}}; // namespace oac::we
