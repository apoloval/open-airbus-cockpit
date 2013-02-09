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

#include <boost/math/special_functions/round.hpp>

#include "wilco-fcu.h"

namespace oac { namespace we {

FlightControlUnitImpl::FlightControlUnitImpl(
      const DllInfo& dll_info, HINSTANCE dll_instance) :
   DllInspector(dll_info, dll_instance), _fsuipc(new LocalFSUIPC())
{}

SpeedUnits
FlightControlUnitImpl::getSpeedDisplayUnits() const
{ return SpeedUnits(this->getDataObject<DWORD>(VADDR_FCU_SPD_DISPLAY)); }

void
FlightControlUnitImpl::pushSpeedUnitsButton()
{
   auto units = this->getDataObject<DWORD>(VADDR_FCU_SPD_DISPLAY);
   this->setDataObject<DWORD>(VADDR_FCU_SPD_DISPLAY, units ^ 1);
}

GuidanceDisplayMode
FlightControlUnitImpl::getGuidanceDisplayMode() const
{
   return this->access<GuidanceDisplayMode>([](const Wilco_FCU& fcu) {
      return GuidanceDisplayMode(fcu.hdg_track_display_mode &&
         fcu.vs_fpa_display_mode);
   });
}

void
FlightControlUnitImpl::pushGuidanceModeButton()
{
   this->sendCommand(CMD_FCU_PRESS_HDG_TRK_BTN);
}

AltitudeUnits
FlightControlUnitImpl::getAltitudeDisplayUnits() const
{
   return this->access<AltitudeUnits>([](const Wilco_FCU& fcu) {
      return AltitudeUnits(fcu.metric_altitude);
   });
}

void
FlightControlUnitImpl::pushAltitudeUnitsButton()
{
   this->sendCommand(CMD_FCU_PRESS_METRIC_ALT_BTN);
}

BinarySwitch
FlightControlUnitImpl::getSwitch(FCUSwitch sw) const
{
   return this->access<BinarySwitch>([&sw](const Wilco_FCU& fcu) {
      switch (sw)
      {
         case FCU_SWITCH_LOC:
            return BinarySwitch(
                  (fcu.armed_lateral_mode == LAT_MOD_LOC ||
                     fcu.active_lateral_mode == LAT_MOD_LOC) &&
                  (fcu.armed_vertical_mode != VER_MOD_GS &&
                     fcu.active_vertical_mode != VER_MOD_GS));
         case FCU_SWITCH_ATHR:
            return fcu.auto_thrust > 0 ? SWITCHED_ON : SWITCHED_OFF;
         case FCU_SWITCH_EXPE:
            return BinarySwitch(fcu.expedite);
         case FCU_SWITCH_APPR:
             return BinarySwitch(fcu.armed_vertical_mode == VER_MOD_GS ||
                  fcu.active_vertical_mode == VER_MOD_GS);
         case FCU_SWITCH_AP1:
            return BinarySwitch(fcu.autopilot & AP_1);
         case FCU_SWITCH_AP2:
            return BinarySwitch(fcu.autopilot & AP_2);
         default:
            return SWITCHED_OFF;
      }
   });
}

void
FlightControlUnitImpl::pushSwitch(FCUSwitch sw)
{
   switch (sw)
   {
      case FCU_SWITCH_LOC:
         this->sendCommand(CMD_FCU_PUSH_LOC_BTN);
         break;
      case FCU_SWITCH_ATHR:
         this->mutate([](Wilco_FCU& fcu) {
            fcu.auto_thrust ^= 2;
         });
         break;
      case FCU_SWITCH_EXPE:
         this->sendCommand(CMD_FCU_PUSH_EXPED_BTN);
         break;
      case FCU_SWITCH_APPR:
         this->sendCommand(CMD_FCU_PUSH_APPR_BTN);
         break;
      case FCU_SWITCH_AP1:
         this->sendCommand(CMD_FCU_PUSH_AP1_BTN);
         break;
      case FCU_SWITCH_AP2:
         this->sendCommand(CMD_FCU_PUSH_AP2_BTN);
         break;
   }
}

FCUManagementMode
FlightControlUnitImpl::getSpeedMode() const
{
   return this->access<FCUManagementMode>([](const Wilco_FCU& fcu) {
      return FCUManagementMode(fcu.speed_knob);
   });
}

FCUManagementMode
FlightControlUnitImpl::getLateralMode() const
{
   return this->access<FCUManagementMode>([](const Wilco_FCU& fcu) {
      return FCUManagementMode(fcu.heading_knob);
   });
}

FCUManagementMode
FlightControlUnitImpl::getVerticalMode() const
{
   return this->access<FCUManagementMode>([](const Wilco_FCU& fcu) {
      return FCUManagementMode(fcu.vertical_speed_knob);
   });
}

Knots
FlightControlUnitImpl::getSpeedValue() const
{
   return _fsuipc->readAs<WORD>(0x07E2);
}

void
FlightControlUnitImpl::setSpeedValue(Knots value)
{
   this->sendCommand<Knots>(CMD_FCU_SET_SPD_VALUE, value);
}

Mach100
FlightControlUnitImpl::getMachValue() const
{
   auto v = _fsuipc->readAs<DWORD>(0x07E8);
   return boost::math::iround<double>(v / 655.36);
}

void
FlightControlUnitImpl::setMachValue(Mach100 value)
{
   DWORD v = boost::math::iround(value * 655.36);
   _fsuipc->writeAs<DWORD>(0x07E8, v);
}

Degrees
FlightControlUnitImpl::getHeadingValue() const
{
   auto v = _fsuipc->readAs<DWORD>(0x07CC);
   return boost::math::iround<double>(v * 360.0 / 65536.0);
}

void
FlightControlUnitImpl::setHeadingValue(Degrees value)
{
   this->sendCommand<Degrees>(CMD_FCU_SET_HDG_VALUE, value);
}

Degrees
FlightControlUnitImpl::getTrackValue() const
{
   return this->access<Degrees>([](const Wilco_FCU& fcu) {
      return Degrees(fcu.selected_track);
   });
}

void
FlightControlUnitImpl::setTrackValue(Degrees value)
{
   this->mutate([value](Wilco_FCU& fcu) {
      fcu.selected_track = value;
   });
}

Feet
FlightControlUnitImpl::getTargetAltitude() const
{
   return this->access<Feet>([](const Wilco_FCU& fcu) {
      return fcu.target_altitude;
   });
}

void
FlightControlUnitImpl::setTargetAltitude(Feet value)
{
   this->sendCommand<Feet>(CMD_FCU_SET_ALT_VALUE, value);
}

FeetPerMin
FlightControlUnitImpl::getVerticalSpeedValue() const
{
   return this->access<FeetPerMin>([](const Wilco_FCU& fcu) {
      return fcu.selected_vertical_speed;
   });
}

void
FlightControlUnitImpl::setVerticalSpeedValue(FeetPerMin value)
{
   auto fvalue = FLOAT(value);
   this->sendCommand<FLOAT>(CMD_FCU_SET_VS_VALUE, fvalue);
}

Degrees100
FlightControlUnitImpl::getFPAValue() const
{
   return this->access<Degrees100>([](const Wilco_FCU& fcu) {
      return boost::math::iround(fcu.selected_fpa * 100.0f);
   });
}

void
FlightControlUnitImpl::setFPAValue(Degrees100 value)
{
   this->mutate([value](Wilco_FCU& fcu) {
      fcu.selected_fpa = (value / 100.0f);
   });
}

void
FlightControlUnitImpl::pushKnob(FCUKnob knob)
{
   switch (knob)
   {
      case FCU_KNOB_SPD:
         this->sendCommand(CMD_FCU_PUSH_SPD_KNOB);
         break;
      case FCU_KNOB_HDG:
         this->sendCommand(CMD_FCU_PUSH_HDG_KNOB);
         break;
      case FCU_KNOB_ALT:
         this->sendCommand(CMD_FCU_PUSH_ALT_KNOB);
         break;
      case FCU_KNOB_VS:
         this->sendCommand(CMD_FCU_PUSH_VS_KNOB);
         break;
   }
}

void
FlightControlUnitImpl::pullKnob(FCUKnob knob)
{
   switch (knob)
   {
      case FCU_KNOB_SPD:
         this->sendCommand(CMD_FCU_PULL_SPD_KNOB);
         break;
      case FCU_KNOB_HDG:
         this->sendCommand(CMD_FCU_PULL_HDG_KNOB);
         break;
      case FCU_KNOB_ALT:
         this->sendCommand(CMD_FCU_PULL_ALT_KNOB);
         break;
      case FCU_KNOB_VS:
         this->sendCommand(CMD_FCU_PULL_VS_KNOB);
         break;
   }
}


}} // namespace oac
