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

#include "wilco-efis.h"

namespace oac { namespace we {

efis_control_panel_impl::efis_control_panel_impl(
      const dll_info& dll_info, HINSTANCE dll_instance) :
   dll_inspector(dll_info, dll_instance), _fsuipc(new local_fsuipc())
{}

barometric_mode
efis_control_panel_impl::get_barometric_mode() const
{ return barometric_mode(this->get_data_object<DWORD>(VADDR_BARO_STD)); }

void
efis_control_panel_impl::set_barometric_mode(barometric_mode mode)
{
   switch (mode)
   {
      case BARO_SELECTED:
         this->send_command(CMD_EFIS_CTRL_PUSH_BARO_KNOB);
         break;
      case BARO_STANDARD:
         this->send_command(CMD_EFIS_CTRL_PULL_BARO_KNOB);
         break;
   }
}

barometric_format
efis_control_panel_impl::get_barometric_format() const
{ return barometric_format(this->get_data_object<DWORD>(VADDR_BARO_FORMAT)); }

void
efis_control_panel_impl::set_barometric_format(barometric_format fmt)
{
   switch (fmt)
   {
      case BARO_FMT_IN_HG:
         this->send_command(CMD_EFIS_CTRL_SET_INHG_BARO_KNOB);
         break;
      case BARO_FMT_H_PA:
         this->send_command(CMD_EFIS_CTRL_SET_HPA_BARO_KNOB);
         break;
   }
}

binary_switch
efis_control_panel_impl::get_fd_button() const
{
   return binary_switch(buffer::read_as<DWORD>(*_fsuipc, 0x2EE0));
}

void
efis_control_panel_impl::push_fd_button()
{
   buffer::write_as<DWORD>(
            *_fsuipc, 0x2EE0, this->invert(this->get_fd_button()));
}

binary_switch
efis_control_panel_impl::get_ils_button() const
{ return binary_switch(this->get_data_object<DWORD>(VADDR_ILS_SWITCH)); }

void
efis_control_panel_impl::push_ils_button()
{
   this->send_command(CMD_EFIS_CTRL_PRESS_ILS_BTN);
}

binary_switch
efis_control_panel_impl::get_mcp_switch(mcp_switch sw) const
{
   static virtual_address_key addresses[] =
   {
      VADDR_MCP_CONSTRAINT,
      VADDR_MCP_WAYPOINT,
      VADDR_MCP_VORD,
      VADDR_MCP_NDB,
      VADDR_MCP_AIRPORT,
   };
   return binary_switch(this->get_data_object<DWORD>(addresses[sw]));
}

void
efis_control_panel_impl::push_mcp_switch(mcp_switch sw)
{
   switch (sw)
   {
      case MCP_CONSTRAINT:
         this->send_command(CMD_EFIS_CTRL_PRESS_CSTR_BTN);
         break;
      case MCP_WAYPOINT:
         this->send_command(CMD_EFIS_CTRL_PRESS_WPT_BTN);
         break;
      case MCP_VORD:
         this->send_command(CMD_EFIS_CTRL_PRESS_VORD_BTN);
         break;
      case MCP_NDB:
         this->send_command(CMD_EFIS_CTRL_PRESS_NDB_BTN);
         break;
      case MCP_AIRPORT:
         this->send_command(CMD_EFIS_CTRL_PRESS_ARPT_BTN);
         break;
   }
}

nd_mode_switch
efis_control_panel_impl::get_nd_mode_switch() const
{ return nd_mode_switch(this->get_data_object<DWORD>(VADDR_ND_MODE)); }

void
efis_control_panel_impl::set_nd_mode_switch(nd_mode_switch mode)
{ this->send_command<nd_mode_switch>(CMD_EFIS_CTRL_SET_ND_MODE, mode); }

nd_range_switch
efis_control_panel_impl::get_nd_range_switch() const
{ return nd_range_switch(this->get_data_object<DWORD>(VADDR_ND_RANGE)); }

void
efis_control_panel_impl::set_nd_range_switch(nd_range_switch range)
{ this->send_command<nd_range_switch>(CMD_EFIS_CTRL_SET_ND_RANGE, range); }

nd_nav_mode_switch
efis_control_panel_impl::get_nd_nav1_mode_switch() const
{ return nd_nav_mode_switch(this->get_data_object<DWORD>(VADDR_MCP_NAV_LEFT)); }

void
efis_control_panel_impl::set_nd_nav1_mode_switch(nd_nav_mode_switch value)
{
   switch (value)
   {
      case ND_NAV_ADF:
         this->send_command(CMD_EFIS_CTRL_LEFT_NAV_SW_ADF);
         break;
      case ND_NAV_OFF:
         this->send_command(CMD_EFIS_CTRL_LEFT_NAV_SW_OFF);
         break;
      case ND_NAV_VOR:
         this->send_command(CMD_EFIS_CTRL_LEFT_NAV_SW_VOR);
         break;
   }
}

nd_nav_mode_switch
efis_control_panel_impl::get_nd_nav2_mode_switch() const
{ return nd_nav_mode_switch(this->get_data_object<DWORD>(VADDR_MCP_NAV_RIGHT)); }

void
efis_control_panel_impl::set_nd_nav2_mode_switch(nd_nav_mode_switch value)
{
   switch (value)
   {
      case ND_NAV_ADF:
         this->send_command(CMD_EFIS_CTRL_RIGHT_NAV_SW_ADF);
         break;
      case ND_NAV_OFF:
         this->send_command(CMD_EFIS_CTRL_RIGHT_NAV_SW_OFF);
         break;
      case ND_NAV_VOR:
         this->send_command(CMD_EFIS_CTRL_RIGHT_NAV_SW_VOR);
         break;
   }
}

}} // namespace oac
