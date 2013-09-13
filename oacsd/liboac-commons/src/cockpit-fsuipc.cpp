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

#include <functional>

#include <Boost/format.hpp>

#include "cockpit.h"
#include "cockpit-fsuipc.h"
#include "fsuipc.h"
#include "logging.h"

namespace oac {

namespace {

typedef double_buffer<shifted_buffer<linear_buffer>> buffer_type;

typedef std::shared_ptr<buffer_type> buffer_ptr;

template <DWORD offset, typename Data, typename event_type>
inline void import_nullary_event(
      buffer_type& buff,
      std::list<cockpit_back::event<event_type>>& events,
      event_type ev_type,
      DWORD event_value = 0)
{
   auto push = buffer::read_as<Data>(buff, offset);
   if (push)
   {
      cockpit_back::event<event_type> ev = { ev_type, event_value };
      events.push_back(ev);
      buffer::write_as(buff, offset, Data(0));
   }
}

template <DWORD offset, typename Data, typename event_type>
inline static void ImportUnaryEvent(
      buffer_type& buff,
      std::list<cockpit_back::event<event_type>>& events,
      event_type ev_type)
{
   if (buff.is_modified_as<Data>(offset))
   {
      cockpit_back::event<event_type> ev =
      { 
         ev_type,
         buffer::read_as<Data>(buff, offset)
      };
      events.push_back(ev);
   }
}

class flight_control_unit_back : public cockpit_back::flight_control_unit {
public:

   inline flight_control_unit_back(const buffer_ptr& buffer) :
      _buffer(buffer)
   {}

   virtual void poll_events(event_list& events)
   {
      import_nullary_event<0x5620, BYTE, event_type>(
            *_buffer, events, FCU_SPD_BTN_PRESSED);
      import_nullary_event<0x5621, BYTE, event_type>(
            *_buffer, events, FCU_GUI_MODE_BTN_PRESSED);
      import_nullary_event<0x5622, BYTE, event_type>(
            *_buffer, events, FCU_ALT_UNITS_BTN_PRESSED);
      import_nullary_event<0x5608, BYTE, event_type>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_LOC);
      import_nullary_event<0x560B, BYTE, event_type>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_ATHR);
      import_nullary_event<0x560C, BYTE, event_type>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_EXPE);
      import_nullary_event<0x560D, BYTE, event_type>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_APPR);
      import_nullary_event<0x5609, BYTE, event_type>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_AP1);
      import_nullary_event<0x560A, BYTE, event_type>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_AP2);
      import_nullary_event<0x5627, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_SPD);
      import_nullary_event<0x5628, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_SPD);
      import_nullary_event<0x5629, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_HDG);
      import_nullary_event<0x562A, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_HDG);
      import_nullary_event<0x562B, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_ALT);
      import_nullary_event<0x562C, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_ALT);
      import_nullary_event<0x562D, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_VS);
      import_nullary_event<0x562E, BYTE, event_type>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_VS);
      ImportUnaryEvent<0x5638, DWORD, event_type>(
            *_buffer, events, FCU_SPD_VALUE_CHANGED);
      ImportUnaryEvent<0x563C, DWORD, event_type>(
            *_buffer, events, FCU_MACH_VALUE_CHANGED);
      ImportUnaryEvent<0x5640, DWORD, event_type>(
            *_buffer, events, FCU_HDG_VALUE_CHANGED);
      ImportUnaryEvent<0x5644, DWORD, event_type>(
            *_buffer, events, FCU_TRACK_VALUE_CHANGED);
      ImportUnaryEvent<0x5648, DWORD, event_type>(
            *_buffer, events, FCU_ALT_VALUE_CHANGED);
      ImportUnaryEvent<0x564C, DWORD, event_type>(
            *_buffer, events, FCU_VS_VALUE_CHANGED);
      ImportUnaryEvent<0x5654, DWORD, event_type>(
            *_buffer, events, FCU_FPA_VALUE_CHANGED);
   }

   virtual void set_speed_display_units(speed_units units)
   { buffer::write_as(*_buffer, 0x562F, BYTE(units)); }

   virtual void set_guidance_display_mode(guidance_display_mode mode)
   { buffer::write_as(*_buffer, 0x5630, BYTE(mode)); }
   
   virtual void set_altitude_display_units(altitude_units units)
   { buffer::write_as(*_buffer, 0x5631, BYTE(units)); }
   
   virtual void set_switch(fcu_switch sw, binary_switch value)
   {
      switch (sw)
      {
         case FCU_SWITCH_LOC:
            buffer::write_as(*_buffer, 0x5614, BYTE(value)); break;
         case FCU_SWITCH_ATHR:
            buffer::write_as(*_buffer, 0x5617, BYTE(value)); break;
         case FCU_SWITCH_EXPE:
            buffer::write_as(*_buffer, 0x5618, BYTE(value)); break;
         case FCU_SWITCH_APPR:
            buffer::write_as(*_buffer, 0x5619, BYTE(value)); break;
         case FCU_SWITCH_AP1:
            buffer::write_as(*_buffer, 0x5615, BYTE(value)); break;
         case FCU_SWITCH_AP2:
            buffer::write_as(*_buffer, 0x5616, BYTE(value)); break;
      }
   }
   
   virtual void set_speed_mode(fcu_management_mode mode)
   { buffer::write_as(*_buffer, 0x5632, BYTE(mode)); }
   
   virtual void set_lateral_mode(fcu_management_mode mode)
   { buffer::write_as(*_buffer, 0x5633, BYTE(mode)); }
   
   virtual void set_vertical_mode(fcu_management_mode mode)
   { buffer::write_as(*_buffer, 0x5634, BYTE(mode)); }
   
   virtual void set_speed_value(knots value)
   { buffer::write_as(*_buffer, 0x5638, DWORD(value)); }

   virtual void set_mach_value(mach100 value)
   { buffer::write_as(*_buffer, 0x563C, DWORD(value)); }

   virtual void set_heading_value(degrees value)
   { buffer::write_as(*_buffer, 0x5640, DWORD(value)); }

   virtual void set_track_value(degrees value)
   { buffer::write_as(*_buffer, 0x5644, DWORD(value)); }
   
   virtual void set_target_altitude(feet value)
   { buffer::write_as(*_buffer, 0x5648, DWORD(value)); }
   
   virtual void set_vertical_speed_value(feet_per_min value)
   { buffer::write_as(*_buffer, 0x564C, DWORD(value)); }
   
   virtual void set_fpa_value(degrees100 value)
   { buffer::write_as(*_buffer, 0x5650, DWORD(value)); }

private:

   buffer_ptr _buffer;
};

class efis_control_panel_back : public cockpit_back::efis_control_panel {
public:

   inline efis_control_panel_back(const buffer_ptr& buffer) :
      _buffer(buffer)
   {}

   virtual void poll_events(event_list& events)
   {
      import_nullary_event<0x5601, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_FD_BTN_PRESSED);
      import_nullary_event<0x5602, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_ILS_BTN_PRESSED);
      import_nullary_event<0x5603, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_CONSTRAINT);
      import_nullary_event<0x5604, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_WAYPOINT);
      import_nullary_event<0x5605, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_VORD);
      import_nullary_event<0x5606, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_NDB);
      import_nullary_event<0x5607, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_AIRPORT);
      import_nullary_event<0x561A, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_BARO_FMT_SELECTED, BARO_FMT_IN_HG);
      import_nullary_event<0x561B, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_BARO_FMT_SELECTED, BARO_FMT_H_PA);
      import_nullary_event<0x561D, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_BARO_MODE_SELECTED, BARO_SELECTED);
      import_nullary_event<0x561E, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_BARO_MODE_SELECTED, BARO_STANDARD);
      ImportUnaryEvent<0x561F, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_BARO_MODE_SELECTED);
      ImportUnaryEvent<0x5623, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_ND_MODE_SELECTED);
      ImportUnaryEvent<0x5624, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_ND_RANGE_SELECTED);
      ImportUnaryEvent<0x5625, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_ND_NAV1_MODE_SELECTED);
      ImportUnaryEvent<0x5626, BYTE, event_type>(
            *_buffer, events, EFIS_CTRL_ND_NAV2_MODE_SELECTED);
   }

   virtual void set_barometric_mode(barometric_mode mode)
   { buffer::write_as(*_buffer, 0x561F, BYTE(mode)); }
   
   virtual void set_barometric_format(barometric_format fmt)
   { buffer::write_as(*_buffer, 0x561C, BYTE(fmt)); }
   
   virtual void set_ils_button(binary_switch btn)
   { buffer::write_as(*_buffer, 0x560E, BYTE(btn)); }
   
   virtual void set_mcp_switch(mcp_switch sw, binary_switch value)
   { 
      switch (sw)
      {
         case MCP_CONSTRAINT:
            buffer::write_as(*_buffer, 0x560F, BYTE(value)); break;
         case MCP_WAYPOINT:
            buffer::write_as(*_buffer, 0x5610, BYTE(value)); break;
         case MCP_VORD:
            buffer::write_as(*_buffer, 0x5611, BYTE(value)); break;
         case MCP_NDB:
            buffer::write_as(*_buffer, 0x5612, BYTE(value)); break;
         case MCP_AIRPORT:
            buffer::write_as(*_buffer, 0x5613, BYTE(value)); break;
      }
   }
   
   virtual void set_nd_mode_switch(nd_mode_switch mode)
   { buffer::write_as(*_buffer, 0x5623, BYTE(mode)); }
   
   virtual void set_nd_range_switch(nd_range_switch range)
   { buffer::write_as(*_buffer, 0x5624, BYTE(range)); }
   
   virtual void set_nd_nav1_mode_switch(nd_nav_mode_switch value)
   { buffer::write_as(*_buffer, 0x5625, BYTE(value)); }
   
   virtual void set_nd_nav2_mode_switch(nd_nav_mode_switch value)
   { buffer::write_as(*_buffer, 0x5626, BYTE(value)); }

private:

   buffer_ptr _buffer;
};

} // anonymous namespace

void
fsuipc_cockpit_back::sync_up()
throw (sync_error)
{ 
   if (!_buffer)
      return;
   if (!_fsuipc)
      this->init_fsuipc();
   buffer::write_as(*_buffer, 0x5600, BYTE(1));
   _fsuipc->copy(*_buffer, 0x5600, 0x5600, 512);
}

void
fsuipc_cockpit_back::sync_down()
throw (sync_error)
{
   try 
   {
      if (!_fsuipc)
         this->init_fsuipc();
      if (!_buffer) 
      {
         this->init_buffer();
         _buffer->copy(*_fsuipc, 0x5600, 0x5600, 512);
      }
      _buffer->swap();
      _buffer->copy(*_fsuipc, 0x5600, 0x5600, 512);
   } 
   catch (oac::exception& e)
   {
      _fsuipc.reset();
      OAC_THROW_EXCEPTION(sync_error().with_cause(e));
   } 
}

cockpit_back::flight_control_unit&
fsuipc_cockpit_back::get_flight_control_unit()
throw (sync_error)
{
   if (!_fcu)
      this->init_fcu();
   return *_fcu;
}

cockpit_back::efis_control_panel&
fsuipc_cockpit_back::get_efis_control_panel()
throw (sync_error)
{
   if (!_efis_ctrl_panel)
      this->init_efis_control_panel();
   return *_efis_ctrl_panel;
}

void
fsuipc_cockpit_back::init_buffer()
{
   auto fb_fact = std::make_shared<linear_buffer::factory>(512);
   auto sh_fact = std::make_shared<shifted_buffer<linear_buffer>::factory>(
            fb_fact, 0x5600);
   auto do_fact =
         std::make_shared<double_buffer<shifted_buffer<linear_buffer>>::factory>(
               sh_fact);
   _buffer = buffer_ptr(do_fact->create_buffer());
}

void
fsuipc_cockpit_back::init_fsuipc()
throw (sync_error)
{
   try 
   {
      _fsuipc = local_fsuipc_ptr(_fsuipc_fact->create_fsuipc());
   }
   catch (fsuipc_error& e)
   {
      OAC_THROW_EXCEPTION(sync_error().with_cause(e));
   }
}

void
fsuipc_cockpit_back::init_fcu()
throw (sync_error)
{
   if (!this->is_sync())
      OAC_THROW_EXCEPTION(sync_error());
   _fcu = std::make_shared<flight_control_unit_back>(_buffer);
}

void
fsuipc_cockpit_back::init_efis_control_panel()
throw (sync_error)
{
   if (!this->is_sync())
      OAC_THROW_EXCEPTION(sync_error());
   _efis_ctrl_panel = std::make_shared<efis_control_panel_back>(_buffer);
}

bool
fsuipc_cockpit_back::is_sync() const
{ return _buffer != nullptr; }

} // namespace oac
