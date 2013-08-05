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

#include "cockpit.h"

namespace oac {

namespace {

void
MapFCUDown(cockpit_back& back, cockpit_front& front)
throw (cockpit_back::sync_error)
{
   cockpit_back::flight_control_unit::event_list events;
   auto& fcu = front.get_flight_control_unit();
   back.get_flight_control_unit().poll_events(events);
   for (auto ev : events)
   {
      switch (ev.type)
      {
         case cockpit_back::flight_control_unit::FCU_SPD_BTN_PRESSED:
            fcu.push_speed_units_button();
            break;
         case cockpit_back::flight_control_unit::FCU_GUI_MODE_BTN_PRESSED:
            fcu.push_guidance_display_mode();
            break;
         case cockpit_back::flight_control_unit::FCU_ALT_UNITS_BTN_PRESSED:
            fcu.push_altitude_units_button();
            break;
         case cockpit_back::flight_control_unit::FCU_SW_PRESSED:
            fcu.push_switch(fcu_switch(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_KNOB_PRESSED:
            fcu.push_knob(fcu_knob(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_KNOB_PULLED:
            fcu.pull_knob(fcu_knob(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_SPD_VALUE_CHANGED:
            fcu.set_speed_value(knots(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_MACH_VALUE_CHANGED:
            fcu.set_mach_value(mach100(ev.value));
            break;            
         case cockpit_back::flight_control_unit::FCU_HDG_VALUE_CHANGED:
            fcu.set_heading_value(degrees(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_TRACK_VALUE_CHANGED:
            fcu.set_track_value(degrees(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_ALT_VALUE_CHANGED:
            fcu.set_target_altitude(feet(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_VS_VALUE_CHANGED:
            fcu.set_vertical_speed_value(feet_per_min(ev.value));
            break;
         case cockpit_back::flight_control_unit::FCU_FPA_VALUE_CHANGED:
            fcu.set_fpa_value(degrees100(ev.value));
            break;
      }
   }
}

void
Mapefis_control_panelDown(cockpit_back& back, cockpit_front& front)
throw (cockpit_back::sync_error)
{
   cockpit_back::efis_control_panel::event_list events;
   auto& efis = front.get_efis_control_panel();
   back.get_efis_control_panel().poll_events(events);
   for (auto ev : events)
   {
      switch (ev.type)
      {
         case cockpit_back::efis_control_panel::EFIS_CTRL_FD_BTN_PRESSED:
            efis.push_fd_button(); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_ILS_BTN_PRESSED:
            efis.push_ils_button(); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_MCP_SW_PRESSED:
            efis.push_mcp_switch(mcp_switch(ev.value)); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_BARO_MODE_SELECTED:
            efis.set_barometric_mode(barometric_mode(ev.value)); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_BARO_FMT_SELECTED:
            efis.set_barometric_format(barometric_format(ev.value)); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_ND_MODE_SELECTED:
            efis.set_nd_mode_switch(nd_mode_switch(ev.value)); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_ND_RANGE_SELECTED:
            efis.set_nd_range_switch(nd_range_switch(ev.value)); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_ND_NAV1_MODE_SELECTED:
            efis.set_nd_nav1_mode_switch(nd_nav_mode_switch(ev.value)); 
            break;
         case cockpit_back::efis_control_panel::EFIS_CTRL_ND_NAV2_MODE_SELECTED:
            efis.set_nd_nav2_mode_switch(nd_nav_mode_switch(ev.value)); 
            break;
      }
   }
}

void
MapDown(cockpit_back& back, cockpit_front& front)
throw (cockpit_back::sync_error)
{
   MapFCUDown(back, front);
   Mapefis_control_panelDown(back, front);
}

void
MapFCUUp(cockpit_back& back, cockpit_front& front)
throw (cockpit_back::sync_error)
{
   auto& fcu_back = back.get_flight_control_unit();
   auto& fcu_front = front.get_flight_control_unit();

   fcu_back.set_speed_display_units(fcu_front.get_speed_display_units());
   fcu_back.set_guidance_display_mode(fcu_front.get_guidance_display_mode());
   fcu_back.set_altitude_display_units(fcu_front.get_altitude_display_units());
   fcu_back.set_switch(FCU_SWITCH_LOC, fcu_front.get_switch(FCU_SWITCH_LOC));
   fcu_back.set_switch(FCU_SWITCH_ATHR, fcu_front.get_switch(FCU_SWITCH_ATHR));
   fcu_back.set_switch(FCU_SWITCH_EXPE, fcu_front.get_switch(FCU_SWITCH_EXPE));
   fcu_back.set_switch(FCU_SWITCH_APPR, fcu_front.get_switch(FCU_SWITCH_APPR));
   fcu_back.set_switch(FCU_SWITCH_AP1, fcu_front.get_switch(FCU_SWITCH_AP1));
   fcu_back.set_switch(FCU_SWITCH_AP2, fcu_front.get_switch(FCU_SWITCH_AP2));
   fcu_back.set_speed_mode(fcu_front.get_speed_mode());
   fcu_back.set_lateral_mode(fcu_front.get_lateral_mode());
   fcu_back.set_vertical_mode(fcu_front.get_vertical_mode());
   fcu_back.set_speed_value(fcu_front.get_speed_value());
   fcu_back.set_mach_value(fcu_front.get_mach_value());
   fcu_back.set_heading_value(fcu_front.get_heading_value());
   fcu_back.set_track_value(fcu_front.get_track_value());
   fcu_back.set_target_altitude(fcu_front.get_target_altitude());
   fcu_back.set_vertical_speed_value(fcu_front.get_vertical_speed_value());
   fcu_back.set_fpa_value(fcu_front.get_fpa_value());
}

void
Mapefis_control_panelUp(cockpit_back& back, cockpit_front& front)
throw (cockpit_back::sync_error)
{
   auto& panel_back = back.get_efis_control_panel();
   auto& panel_front = front.get_efis_control_panel();
   panel_back.set_barometric_mode(panel_front.get_barometric_mode());
   panel_back.set_barometric_format(panel_front.get_barometric_format());
   panel_back.set_ils_button(panel_front.get_ils_button());
   panel_back.set_mcp_switch(MCP_CONSTRAINT, 
         panel_front.get_mcp_switch(MCP_CONSTRAINT));
   panel_back.set_mcp_switch(MCP_WAYPOINT, 
         panel_front.get_mcp_switch(MCP_WAYPOINT));
   panel_back.set_mcp_switch(MCP_VORD, 
         panel_front.get_mcp_switch(MCP_VORD));
   panel_back.set_mcp_switch(MCP_NDB, 
         panel_front.get_mcp_switch(MCP_NDB));
   panel_back.set_mcp_switch(MCP_AIRPORT, 
         panel_front.get_mcp_switch(MCP_AIRPORT));
   panel_back.set_nd_mode_switch(panel_front.get_nd_mode_switch());
   panel_back.set_nd_range_switch(panel_front.get_nd_range_switch());
   panel_back.set_nd_nav1_mode_switch(panel_front.get_nd_nav1_mode_switch());
   panel_back.set_nd_nav2_mode_switch(panel_front.get_nd_nav2_mode_switch());
}

void
MapUp(cockpit_back& back, cockpit_front& front)
throw (cockpit_back::sync_error)
{
   MapFCUUp(back, front);
   Mapefis_control_panelUp(back, front);
}

void
Map(cockpit_back& back, cockpit_front& front)
throw (cockpit_back::sync_error)
{
   back.sync_down();
   MapDown(back, front);
   MapUp(back, front);
   back.sync_up();
}

}; // anonymous namespace

void
cockpit_front::map(cockpit_back& back) 
{ Map(back, *this); }

void
cockpit_back::map(cockpit_front& front)
throw (sync_error)
{ Map(*this, front); }

}; // namespace oac
