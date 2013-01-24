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

#include "cockpit.h"

namespace oac {

namespace {

void
MapFCUDown(CockpitBack& back, CockpitFront& front)
throw (CockpitBack::SyncException)
{
   CockpitBack::FlightControlUnit::EventList events;
   auto& fcu = front.getFlightControlUnit();
   back.getFlightControlUnit().pollEvents(events);
   for (auto ev : events)
   {
      switch (ev.type)
      {
         case CockpitBack::FlightControlUnit::FCU_SPD_BTN_PRESSED:
            fcu.pushSpeedUnitsButton();
            break;
         case CockpitBack::FlightControlUnit::FCU_GUI_MODE_BTN_PRESSED:
            fcu.pushGuidanceModeButton();
            break;
         case CockpitBack::FlightControlUnit::FCU_ALT_UNITS_BTN_PRESSED:
            fcu.pushAltitudeUnitsButton();
            break;
         case CockpitBack::FlightControlUnit::FCU_SW_PRESSED:
            fcu.pushSwitch(FCUSwitch(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_KNOB_PRESSED:
            fcu.pushKnob(FCUKnob(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_KNOB_PULLED:
            fcu.pullKnob(FCUKnob(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_SPD_VALUE_CHANGED:
            fcu.setSpeedValue(Knots(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_MACH_VALUE_CHANGED:
            fcu.setMachValue(Mach100(ev.value));
            break;            
         case CockpitBack::FlightControlUnit::FCU_HDG_VALUE_CHANGED:
            fcu.setHeadingValue(Degrees(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_TRACK_VALUE_CHANGED:
            fcu.setTrackValue(Degrees(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_ALT_VALUE_CHANGED:
            fcu.setTargetAltitude(Feet(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_VS_VALUE_CHANGED:
            fcu.setVerticalSpeedValue(FeetPerMin(ev.value));
            break;
         case CockpitBack::FlightControlUnit::FCU_FPA_VALUE_CHANGED:
            fcu.setFPAValue(Degrees100(ev.value));
            break;
      }
   }
}

void
MapEFISControlPanelDown(CockpitBack& back, CockpitFront& front)
throw (CockpitBack::SyncException)
{
   CockpitBack::EFISControlPanel::EventList events;
   auto& efis = front.getEFISControlPanel();
   back.getEFISControlPanel().pollEvents(events);
   for (auto ev : events)
   {
      switch (ev.type)
      {
         case CockpitBack::EFISControlPanel::EFIS_CTRL_FD_BTN_PRESSED:
            efis.pushFDButton(); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_ILS_BTN_PRESSED:
            efis.pushILSButton(); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_MCP_SW_PRESSED:
            efis.pushMCPSwitch(MCPSwitch(ev.value)); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_BARO_MODE_SELECTED:
            efis.setBarometricMode(BarometricMode(ev.value)); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_BARO_FMT_SELECTED:
            efis.setBarometricFormat(BarometricFormat(ev.value)); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_ND_MODE_SELECTED:
            efis.setNDModeSwitch(NDModeSwitch(ev.value)); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_ND_RANGE_SELECTED:
            efis.setNDRangeSwitch(NDRangeSwitch(ev.value)); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_ND_NAV1_MODE_SELECTED:
            efis.setNDNav1ModeSwitch(NDNavModeSwitch(ev.value)); 
            break;
         case CockpitBack::EFISControlPanel::EFIS_CTRL_ND_NAV2_MODE_SELECTED:
            efis.setNDNav2ModeSwitch(NDNavModeSwitch(ev.value)); 
            break;
      }
   }
}

void
MapDown(CockpitBack& back, CockpitFront& front)
throw (CockpitBack::SyncException)
{
   MapFCUDown(back, front);
   MapEFISControlPanelDown(back, front);
}

void
MapFCUUp(CockpitBack& back, CockpitFront& front)
throw (CockpitBack::SyncException)
{
   auto& fcu_back = back.getFlightControlUnit();
   auto& fcu_front = front.getFlightControlUnit();

   fcu_back.setSpeedDisplayUnits(fcu_front.getSpeedDisplayUnits());
   fcu_back.setGuidanceDisplayMode(fcu_front.getGuidanceDisplayMode());
   fcu_back.setAltitudeDisplayUnits(fcu_front.getAltitudeDisplayUnits());
   fcu_back.setSwitch(FCU_SWITCH_LOC, fcu_front.getSwitch(FCU_SWITCH_LOC));
   fcu_back.setSwitch(FCU_SWITCH_ATHR, fcu_front.getSwitch(FCU_SWITCH_ATHR));
   fcu_back.setSwitch(FCU_SWITCH_EXPE, fcu_front.getSwitch(FCU_SWITCH_EXPE));
   fcu_back.setSwitch(FCU_SWITCH_APPR, fcu_front.getSwitch(FCU_SWITCH_APPR));
   fcu_back.setSwitch(FCU_SWITCH_AP1, fcu_front.getSwitch(FCU_SWITCH_AP1));
   fcu_back.setSwitch(FCU_SWITCH_AP2, fcu_front.getSwitch(FCU_SWITCH_AP2));
   fcu_back.setSpeedMode(fcu_front.getSpeedMode());
   fcu_back.setLateralMode(fcu_front.getLateralMode());
   fcu_back.setVerticalMode(fcu_front.getVerticalMode());
   fcu_back.setSpeedValue(fcu_front.getSpeedValue());
   fcu_back.setMachValue(fcu_front.getMachValue());
   fcu_back.setHeadingValue(fcu_front.getHeadingValue());
   fcu_back.setTrackValue(fcu_front.getTrackValue());
   fcu_back.setTargetAltitude(fcu_front.getTargetAltitude());
   fcu_back.setVerticalSpeedValue(fcu_front.getVerticalSpeedValue());
   fcu_back.setFPAValue(fcu_front.getFPAValue());
}

void
MapEFISControlPanelUp(CockpitBack& back, CockpitFront& front)
throw (CockpitBack::SyncException)
{
   auto& panel_back = back.getEFISControlPanel();
   auto& panel_front = front.getEFISControlPanel();
   panel_back.setBarometricMode(panel_front.getBarometricMode());
   panel_back.setBarometricFormat(panel_front.getBarometricFormat());
   panel_back.setILSButton(panel_front.getILSButton());
   panel_back.setMCPSwitch(MCP_CONSTRAINT, 
         panel_front.getMCPSwitch(MCP_CONSTRAINT));
   panel_back.setMCPSwitch(MCP_WAYPOINT, 
         panel_front.getMCPSwitch(MCP_WAYPOINT));
   panel_back.setMCPSwitch(MCP_VORD, 
         panel_front.getMCPSwitch(MCP_VORD));
   panel_back.setMCPSwitch(MCP_NDB, 
         panel_front.getMCPSwitch(MCP_NDB));
   panel_back.setMCPSwitch(MCP_AIRPORT, 
         panel_front.getMCPSwitch(MCP_AIRPORT));
   panel_back.setNDModeSwitch(panel_front.getNDModeSwitch());
   panel_back.setNDRangeSwitch(panel_front.getNDRangeSwitch());
   panel_back.setNDNav1ModeSwitch(panel_front.getNDNav1ModeSwitch());
   panel_back.setNDNav2ModeSwitch(panel_front.getNDNav2ModeSwitch());
}

void
MapUp(CockpitBack& back, CockpitFront& front)
throw (CockpitBack::SyncException)
{
   MapFCUUp(back, front);
   MapEFISControlPanelUp(back, front);
}

void
Map(CockpitBack& back, CockpitFront& front)
throw (CockpitBack::SyncException)
{
   back.syncDown();
   MapDown(back, front);
   MapUp(back, front);
   back.syncUp();
}

}; // anonymous namespace

void
CockpitFront::map(CockpitBack& back) 
{ Map(back, *this); }

void
CockpitBack::map(CockpitFront& front)
throw (SyncException)
{ Map(*this, front); }

}; // namespace oac
