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

#ifndef OAC_WE_WILCO_INTERNAL_H
#define OAC_WE_WILCO_INTERNAL_H

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "wilco.h"

namespace oac { namespace we {

typedef std::wstring dll_name;
typedef const char* function_name;
typedef size_t virtual_address;

/**
 * Enumeration of all known virtual addresses in Wilco libraries.
 */
enum virtual_address_key
{
   VADDR_BASE,
   VADDR_FN_PUSH_MCP_CONSTRAINT,
   VADDR_FN_PUSH_MCP_WAYPOINT,
   VADDR_FN_PUSH_MCP_VORD,
   VADDR_FN_PUSH_MCP_NDB,
   VADDR_FN_PUSH_MCP_AIRPORT,
   VADDR_FN_IS_APU_AVAILABLE,
   VADDR_FN_GET_FADEC_MODE,
   VADDR_FN_SEND_COMMAND,
   VADDR_ND_RANGE,
   VADDR_ND_MODE,
   VADDR_MCP_NAV_LEFT,
   VADDR_MCP_NAV_RIGHT,
   VADDR_FADEC,
   VADDR_BARO_FORMAT,
   VADDR_BARO_STD,
   VADDR_FCU_SPD_DISPLAY,
   VADDR_MCP_CONSTRAINT,
   VADDR_MCP_WAYPOINT,
   VADDR_MCP_VORD,
   VADDR_MCP_NDB,
   VADDR_MCP_AIRPORT,
   VADDR_ILS_SWITCH,
   VADDR_FMGC,
   VADDR_FCU,
   VADDR_FBW,
   VADDR_HEAD_PANEL,
   VADDR_GPWS,
   VADDR_PEDESTAL,
   VADDR_APU,
   VADDR_GPU,
   VADDR_NELEMENTS,
};

struct dll_info
{
   const dll_name name;
   const virtual_address virtual_addresses[VADDR_NELEMENTS];

   static const dll_info& for_aircraft(const aircraft& aircraft);
};

struct function_names
{
   static function_name GET_INTERNAL_DATA;
   static function_name GET_EXTENDED_DATA;
   static function_name RESET_INTERNAL_DATA;
   static function_name RESET_FLIGHT;
};

struct wilco_fcu
{
   DWORD target_altitude;                 // 0x00
   UINT32 selected_vertical_speed;        // 0x04
   DWORD selected_track;                  // 0x08
   FLOAT selected_fpa;                    // 0x0C
   DWORD autopilot;                       // 0x10, autopilot
   DWORD unknown_14;                      // 0x14 e.g., 0
   DWORD unknown_18;                      // 0x18 e.g., 0
   DWORD unknown_1c;                      // 0x1C e.g., 0
   DWORD auto_thrust;                     // 0x20, auto_thrust
   DWORD speed_mode;                      // 0x24, speed_mode
   DWORD active_vertical_mode;            // 0x28, vertical_mode
   DWORD armed_vertical_mode;             // 0x2C, vertical_mode
   DWORD active_lateral_mode;             // 0x30, lateral_mode
   DWORD armed_lateral_mode;              // 0x34, lateral_mode
   DWORD hdg_track_display_mode;          // 0x38, FCUDisplayMode
   DWORD vs_fpa_display_mode;             // 0x3C, FCUDisplayMode
   DWORD speed_knob;                      // 0x40, FCUKnob
   DWORD heading_knob;                    // 0x44, FCUKnob
   DWORD metric_altitude;                 // 0x48, binary_switch
   DWORD vertical_speed_knob;             // 0x4C, FCUKnob
   void* unknown_50;                      // 0x50
   DWORD unknown_54;                      // 0x54 e.g., 0
   DWORD unknown_58;                      // 0x58 e.g., 500
   DWORD unknown_5c;                      // 0x5C e.g., 50
   DWORD unknown_60;                      // 0x60 e.g., 9
   DWORD unknown_64;                      // 0x64 e.g., 18
   DWORD unknown_68;                      // 0x68 e.g., 0
   DWORD unknown_6c;                      // 0x6C e.g., 0
   DWORD expedite;                        // 0x70, binary_switch
   DWORD autoland;                        // 0x74, binary_switch when AP engaged
   void* unknown_78;                      // 0x78
   DWORD unknown_7c;                      // 0x7C e.g., 0
   DWORD unknown_80;                      // 0x80 e.g., 1
   DWORD indicated_altitude;              // 0x84
}; // size: 34 (0x22) words, 136 (0x88) bytes

struct wilco_headpanel
{
   BYTE unused_0000[0x550C];              // 0x0000
   DWORD ldgFlap3_1;                      // 0x550C
   DWORD ldgFlap3_2;                      // 0x5510
   DWORD unknown_5514;                    // 0x5514
   DWORD unknown_5518;                    // 0x5518
   DWORD unknown_551C;                    // 0x551C
   DWORD unknown_5520;                    // 0x5520,
   DWORD unknown_5524;                    // 0x5524
   DWORD unknown_5528;                    // 0x5528
   DWORD unknown_552C;                    // 0x552C
   DWORD unknown_5530;                    // 0x5530,
   DWORD master_caution;                  // 0x5534
   DWORD master_warning;                  // 0x5538
   DWORD unknown_553C;                    // 0x553C
   DWORD unknown_5540;                    // 0x5540,
   DWORD unknown_5544;                    // 0x5544
   DWORD ptu;                             // 0x5548
   DWORD yellow_electric_pump;            // 0x554C
   DWORD unknown_5550;                    // 0x5550, A340 electric pump?
   DWORD green_pump;                      // 0x5554
   DWORD blue_pump;                       // 0x5558
   DWORD yellow_pump;                     // 0x555C
   DWORD unknown_5560;                    // 0x5560, A340 additional pump?
   DWORD strobe_light;                    // 0x5564
   DWORD unknown_5568;                    // 0x5568
   DWORD seat_belts_sign;                 // 0x556C
   DWORD no_smoking_sign;                 // 0x5570
   DWORD battery_1;                       // 0x5574
   DWORD battery_2;                       // 0x5578
   DWORD unknown_557C;                    // 0x557C, A340 battery 3?
   DWORD unknown_5580;                    // 0x5580, A340 battery 4?
   DWORD unknown_5584;                    // 0x5584
   DWORD unknown_5588;                    // 0x5588
   DWORD unknown_558C;                    // 0x558C
   DWORD gpu_energy;                      // 0x5590
   DWORD adirs_display_mode;              // 0x5594
   DWORD adirs_display_sys;               // 0x5598
   DWORD pack_flow;                       // 0x559C
   DWORD xbleed;                          // 0x55A0
   DWORD cockpit_temp;                    // 0x55A4
   DWORD fwdCabin_temp;                   // 0x55A8
   DWORD aftCabin_temp;                   // 0x55AC
   DWORD hot_air;                         // 0x55B0
   DWORD pack_1;                          // 0x55B4
   DWORD pack_2;                          // 0x55B8
   DWORD unknown_55BC;                    // 0x55BC
   DWORD unknown_55C0;                    // 0x55C0
   DWORD engine_1_bleed;                  // 0x55C4
   DWORD engine_2_bleed;                  // 0x55C8
   DWORD unknown_55CC;                    // 0x55CC
   DWORD unknown_55D0;                    // 0x55D0
   DWORD unknown_55D4;                    // 0x55D4
   DWORD unknown_55D8;                    // 0x55D8
   DWORD unknown_55DC;                    // 0x55DC
   DWORD probe_window_heat;               // 0x55E0
   DWORD unknown_55E4;                    // 0x55E4
   DWORD master_caution_counter;          // 0x55E8
   DWORD unknown_55EC;                    // 0x55EC
   DWORD unknown_55F0;                    // 0x55F0
   DWORD unknown_55F4;                    // 0x55F4
   DWORD elac_1;                          // 0x55F8
   DWORD elac_2;                          // 0x55FC
   DWORD sec_1;                           // 0x5600
   DWORD sec_2;                           // 0x5604
   DWORD sec_3;                           // 055608
   DWORD fac_1;                           // 0x560C
   DWORD fac_2;                           // 0x5610
   DWORD unknown_5614;                    // 0x5614
   DWORD unknown_5618;                    // 055618
   DWORD unknown_561C;                    // 0x561C
};

struct wilco_gpu_vtable
{
   uint8_t unknown_0000 [0x2C];                             // 0x00
   DWORD (__fastcall *isGpuAvailable)(void* _this);      // 0x2C
};

struct wilco_gpu {
   wilco_gpu_vtable* vtable;
};

struct wilco_apu {
   DWORD master_switch;                   // 0x00
   DWORD unused_04;                       // 0x04
   DWORD unused_08;                       // 0x08
   DWORD unused_0c;                       // 0x0C
   DWORD unused_10[11];                   // 0x10
   DWORD electric_gen;                    // 0x3C
   DWORD bleed;                           // 0x40
   DWORD unused_44;                       // 0x44

   bool isAvailable() { return !unused_08 && unused_0c == 0x40590000; }
}; // 0x48 bytes


struct wilco_fadec{};

struct wilco_fmgc
{
   BYTE unused_00000[0x1F0];              // 0x00000
   DWORD cost_index;                      // 0x001F0
   BYTE unused_001F4[0x7F230];            // 0x001F4
   double perf_factor;                    // 0x7F420

};

struct wilco_gpws
{
   DWORD sys_switch;                      // 0x00
   DWORD unused_04[6];                    // 0x04
   DWORD terr_switch;                     // 0x1C
   DWORD gs_mode_switch;                  // 0x20
   DWORD flap_mode_switch;                // 0x24
   DWORD ldg_flap3_switch;                // 0x28
};

struct wilco_pedestal
{
   BYTE unused_0000[0x08];                // 0x00
   double ctl_value;                      // 0x08
   BYTE unused_0010[0x18];                // 0x10
   DWORD sd_page_selected;                // 0x28
   DWORD unused_002c;                     // 0x2C
   DWORD sd_page_button;                  // 0x30
};

struct wilco_internal_data
{
   DWORD apu_status;                      // 0x00, apu_status
   DWORD engine_max_pos;                  // 0x04, thrust_levers
   DWORD tcas;                            // 0x08, tcas
   DWORD selected_barometric_mode;        // 0x0C, BarometricMode
   DWORD transition_altitude;             // 0x10
   DWORD irs;                             // 0x14, irs_status
   DWORD selected_engine_mode;            // 0x18, EngineMode
   DWORD seat_belts_sign;                 // 0x1C, CabinSign
   DWORD no_smoking_sign;                 // 0x20, CabinSign
   DWORD unknown_24;                      // 0x24
   DWORD unknown_28;                      // 0x28
   DWORD unknown_2c;                      // 0x2C
   DWORD unknown_30;                      // 0x30
   DWORD master_caution_count;            // 0x34
   DWORD master_warning_count;            // 0x38
};

struct wilco_extended_data
{
   void* unknown_00;                      // 0x00
   void* unknown_04;                      // 0x04
   void* unknown_08;                      // 0x08
   DWORD throttle_red;                    // 0x0C
   DWORD engine_out_acc;                  // 0x10
   DWORD flight_phase;                    // 0x14
   uint8_t unknown_18;                    // 0x18, not written
   uint8_t fcu_hdg_knob;                  // 0x19
   uint8_t ils_switch;                    // 0x1A
   uint8_t unknown_1B;                    // 0x1B, not written
   DWORD fcu_display_mode;                // 0x1C
   void* fmgc;                            // 0x20
   void* mcdu;                            // 0x24
   wilco_fcu* fcu;                        // 0x28
   void* fadec;                           // 0x2C
   wilco_headpanel* head_panel;           // 0x30
   void* fbw;                             // 0x34, 0x160 len
   void* gpws;                            // 0x38
   void* pedestal;                        // 0x3C
   void* gpu;                             // 0x40, 57 words
};

enum wilco_command {
   CMD_PEDESTAL_FLOOD_LGT = 0x01,
   CMD_RESET_FLIGHT = 0x02,
   CMD_MCDU_PRESS_INIT_BTN = 0x03,
   CMD_MCDU_PRESS_DIR_BTN = 0x04,
   CMD_MCDU_PRESS_PERF_BTN = 0x05,
   CMD_MCDU_PRESS_PROG_BTN = 0x06,
   CMD_MCDU_PRESS_DATA_BTN = 0x07,
   CMD_MCDU_PRESS_FPLAN_BTN = 0x08,
   CMD_MCDU_PRESS_RADNAV_BTN = 0x09,
   CMD_MCDU_PRESS_FUELPRED_BTN = 0x0A,
   CMD_MCDU_PRESS_MENU_BTN = 0x0B,
   CMD_MCDU_PRESS_NEXTPAGE_BTN = 0x0C,
   CMD_MCDU_PRESS_UP_BTN = 0x0D,
   CMD_MCDU_PRESS_DOWN_BTN = 0x0E,
   CMD_EFIS_CTRL_INC_ND_MODE = 0x0F,
   CMD_EFIS_CTRL_DEC_ND_MODE = 0x10,
   CMD_EFIS_CTRL_INC_ND_RANGE = 0x11,
   CMD_EFIS_CTRL_DEC_ND_RANGE = 0x12,
   CMD_PRESS_MASTER_WARN = 0x13,
   CMD_PRESS_MASTER_CAUTION = 0x14,
   CMD_PFD_INC_BRIGHT = 0x15,
   CMD_PFD_DEC_BRIGHT = 0x16,
   CMD_LCD_RESET_BRIGHT = 0x17,
   CMD_FCU_INC1_SPD_KNOB = 0x18,
   CMD_FCU_DEC1_SPD_KNOB = 0x19,
   CMD_FCU_INC10_SPD_KNOB = 0x1A,
   CMD_FCU_DEC10_SPD_KNOB = 0x1B,
   CMD_FCU_INC1_HDG_KNOB = 0x1C,
   CMD_FCU_DEC1_HDG_KNOB = 0x1D,
   CMD_FCU_INC10_HDG_KNOB = 0x1E,
   CMD_FCU_DEC10_HDG_KNOB = 0x1F,
   CMD_FCU_INC100_ALT_KNOB = 0x20,
   CMD_FCU_DEC100_ALT_KNOB = 0x21,
   CMD_FCU_INC1000_ALT_KNOB = 0x22,
   CMD_FCU_DEC1000_ALT_KNOB = 0x23,
   CMD_FCU_INC100_VS_KNOB = 0x24,
   CMD_FCU_DEC100_VS_KNOB = 0x25,
   CMD_FCU_INC1000_VS_KNOB = 0x26,
   CMD_FCU_DEC1000_VS_KNOB = 0x27,
   CMD_FCU_PUSH_AP1_BTN = 0x28,
   CMD_FCU_PUSH_AP2_BTN = 0x29,
   CMD_FCU_PUSH_EXPED_BTN = 0x2A,
   CMD_FCU_PUSH_LOC_BTN = 0x2B,
   CMD_FCU_PUSH_APPR_BTN = 0x2C,
   CMD_EFIS_CTRL_ALT_UNITS_BARO_KNOB = 0x2D,
   CMD_EFIS_CTRL_SET_INHG_BARO_KNOB = 0x2E,
   CMD_EFIS_CTRL_SET_HPA_BARO_KNOB = 0x2F,
   CMD_EFIS_CTRL_PULL_BARO_KNOB = 0x30,
   CMD_EFIS_CTRL_PUSH_BARO_KNOB = 0x31,
   CMD_EFIS_CTRL_INC_BARO_KNOB = 0x32,
   CMD_EFIS_CTRL_DEC_BARO_KNOB = 0x33,
   CMD_FCU_PULL_SPD_KNOB = 0x34,
   CMD_FCU_PUSH_SPD_KNOB = 0x35,
   CMD_FCU_PULL_HDG_KNOB = 0x36,
   CMD_FCU_PUSH_HDG_KNOB = 0x37,
   CMD_FCU_PULL_ALT_KNOB = 0x38,
   CMD_FCU_PUSH_ALT_KNOB = 0x39,
   CMD_FCU_PULL_VS_KNOB = 0x3A,
   CMD_FCU_PUSH_VS_KNOB = 0x3B,
   CMD_FCU_PRESS_SPD_MACH_BTN = 0x3C,
   CMD_FCU_PRESS_HDG_TRK_BTN = 0x3D,
   CMD_FCU_PRESS_METRIC_ALT_BTN = 0x3E,
   CMD_FCU_ALT_SET_HUNDRED = 0x3F,
   CMD_FCU_ALT_SET_THOUSAND = 0x40,
   CMD_EFIS_CTRL_PRESS_ARPT_BTN = 0x41,
   CMD_EFIS_CTRL_PRESS_NDB_BTN = 0x42,
   CMD_EFIS_CTRL_PRESS_VORD_BTN = 0x43,
   CMD_EFIS_CTRL_PRESS_WPT_BTN = 0x44,
   CMD_EFIS_CTRL_PRESS_CSTR_BTN = 0x45,
   CMD_EFIS_CTRL_LEFT_NAV_SW_ADF = 0x46,
   CMD_EFIS_CTRL_LEFT_NAV_SW_OFF = 0x47,
   CMD_EFIS_CTRL_LEFT_NAV_SW_VOR = 0x48,
   CMD_EFIS_CTRL_RIGHT_NAV_SW_ADF = 0x49,
   CMD_EFIS_CTRL_RIGHT_NAV_SW_OFF = 0x4A,
   CMD_EFIS_CTRL_RIGHT_NAV_SW_VOR = 0x4B,
   CMD_EFIS_CTRL_PRESS_ILS_BTN = 0x4C,
   CMD_ECAM_CP_PRESS_ENG_BTN = 0x4D,
   CMD_ECAM_CP_PRESS_BLEED_BTN = 0x4E,
   CMD_ECAM_CP_PRESS_PRESS_BTN = 0x4F,
   CMD_ECAM_CP_PRESS_ELEC2_BTN = 0x50,
   CMD_ECAM_CP_PRESS_ELEC_BTN = 0x51,
   CMD_ECAM_CP_PRESS_HYD_BTN = 0x52,
   CMD_ECAM_CP_PRESS_APU_BTN = 0x53,
   CMD_ECAM_CP_PRESS_COND_BTN = 0x54,
   CMD_ECAM_CP_PRESS_DOOR_BTN = 0x55,
   CMD_ECAM_CP_PRESS_WHEEL_BTN = 0x56,
   CMD_ECAM_CP_PRESS_FCTRL_BTN = 0x57,
   CMD_ECAM_CP_PRESS_FUEL_BTN = 0x58,
   CMD_ECAM_CP_PRESS_STS_BTN = 0x59,

   CMD_EFIS_CTRL_SET_ND_MODE = 0x5E,
   CMD_EFIS_CTRL_SET_ND_RANGE = 0x5F,
   CMD_FCU_SET_SPD_VALUE = 0x60,
   CMD_FCU_SET_HDG_VALUE = 0x61,
   CMD_FCU_SET_ALT_VALUE = 0x62,
   CMD_FCU_SET_VS_VALUE = 0x63,
};


typedef DWORD (__fastcall *wilco_get_fadec_mode)(
      wilco_fadec* f, DWORD unused, DWORD fadecn);

typedef double (__fastcall *wilco_get_fadec_tla)(
      wilco_fadec* f, DWORD unused, DWORD fadecn);

typedef DWORD (__fastcall *wilco_is_apu_available)(wilco_apu* apu);

typedef DWORD (__stdcall *wilco_push_mcp_button)(DWORD num1, DWORD num2);

typedef void (*wilco_reset_internal_data)(void);

typedef void (*wilco_reset_flight)(void);

typedef void (__cdecl *wilco_set_nd_mode)(DWORD mode);

typedef void (__cdecl *wilco_send_command)(DWORD cmd, const void* args);


/**
 * Get the function with signature F or throw the exception E if not found.
 */
template <typename F, typename E>
F get_function_or_throw(HINSTANCE inst, const char* func_name)
{
   auto result = (F) GetProcAddress(inst, func_name);
   if (!result)
      BOOST_THROW_EXCEPTION(E() << function_nameInfo(func_name));
   return result;
}

/**
 * Load DLL library for given aircraft. invalid_aircraft_error is thrown if
 * given aircraft is unknown or DLL cannot be resolved for it.
 */
HINSTANCE load_dll_for_aircraft(const aircraft& aircraft)
      throw (wilco_cockpit::invalid_aircraft_error);

/**
 * Free a DLL instance previously loaded.
 */
void free_dll(HINSTANCE lib);

/**
 * Track changes on memory location. This auxiliary function should be used
 * very carefully only for debugging and reverse-engineering purposes. It
 * tracks any change on given memory location. The first time it's called,
 * it initializes an internal buffer and copy the contents of mem into it.
 * On each subsequent call, the previous content is compared with the new
 * one and any difference is reported to the log.
 */
void track_changes_on_memory(void* mem, size_t len);

/**
 * DLL inspector class. This is a support class that may be extended by any
 * other class which has the responsibility to inspect the variables of
 * Wilco DLLs.
 */
class dll_inspector
{
public:

   inline dll_inspector(const dll_info& dll_info, HINSTANCE dll_instance) :
         _dll_info(dll_info), _dll_instance(dll_instance)
   {}

protected:

   /**
    * Get the actual address for the given virtual address.
    */
   void* get_actual_address(virtual_address vaddr) const
   {
      auto offset = vaddr - _dll_info.virtual_addresses[VADDR_BASE];
      auto actualAddress = virtual_address(_dll_instance) + offset;
      return (void*)(actualAddress);
   }

   /**
    * Get the data object located at given virtual address. The virtual
    * address is mapped to an actual address and then casted as a pointer
    * to given DataObject.
    */
   template <typename DataObject>
   DataObject get_data_object(virtual_address_key vaddr_key) const
   {
      auto virtual_address = _dll_info.virtual_addresses[vaddr_key];
      auto data = this->get_actual_address(virtual_address);
      return *(DataObject*) data;
   }

   /**
    * Get the function located at given virtual address. The virtual address
    * is mapped to an actual address and then casted as a pointer to
    * given function.
    */
   template <typename Function>
   Function get_function(virtual_address_key virtual_address_key) const
   {
      auto virtual_address = _dll_info.virtual_addresses[virtual_address_key];
      auto addr = this->get_actual_address(virtual_address);
      return Function(addr);
   }

   /**
    * Set the data object located at given virtual address. The virtual
    * address is mapped to an actual address and then casted as a pointer to
    * given function. The new data passed as argument is assigned to the
    * object located there.
    */
   template <typename DataObject>
   void set_data_object(
         virtual_address_key virtual_addressKey, const DataObject& new_data)
   {
      auto virtual_address = _dll_info.virtual_addresses[virtual_addressKey];
      auto data = (DataObject*) this->get_actual_address(virtual_address);
      *data = new_data;
   }

   /**
    * Toggles a boolean value located at given virtual address. The virtual
    * address is mapped to an actual address and then casted as a pointer to
    * given boolean value, which is then toggled.
    */
   template <typename DataObject>
   void toggle_bool_object(virtual_address_key virtual_addressKey)
   {
      auto virtual_address = _dll_info.virtual_addresses[virtual_addressKey];
      auto data = (DataObject*) this->get_actual_address(virtual_address);
      *data = !(*data);
   }

   /**
    * Send a command to Wilco Airbus using the facilities provided by its
    * DLL. The given value is passed as argument to the command.
    */
   template <typename T>
   inline void send_command(wilco_command cmd, const T& arg)
   {
      auto send_cmd =
            this->get_function<wilco_send_command>(VADDR_FN_SEND_COMMAND);
      send_cmd(cmd, &arg);
   }

   /**
    * Send a command to Wilco Airbus using the facilities provided by its
    * DLL. No argument as passed along the command.
    */
   inline void send_command(wilco_command cmd)
   {
      auto send_cmd =
            this->get_function<wilco_send_command>(VADDR_FN_SEND_COMMAND);
      send_cmd(cmd, nullptr);
   }

   inline HINSTANCE get_dll_instance() const
   { return _dll_instance; }

private:

   dll_info _dll_info;
   HINSTANCE _dll_instance;
};

}} // namespace oac::we

#endif // OAC_WE_WILCO_INTERNAL_H
