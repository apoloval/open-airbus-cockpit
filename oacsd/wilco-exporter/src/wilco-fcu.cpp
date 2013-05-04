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

flight_control_unit_impl::flight_control_unit_impl(
      const dll_info& dll_info, HINSTANCE dll_instance) :
   dll_inspector(dll_info, dll_instance), _fsuipc(new local_fsuipc())
{}

speed_units
flight_control_unit_impl::get_speed_display_units() const
{ return speed_units(this->get_data_object<DWORD>(VADDR_FCU_SPD_DISPLAY)); }

void
flight_control_unit_impl::push_speed_units_button()
{
   auto units = this->get_data_object<DWORD>(VADDR_FCU_SPD_DISPLAY);
   this->set_data_object<DWORD>(VADDR_FCU_SPD_DISPLAY, units ^ 1);
}

guidance_display_mode
flight_control_unit_impl::get_guidance_display_mode() const
{
   return this->access<guidance_display_mode>([](const wilco_fcu& fcu) {
      return guidance_display_mode(fcu.hdg_track_display_mode &&
         fcu.vs_fpa_display_mode);
   });
}

void
flight_control_unit_impl::push_guidance_display_mode()
{
   this->send_command(CMD_FCU_PRESS_HDG_TRK_BTN);
}

altitude_units
flight_control_unit_impl::get_altitude_display_units() const
{
   return this->access<altitude_units>([](const wilco_fcu& fcu) {
      return altitude_units(fcu.metric_altitude);
   });
}

void
flight_control_unit_impl::push_altitude_units_button()
{
   this->send_command(CMD_FCU_PRESS_METRIC_ALT_BTN);
}

binary_switch
flight_control_unit_impl::get_switch(fcu_switch sw) const
{
   return this->access<binary_switch>([&sw](const wilco_fcu& fcu) {
      switch (sw)
      {
         case FCU_SWITCH_LOC:
            return binary_switch(
                  (fcu.armed_lateral_mode == LAT_MOD_LOC ||
                     fcu.active_lateral_mode == LAT_MOD_LOC) &&
                  (fcu.armed_vertical_mode != VER_MOD_GS &&
                     fcu.active_vertical_mode != VER_MOD_GS));
         case FCU_SWITCH_ATHR:
            return fcu.auto_thrust > 0 ? SWITCHED_ON : SWITCHED_OFF;
         case FCU_SWITCH_EXPE:
            return binary_switch(fcu.expedite);
         case FCU_SWITCH_APPR:
             return binary_switch(fcu.armed_vertical_mode == VER_MOD_GS ||
                  fcu.active_vertical_mode == VER_MOD_GS);
         case FCU_SWITCH_AP1:
            return binary_switch(fcu.autopilot & AP_1);
         case FCU_SWITCH_AP2:
            return binary_switch(fcu.autopilot & AP_2);
         default:
            return SWITCHED_OFF;
      }
   });
}

void
flight_control_unit_impl::push_switch(fcu_switch sw)
{
   switch (sw)
   {
      case FCU_SWITCH_LOC:
         this->send_command(CMD_FCU_PUSH_LOC_BTN);
         break;
      case FCU_SWITCH_ATHR:
         this->mutate([](wilco_fcu& fcu) {
            fcu.auto_thrust ^= 2;
         });
         break;
      case FCU_SWITCH_EXPE:
         this->send_command(CMD_FCU_PUSH_EXPED_BTN);
         break;
      case FCU_SWITCH_APPR:
         this->send_command(CMD_FCU_PUSH_APPR_BTN);
         break;
      case FCU_SWITCH_AP1:
         this->send_command(CMD_FCU_PUSH_AP1_BTN);
         break;
      case FCU_SWITCH_AP2:
         this->send_command(CMD_FCU_PUSH_AP2_BTN);
         break;
   }
}

fcu_management_mode
flight_control_unit_impl::get_speed_mode() const
{
   return this->access<fcu_management_mode>([](const wilco_fcu& fcu) {
      return fcu_management_mode(fcu.speed_knob);
   });
}

fcu_management_mode
flight_control_unit_impl::get_lateral_mode() const
{
   return this->access<fcu_management_mode>([](const wilco_fcu& fcu) {
      return fcu_management_mode(fcu.heading_knob);
   });
}

fcu_management_mode
flight_control_unit_impl::get_vertical_mode() const
{
   return this->access<fcu_management_mode>([](const wilco_fcu& fcu) {
      return fcu_management_mode(fcu.vertical_speed_knob);
   });
}

knots
flight_control_unit_impl::get_speed_value() const
{
   return buffer::read_as<WORD>(*_fsuipc, 0x07E2);
}

void
flight_control_unit_impl::set_speed_value(knots value)
{
   this->send_command<knots>(CMD_FCU_SET_SPD_VALUE, value);
}

mach100
flight_control_unit_impl::get_mach_value() const
{
   auto v = buffer::read_as<DWORD>(*_fsuipc, 0x07E8);
   return boost::math::iround<double>(v / 655.36);
}

void
flight_control_unit_impl::set_mach_value(mach100 value)
{
   DWORD v = boost::math::iround(value * 655.36);
   buffer::write_as<DWORD>(*_fsuipc, 0x07E8, v);
}

degrees
flight_control_unit_impl::get_heading_value() const
{
   auto v = buffer::read_as<DWORD>(*_fsuipc, 0x07CC);
   return boost::math::iround<double>(v * 360.0 / 65536.0);
}

void
flight_control_unit_impl::set_heading_value(degrees value)
{
   this->send_command<degrees>(CMD_FCU_SET_HDG_VALUE, value);
}

degrees
flight_control_unit_impl::get_track_value() const
{
   return this->access<degrees>([](const wilco_fcu& fcu) {
      return degrees(fcu.selected_track);
   });
}

void
flight_control_unit_impl::set_track_value(degrees value)
{
   this->mutate([value](wilco_fcu& fcu) {
      fcu.selected_track = value;
   });
}

feet
flight_control_unit_impl::get_target_altitude() const
{
   return this->access<feet>([](const wilco_fcu& fcu) {
      return fcu.target_altitude;
   });
}

void
flight_control_unit_impl::set_target_altitude(feet value)
{
   this->send_command<feet>(CMD_FCU_SET_ALT_VALUE, value);
}

feet_per_min
flight_control_unit_impl::get_vertical_speed_value() const
{
   return this->access<feet_per_min>([](const wilco_fcu& fcu) {
      return fcu.selected_vertical_speed;
   });
}

void
flight_control_unit_impl::set_vertical_speed_value(feet_per_min value)
{
   auto fvalue = FLOAT(value);
   this->send_command<FLOAT>(CMD_FCU_SET_VS_VALUE, fvalue);
}

degrees100
flight_control_unit_impl::get_fpa_value() const
{
   return this->access<degrees100>([](const wilco_fcu& fcu) {
      return boost::math::iround(fcu.selected_fpa * 100.0f);
   });
}

void
flight_control_unit_impl::set_fpa_value(degrees100 value)
{
   this->mutate([value](wilco_fcu& fcu) {
      fcu.selected_fpa = (value / 100.0f);
   });
}

void
flight_control_unit_impl::push_knob(fcu_knob knob)
{
   switch (knob)
   {
      case FCU_KNOB_SPD:
         this->send_command(CMD_FCU_PUSH_SPD_KNOB);
         break;
      case FCU_KNOB_HDG:
         this->send_command(CMD_FCU_PUSH_HDG_KNOB);
         break;
      case FCU_KNOB_ALT:
         this->send_command(CMD_FCU_PUSH_ALT_KNOB);
         break;
      case FCU_KNOB_VS:
         this->send_command(CMD_FCU_PUSH_VS_KNOB);
         break;
   }
}

void
flight_control_unit_impl::pull_knob(fcu_knob knob)
{
   switch (knob)
   {
      case FCU_KNOB_SPD:
         this->send_command(CMD_FCU_PULL_SPD_KNOB);
         break;
      case FCU_KNOB_HDG:
         this->send_command(CMD_FCU_PULL_HDG_KNOB);
         break;
      case FCU_KNOB_ALT:
         this->send_command(CMD_FCU_PULL_ALT_KNOB);
         break;
      case FCU_KNOB_VS:
         this->send_command(CMD_FCU_PULL_VS_KNOB);
         break;
   }
}


}} // namespace oac
