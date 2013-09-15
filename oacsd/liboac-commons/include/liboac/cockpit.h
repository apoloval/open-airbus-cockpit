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

#ifndef OAC_COCKPIT_H
#define OAC_COCKPIT_H

#include <list>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "exception.h"

namespace oac {

enum altitude_units {
   ALT_FEET,
   ALT_METERS,
};

enum speed_units {
   SPEED_KNOTS,
   SPEED_MACH,
};

typedef UINT32 knots;
typedef UINT32 mach100;
typedef UINT32 degrees;
typedef UINT32 degrees100;
typedef UINT32 feet;
typedef INT32 feet_per_min;

enum binary_switch
{
   SWITCHED_OFF = 0,
   SWITCHED_ON = 1,
};

enum guidance_display_mode
{
   GUI_MOD_HDG_VS = 0,
   GUI_MOD_TRK_FPA = 1,
};

enum fcu_management_mode
{
   FCU_MNGT_SELECTED = 0,
   FCU_MNGT_MANAGED = 1,
};

enum fcu_switch {
   FCU_SWITCH_LOC,
   FCU_SWITCH_ATHR,
   FCU_SWITCH_EXPE,
   FCU_SWITCH_APPR,
   FCU_SWITCH_AP1,
   FCU_SWITCH_AP2,      
};

enum fcu_knob {
   FCU_KNOB_SPD,
   FCU_KNOB_HDG,
   FCU_KNOB_ALT,
   FCU_KNOB_VS,
};

enum mcp_switch
{
   MCP_CONSTRAINT,
   MCP_WAYPOINT,
   MCP_VORD,
   MCP_NDB,
   MCP_AIRPORT,
};

enum nd_mode_switch {
   ND_MOD_ILS,
   ND_MOD_VOR,
   ND_MOD_NAV,
   ND_MOD_ARC,
   ND_MOD_PLAN,
};

enum nd_range_switch {
   ND_RNG_10,
   ND_RNG_20,
   ND_RNG_40,
   ND_RNG_80,
   ND_RNG_160,
   ND_RNG_320,
};

enum nd_nav_mode_switch {
   ND_NAV_ADF,
   ND_NAV_OFF,
   ND_NAV_VOR,
};

enum barometric_mode
{
   BARO_SELECTED,
   BARO_STANDARD,
};

enum barometric_format
{
   BARO_FMT_IN_HG,
   BARO_FMT_H_PA,
};

class cockpit_back;

/**
 * Cockpit front class. This class provides an abstraction for the cockpit
 * front side. That means the capabilities that make the cockpit usable
 * for a pilot (e.g., push a button, check wheter a light is on...). In the 
 * counterpart, there is a cockpit back side, which provides the opposite 
 * capabilities (e.g., check whether a button was pushed, turn on a light...).
 */
class cockpit_front {
public:

   class flight_control_unit {
   public:
      inline virtual ~flight_control_unit() {}
      virtual speed_units get_speed_display_units() const = 0;
      virtual void push_speed_units_button() = 0;
      virtual guidance_display_mode get_guidance_display_mode() const = 0;
      virtual void push_guidance_display_mode() = 0;
      virtual altitude_units get_altitude_display_units() const = 0;
      virtual void push_altitude_units_button() = 0;
      virtual binary_switch get_switch(fcu_switch sw) const = 0;
      virtual void push_switch(fcu_switch sw) = 0;
      virtual fcu_management_mode get_speed_mode() const = 0;
      virtual fcu_management_mode get_lateral_mode() const = 0;
      virtual fcu_management_mode get_vertical_mode() const = 0;
      virtual knots get_speed_value() const = 0;
      virtual void set_speed_value(knots value) = 0;
      virtual mach100 get_mach_value() const = 0;
      virtual void set_mach_value(mach100 value) = 0;
      virtual degrees get_heading_value() const = 0;
      virtual void set_heading_value(degrees value) = 0;
      virtual degrees get_track_value() const = 0;
      virtual void set_track_value(degrees value) = 0;
      virtual feet get_target_altitude() const = 0;
      virtual void set_target_altitude(feet value) = 0;
      virtual feet_per_min get_vertical_speed_value() const = 0;
      virtual void set_vertical_speed_value(feet_per_min value) = 0;
      virtual degrees100 get_fpa_value() const = 0;
      virtual void set_fpa_value(degrees100 value) = 0;
      virtual void push_knob(fcu_knob knob) = 0;
      virtual void pull_knob(fcu_knob knob) = 0;
   };

   typedef std::shared_ptr<flight_control_unit> flight_control_unit_ptr;

   class efis_control_panel {
   public:
      inline virtual ~efis_control_panel() {}
      virtual barometric_mode get_barometric_mode() const = 0;
      virtual void set_barometric_mode(barometric_mode mode) = 0;
      virtual barometric_format get_barometric_format() const = 0;
      virtual void set_barometric_format(barometric_format fmt) = 0;
      virtual binary_switch get_fd_button() const = 0;
      virtual void push_fd_button() = 0;
      virtual binary_switch get_ils_button() const = 0;
      virtual void push_ils_button() = 0;
      virtual binary_switch get_mcp_switch(mcp_switch sw) const = 0;
      virtual void push_mcp_switch(mcp_switch sw) = 0;
      virtual nd_mode_switch get_nd_mode_switch() const = 0;
      virtual void set_nd_mode_switch(nd_mode_switch mode) = 0;
      virtual nd_range_switch get_nd_range_switch() const = 0;
      virtual void set_nd_range_switch(nd_range_switch range) = 0;
      virtual nd_nav_mode_switch get_nd_nav1_mode_switch() const = 0;
      virtual void set_nd_nav1_mode_switch(nd_nav_mode_switch value) = 0;
      virtual nd_nav_mode_switch get_nd_nav2_mode_switch() const = 0;
      virtual void set_nd_nav2_mode_switch(nd_nav_mode_switch value) = 0;
   };

   typedef std::shared_ptr<efis_control_panel> efis_control_panel_ptr;

   inline virtual ~cockpit_front() {}

   void map(cockpit_back& back);

   virtual const flight_control_unit& get_flight_control_unit() const = 0;
   virtual flight_control_unit& get_flight_control_unit() = 0;

   virtual const efis_control_panel& get_efis_control_panel() const = 0;
   virtual efis_control_panel& get_efis_control_panel() = 0;
};

/**
 * Cockpit back class. This class provides an abstraction for the cockpit
 * back side. That means the capabilities that make it possible to govern
 * the cockpit (e.g., check whether a button was pressed, turn on a light...).
 * In the counterpart, there is a cockpit front side, which provides the
 * opposite capabilities (e.g., press a button, check whether a light is on...).
 */
class cockpit_back {
public:

   OAC_DECL_EXCEPTION(
         sync_error,
         oac::exception,
         "synchronization error occurred in cockpit back");

   template <typename T>
   struct event
   {
      T type;
      DWORD value;
   };

   inline virtual ~cockpit_back() {}

   class flight_control_unit {
   public:

      enum event_type
      {
         FCU_SPD_BTN_PRESSED,
         FCU_GUI_MODE_BTN_PRESSED,
         FCU_ALT_UNITS_BTN_PRESSED,
         FCU_SW_PRESSED,
         FCU_KNOB_PRESSED,
         FCU_KNOB_PULLED,
         FCU_SPD_VALUE_CHANGED,
         FCU_MACH_VALUE_CHANGED,
         FCU_HDG_VALUE_CHANGED,
         FCU_TRACK_VALUE_CHANGED,
         FCU_ALT_VALUE_CHANGED,
         FCU_VS_VALUE_CHANGED,
         FCU_FPA_VALUE_CHANGED,
      };

      typedef std::list<event<event_type>> event_list;

      inline virtual ~flight_control_unit() {}
      virtual void poll_events(event_list& events) = 0;
      virtual void set_speed_display_units(speed_units units) = 0;
      virtual void set_guidance_display_mode(guidance_display_mode mode) = 0;
      virtual void set_altitude_display_units(altitude_units units) = 0;
      virtual void set_switch(fcu_switch sw, binary_switch value) = 0;
      virtual void set_speed_mode(fcu_management_mode mode) = 0;
      virtual void set_lateral_mode(fcu_management_mode mode) = 0;
      virtual void set_vertical_mode(fcu_management_mode mode) = 0;
      virtual void set_speed_value(knots value) = 0;
      virtual void set_mach_value(mach100 value) = 0;
      virtual void set_heading_value(degrees value) = 0;
      virtual void set_track_value(degrees value) = 0;
      virtual void set_target_altitude(feet value) = 0;
      virtual void set_vertical_speed_value(feet_per_min value) = 0;
      virtual void set_fpa_value(degrees100 value) = 0;
   };

   typedef std::shared_ptr<flight_control_unit> flight_control_unit_ptr;

   class efis_control_panel {
   public:

      enum event_type
      {
         EFIS_CTRL_FD_BTN_PRESSED,
         EFIS_CTRL_ILS_BTN_PRESSED,
         EFIS_CTRL_MCP_SW_PRESSED,
         EFIS_CTRL_BARO_MODE_SELECTED,
         EFIS_CTRL_BARO_FMT_SELECTED,
         EFIS_CTRL_ND_MODE_SELECTED,
         EFIS_CTRL_ND_RANGE_SELECTED,
         EFIS_CTRL_ND_NAV1_MODE_SELECTED,
         EFIS_CTRL_ND_NAV2_MODE_SELECTED,
      };

      typedef std::list<event<event_type>> event_list;

      inline virtual ~efis_control_panel() {}
      virtual void poll_events(event_list& events) = 0;
      virtual void set_barometric_mode(barometric_mode mode) = 0;
      virtual void set_barometric_format(barometric_format fmt) = 0;
      virtual void set_ils_button(binary_switch btn) = 0;
      virtual void set_mcp_switch(mcp_switch sw, binary_switch value) = 0;
      virtual void set_nd_mode_switch(nd_mode_switch mode) = 0;
      virtual void set_nd_range_switch(nd_range_switch range) = 0;
      virtual void set_nd_nav1_mode_switch(nd_nav_mode_switch value) = 0;
      virtual void set_nd_nav2_mode_switch(nd_nav_mode_switch value) = 0;
   };

   typedef std::shared_ptr<efis_control_panel> efis_control_panel_ptr;

   void map(cockpit_front& front) throw (sync_error);

   /**
    * Move the status of this cockpit back to its counterpart. 
    */
   virtual void sync_up() throw (sync_error) = 0;

   /**
    * Move the status of the counterpart to this cockpit back.
    */
   virtual void sync_down() throw (sync_error) = 0;

   virtual flight_control_unit& get_flight_control_unit() throw (sync_error) = 0;

   virtual efis_control_panel& get_efis_control_panel() throw (sync_error) = 0;
};

typedef std::shared_ptr<cockpit_back> cockpit_back_ptr;

}; // namespace oac

#endif
