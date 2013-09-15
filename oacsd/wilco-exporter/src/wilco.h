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

#ifndef OAC_WE_WILCO_H
#define OAC_WE_WILCO_H

#pragma warning( disable : 4290 )

#include <cstdint>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <liboac/cockpit.h>
#include <liboac/exception.h>

namespace oac { namespace we {

enum apu_status
{
   APU_OFF = 0,
   APU_ON = 1,
};

enum thrust_levers
{
   THRUST_REVERSE = 0,
   THRUST_IDLE = 1,
   THRUST_AUTOTHROTTLE = 2,
   THRUST_CL = 3,
   THRUST_FLEX_MCT = 4,
   THRUST_TOGA = 6,
   THRUST_UNKNOWN = 0xffffffff,
};

enum tcas
{
   tcas_THRT = 0,
   tcas_ALL = 1,
   tcas_ABOVE = 2,
   tcas_BELLOW = 3,
};

enum irs_status
{
   IRS_NOT_ALIGNED,
   IRS_ALIGNING,
   IRS_ALIGNED,
};

enum engine_mode
{
   ENG_MOD_CRANK = 0,
   ENG_MOD_NORM = 1,
   ENG_MOD_IGNITION_START = 2,
};

enum auto_thrust
{
   ATHR_DISENGAGED = 0,
   ATHR_ENGAGED = 2,
};

enum autopilot
{
   AP_NONE = 0,
   AP_1 = 1,
   AP_2 = 2,
   AP_BOTH = 3,
};

enum speed_mode
{
   SPD_MOD_NONE = 0,
   SPD_MOD_SPEED = 1,
   SPD_MOD_UNKNOWN_2 = 2,
   SPD_MOD_THR_CLB = 3,
   SPD_MOD_THR_IDLE = 4,
};

enum vertical_mode
{
   VER_MOD_NONE = 0,
   VER_MOD_GS = 1,
   VER_MOD_GS_ENGAGING = 2,
   VER_MOD_ALT = 3,
   VER_MOD_ALT_ENGAGING = 4,
   VER_MOD_VS_FPA = 5,
   VER_MOD_CLB = 6,
   VER_MOD_DESC = 7, // not sure of this
   VER_MOD_OP_CLB = 8,
   VER_MOD_OP_DESC = 9,
   VER_MOD_SRS = 10,
};

enum lateral_mode
{
   LAT_MOD_NONE = 0,
   LAT_MOD_RWY = 1,
   LAT_MOD_UNKNOWN_2 = 2,
   LAT_MOD_HDG = 3,
   LAT_MOD_TRK = 4,
   LAT_MOD_NAV = 5,
   LAT_MOD_LOC_ENGAGING = 6,
   LAT_MOD_LOC = 7,
};

enum strobe_light_switch
{
   STROBE_OFF = 0,
   STROBE_AUTO = 1,
   STROBE_ON = 2,
};

enum cabin_sign_switch
{
   SIGN_OFF = 0,
   SIGN_AUTO = 1,
   SIGN_ON = 2,
};

enum adirs_display_mode_switch
{
   ADIRS_TEST = 0,
   ADIRS_TK_GS = 1,
   ADIRS_PPOS = 2,
   ADIRS_DATA = 3,
   ADIRS_WIND = 4,
   ADIRS_HDG = 5,
   ADIRS_STS = 6,
};

enum adirs_display_sys_switch
{
   ADIRS_OFF = 0,
   ADIRS_1 = 1,
   ADIRS_3 = 2,
   ADIRS_2 = 3,
};

enum cockpit_temp_switch
{
   TEMP_MINUS_3 = 0,
   TEMP_MINUS_2 = 1,
   TEMP_MINUS_1 = 2,
   TEMP_ZERO = 3,
   TEMP_PLUS_1 = 4,
   TEMP_PLUS_2 = 5,
   TEMP_PLUS_3 = 6,
};

enum packflow_switch
{
   PACK_FLOW_LOW = 0,
   PACK_FLOW_NORM = 1,
   PACK_FLOW_HIGH = 2,
};

enum aircond_xbleed_switch
{
   AC_XBLEED_SHUTDOWN = 0,
   AC_XBLEED_AUTO = 1,
   AC_XBLEED_OPEN = 2,
};

enum probe_window_head_switch
{
   PROBE_WINHEAT_AUTO = 0,
   PROBE_WINHEAT_ON = 1,
};

enum gpu_switch
{
   GPU_OFF,
   GPU_AVAILABLE,
   GPU_ON,
};

enum apu_master_switch
{
   APU_MASTER_OFF,
   APU_MASTER_ON,
};

enum apu_start_switch
{
   APU_START_OFF,
   APU_START_ON,
   APU_START_AVAILABLE,
};

struct apu_switches
{
   apu_master_switch master;
   apu_start_switch start;
};

enum sd_page_button {
   SD_PAGE_NONE,
   SD_PAGE_ENGINE,
   SD_PAGE_BLEED,
   SD_PAGE_PRESS,
   SD_PAGE_ELEC,
   SD_PAGE_HYD,
   SD_PAGE_FUEL,
   SD_PAGE_APU,
   SD_PAGE_COND,
   SD_PAGE_DOOR,
   SD_PAGE_WHEEL,
   SD_PAGE_FCTL,
   SD_PAGE_UNKNOWN,
   SD_PAGE_STS,
};

enum aircraft_type
{
   A320_CFM,
};

typedef std::string aircraft_title;

struct aircraft {

   OAC_DECL_EXCEPTION_WITH_PARAMS(invalid_title, oac::exception,
      ("invalid aircraft title %s", title),
      (title, aircraft_title));

   const aircraft_type type;
   const aircraft_title title;

   aircraft(const aircraft_type& type);

   aircraft(const aircraft_title& title) throw (invalid_title);
};

class wilco_cockpit : public cockpit_front
{
public:

   typedef std::shared_ptr<wilco_cockpit> ptr_type;

   static wilco_cockpit::ptr_type new_cockpit(
         const aircraft& aircraft);

   inline virtual ~wilco_cockpit() {}

   virtual const aircraft& get_aircraft() const = 0;

   virtual gpu_switch get_gpu_switch() const = 0;

   virtual void get_apu_switches(apu_switches& sw) const = 0;

   virtual sd_page_button get_sd_page_button() const = 0;

   virtual const flight_control_unit& get_flight_control_unit() const = 0;
   virtual flight_control_unit& get_flight_control_unit() = 0;

   virtual const efis_control_panel& get_efis_control_panel() const = 0;
   virtual efis_control_panel& get_efis_control_panel() = 0;

   virtual void debug() const = 0;

protected:

   inline wilco_cockpit() {}
};

typedef wilco_cockpit::ptr_type wilco_cockpit_ptr;

}} // namespace oac

#endif
