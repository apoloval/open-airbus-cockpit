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

#include <Windows.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <liboac/logging.h>
#include <SimConnect.h>

#include "wilco.h"

namespace oac { namespace we {

namespace {

const std::string AIRCRAFT_NAME[] =
{
   "Airbus A320 CFM",      // A320_CFM
};

const char* GET_INTERNAL_DATA_FUNC_NAME = "GetInternalData";
const char* GET_EXTENDED_DATA_FUNC_NAME = "GetExtendedData";

typedef size_t VirtualAddress;

enum VirtualAddressKey
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

struct DllInfo
{
   std::wstring name;
   VirtualAddress virtualAddresses[VADDR_NELEMENTS];
};

static const DllInfo DLL_INFO[] = 
{
   { TEXT("A320CFM_FeelThere.DLL"),
      {
         VirtualAddress(0x10000000), // Image base
         VirtualAddress(0x10008BD0), // Wilco_PushMCPButton (Constraint)
         VirtualAddress(0x10008C30), // Wilco_PushMCPButton (Waypoint)
         VirtualAddress(0x10008C90), // Wilco_PushMCPButton (VORD)
         VirtualAddress(0x10008CF0), // Wilco_PushMCPButton (NDB)
         VirtualAddress(0x10008D50), // Wilco_PushMCPButton (Airport)
         VirtualAddress(0x1001D560), // Wilco_IsApuAvailable()
         VirtualAddress(0x1001F170), // Wilco_GetFADECMode()
         VirtualAddress(0x1002C920), // Wilco_SendCommand()
         VirtualAddress(0x100D4A78), // ND Range 
         VirtualAddress(0x100D4A7C), // ND Mode
         VirtualAddress(0x100D4A80), // MCP Nav Left
         VirtualAddress(0x100D4A84), // MCP Nav Right
         VirtualAddress(0x1012A940), // FADEC data
         VirtualAddress(0x1012A95C), // Baro format
         VirtualAddress(0x1012A960), // Baro STD
         VirtualAddress(0x1012A964), // FCU Speed display
         VirtualAddress(0x1012A9AC), // MCP Constraint
         VirtualAddress(0x1012A9B0), // MCP Waypoint
         VirtualAddress(0x1012A9B4), // MCP VORD
         VirtualAddress(0x1012A9B8), // MCP NDB
         VirtualAddress(0x1012A9BC), // MCP Airport
         VirtualAddress(0x1012A9C0), // ILS Switch
         VirtualAddress(0x1012AA08), // FMGC data
         VirtualAddress(0x1012AA10), // FCU data
         VirtualAddress(0x1012AA14), // FBW data
         VirtualAddress(0x1012AA18), // Head panel data
         VirtualAddress(0x1012AA1C), // GPWS data
         VirtualAddress(0x1012AA20), // Pedestal data
         VirtualAddress(0x1012AA28), // APU data
         VirtualAddress(0x1012AA38), // GPU data
      },
   },
};

/******************************************/
/* >> Internal data types of Wilco DLL << */
/******************************************/

struct Wilco_FCU
{
   DWORD target_altitude;                 // 0x00
   UINT32 selected_vertical_speed;        // 0x04
   DWORD selected_track;                  // 0x08
   FLOAT selected_fpa;                    // 0x0C
   DWORD autopilot;                       // 0x10, Autopilot
   DWORD unknown_14;                      // 0x14 e.g., 0
   DWORD unknown_18;                      // 0x18 e.g., 0
   DWORD unknown_1c;                      // 0x1C e.g., 0
   DWORD auto_thrust;                     // 0x20, AutoThrust
   DWORD speed_mode;                      // 0x24, SpeedMode
   DWORD active_vertical_mode;            // 0x28, VerticalMode
   DWORD armed_vertical_mode;             // 0x2C, VerticalMode
   DWORD active_lateral_mode;             // 0x30, LateralMode
   DWORD armed_lateral_mode;              // 0x34, LateralMode
   DWORD hdg_track_display_mode;          // 0x38, FCUDisplayMode
   DWORD vs_fpa_display_mode;             // 0x3C, FCUDisplayMode
   DWORD speed_knob;                      // 0x40, FCUKnob
   DWORD heading_knob;                    // 0x44, FCUKnob
   DWORD metric_altitude;                 // 0x48, BinarySwitch
   DWORD vertical_speed_knob;             // 0x4C, FCUKnob
   void* unknown_50;                      // 0x50
   DWORD unknown_54;                      // 0x54 e.g., 0
   DWORD unknown_58;                      // 0x58 e.g., 500
   DWORD unknown_5c;                      // 0x5C e.g., 50
   DWORD unknown_60;                      // 0x60 e.g., 9
   DWORD unknown_64;                      // 0x64 e.g., 18
   DWORD unknown_68;                      // 0x68 e.g., 0
   DWORD unknown_6c;                      // 0x6C e.g., 0
   DWORD expedite;                        // 0x70, BinarySwitch
   DWORD autoland;                        // 0x74, BinarySwitch when AP engaged
   void* unknown_78;                      // 0x78
   DWORD unknown_7c;                      // 0x7C e.g., 0
   DWORD unknown_80;                      // 0x80 e.g., 1
   DWORD indicated_altitude;              // 0x84
}; // size: 34 (0x22) words, 136 (0x88) bytes

struct Wilco_HeadPanel
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

struct Wilco_GPUVTable
{
   uint8_t unknown_0000 [0x2C];                             // 0x00
   DWORD (__fastcall *isGpuAvailable)(void* _this);      // 0x2C
};

struct Wilco_GPU {
   Wilco_GPUVTable* vtable;
};

struct Wilco_APU {
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

DWORD (__fastcall *Wilco_IsApuAvailable)(Wilco_APU* apu);

struct Wilco_FADEC{};

DWORD (__fastcall *Wilco_GetFadecMode)(
   Wilco_FADEC* f, DWORD unused, DWORD fadecn);
double (__fastcall *Wilco_GetFadecTLA)(
   Wilco_FADEC* f, DWORD unused, DWORD fadecn);

struct Wilco_FMGC
{
   BYTE unused_00000[0x1F0];              // 0x00000
   DWORD cost_index;                      // 0x001F0
   BYTE unused_001F4[0x7F230];            // 0x001F4
   DOUBLE perf_factor;                    // 0x7F420

};

struct Wilco_GPWS
{
   DWORD sys_switch;                      // 0x00
   DWORD unused_04[6];                    // 0x04
   DWORD terr_switch;                     // 0x1C
   DWORD gs_mode_switch;                  // 0x20
   DWORD flap_mode_switch;                // 0x24
   DWORD ldg_flap3_switch;                // 0x28
};

struct Wilco_Pedestal
{
   BYTE unused_0000[0x08];                // 0x00
   DOUBLE ctl_value;                      // 0x08
   BYTE unused_0010[0x18];                // 0x10
   DWORD sd_page_selected;                // 0x28
   DWORD unused_002c;                     // 0x2C
   DWORD sd_page_button;                  // 0x30
};

struct Wilco_InternalData
{
   DWORD apu_status;                      // 0x00, APUStatus
   DWORD engine_max_pos;                  // 0x04, ThrustLevers
   DWORD tcas;                            // 0x08, TCAS
   DWORD selected_barometric_mode;        // 0x0C, BarometricMode
   DWORD transition_altitude;             // 0x10
   DWORD irs;                             // 0x14, IRSStatus
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

struct Wilco_ExtendedData
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
   Wilco_FCU* fcu;                        // 0x28
   void* fadec;                           // 0x2C
   Wilco_HeadPanel* head_panel;           // 0x30
   void* fbw;                             // 0x34, 0x160 len
   void* gpws;                            // 0x38
   void* pedestal;                        // 0x3C
   void* gpu;                             // 0x40, 57 words
};

enum Wilco_Command {
   CMD_SET_ND_MODE = 0x5E,
   CMD_SET_ND_RANGE = 0x5F,
};

/**********************************/
/* >> Functions from Wilco DLL << */
/**********************************/

typedef DWORD (__stdcall *Wilco_PushMCPButton)(DWORD num1, DWORD num2);

typedef void (__cdecl *Wilco_SetNDMode)(DWORD mode);

typedef void (__cdecl *Wilco_SendCommand)(DWORD cmd, void* args);

/******************************/
/* >> Auxiliary operations << */
/******************************/

/**
 * Get function F from DLL instance, or throw E if fail. 
 */
template <typename F, typename E>
F GetFunctionOrThrow(HINSTANCE inst, const char* func_name)
{
   auto result = (F) GetProcAddress(inst, func_name);
   if (!result)
      throw E(str(boost::format(
         "cannot get function %s() from DLL") % func_name));
   return result;
}

HINSTANCE LoadDLLForAircraft(AircraftType aircraft)
throw (InvalidInputException)
{
   auto dll_filename = DLL_INFO[aircraft].name.c_str();
   HINSTANCE lib = LoadLibrary(dll_filename);
   if (lib == NULL)
      LogAndThrow(FAIL, InvalidInputException(str(boost::format(
            "cannot load Wilco Airbus DLL file %s") % dll_filename)));
   return lib;
}

void TrackChangesOnMemory(void* mem, size_t len) {
   static void* buf = nullptr;
   if (buf)
   {
      auto src = (DWORD*) mem;
      auto dst = (DWORD*) buf;
      for (unsigned int i = 0; i < len / size_t(4); i++)
      {
         if (src[i] != dst[i]) {
            Log(INFO, str(boost::format(
                  "word %d (+0x%X) changes from %d to %d") 
                  % i % i % dst[i] % src[i]));
         }
      }
   }
   else
   {
      Log(INFO, str(boost::format("Tracking %d bytes on 0x%X")
            % len % mem));
      buf = new uint8_t[len];
   }
   memcpy(buf, mem, len);
}

BinarySwitch Invert(BinarySwitch value)
{ return value == SWITCHED_ON ? SWITCHED_OFF : SWITCHED_ON; }

class DllInspector
{
public:

   inline DllInspector(const DllInfo& dll_info, HINSTANCE dll_instance) :
         _dll_info(dll_info), _dll_instance(dll_instance)
   {}

protected:

   inline void* getActualAddress(VirtualAddress vaddr) const
   {
      auto offset = vaddr - _dll_info.virtualAddresses[VADDR_BASE];
      auto actualAddress = VirtualAddress(_dll_instance) + offset;
      return (void*)(actualAddress);
   }

   template <typename DataObject>
   DataObject getDataObject(VirtualAddressKey vaddr_key) const
   {
      auto vaddr = _dll_info.virtualAddresses[vaddr_key];
      auto data = this->getActualAddress(vaddr);   
      return *(DataObject*) data;
   }

   template <typename Function>
   Function getFunction(VirtualAddressKey vaddr_key) const
   {
      auto vaddr = _dll_info.virtualAddresses[vaddr_key];
      auto addr = this->getActualAddress(vaddr);   
      return Function(addr);
   }

   template <typename DataObject>
   void setDataObject(
         VirtualAddressKey vaddrKey, const DataObject& new_data)
   {
      auto vaddr = _dll_info.virtualAddresses[vaddrKey];
      auto data = (DataObject*) this->getActualAddress(vaddr);   
      *data = new_data;
   }

   template <typename DataObject>
   void toggleBoolObject(VirtualAddressKey vaddrKey)
   {
      auto vaddr = _dll_info.virtualAddresses[vaddrKey];
      auto data = (DataObject*) this->getActualAddress(vaddr);   
      *data = !(*data);
   }

protected:

   inline HINSTANCE getDLLInstance() const
   { return _dll_instance; }

private:

   const DllInfo& _dll_info;
   HINSTANCE _dll_instance;
};

class EFISControlPanelImpl : 
      public CockpitFront::EFISControlPanel, 
      public DllInspector
{
public:

   EFISControlPanelImpl(const DllInfo& dll_info, HINSTANCE dll_instance) : 
         DllInspector(dll_info, dll_instance)
   {}

   virtual BarometricMode getBarometricMode() const 
   { return BarometricMode(this->getDataObject<DWORD>(VADDR_BARO_STD)); }

   virtual void setBarometricMode(BarometricMode mode)
   { this->setDataObject<DWORD>(VADDR_BARO_STD, mode);}

   virtual BarometricFormat getBarometricFormat() const
   { return BarometricFormat(this->getDataObject<DWORD>(VADDR_BARO_FORMAT)); }

   virtual void setBarometricFormat(BarometricFormat fmt)
   { this->setDataObject<DWORD>(VADDR_BARO_FORMAT, fmt); }

   virtual BinarySwitch getFDButton() const
   {
      // TODO: implement it by SimConnect
      return SWITCHED_OFF;
   }

   virtual void pushFDButton()
   {
      // TODO: implement it by SimConnect
   }

   virtual BinarySwitch getILSButton() const
   { return BinarySwitch(this->getDataObject<DWORD>(VADDR_ILS_SWITCH)); }

   virtual void pushILSButton()
   { 
      this->setDataObject<DWORD>(
            VADDR_ILS_SWITCH, Invert(this->getILSButton()));
   }

   virtual BinarySwitch getMCPSwitch(MCPSwitch sw) const
   {
      static VirtualAddressKey addresses[] =
      {
         VADDR_MCP_CONSTRAINT,
         VADDR_MCP_WAYPOINT,
         VADDR_MCP_VORD,
         VADDR_MCP_NDB,
         VADDR_MCP_AIRPORT,
      };
      return BinarySwitch(this->getDataObject<DWORD>(addresses[sw]));
   }

   virtual void pushMCPSwitch(MCPSwitch sw)
   {
      static VirtualAddressKey addresses[] =
      {
         VADDR_FN_PUSH_MCP_CONSTRAINT,
         VADDR_FN_PUSH_MCP_WAYPOINT,
         VADDR_FN_PUSH_MCP_VORD,
         VADDR_FN_PUSH_MCP_NDB,
         VADDR_FN_PUSH_MCP_AIRPORT,
      };
      auto push_btn = this->getFunction<Wilco_PushMCPButton>(addresses[sw]);
      push_btn(0, 0);
   }

   virtual NDModeSwitch getNDModeSwitch() const
   { return NDModeSwitch(this->getDataObject<DWORD>(VADDR_ND_MODE)); }

   virtual void setNDModeSwitch(NDModeSwitch mode)
   { this->sendCommand(CMD_SET_ND_MODE, &mode); }

   virtual NDRangeSwitch getNDRangeSwitch() const
   { return NDRangeSwitch(this->getDataObject<DWORD>(VADDR_ND_RANGE)); }

   virtual void setNDRangeSwitch(NDRangeSwitch range)
   { this->sendCommand(CMD_SET_ND_RANGE, &range); }

   virtual NDNavModeSwitch getNDNav1ModeSwitch() const
   { return NDNavModeSwitch(this->getDataObject<DWORD>(VADDR_MCP_NAV_LEFT)); }

   virtual void setNDNav1ModeSwitch(NDNavModeSwitch value)
   { this->setDataObject<DWORD>(VADDR_MCP_NAV_LEFT, value); }

   virtual NDNavModeSwitch getNDNav2ModeSwitch() const
   { return NDNavModeSwitch(this->getDataObject<DWORD>(VADDR_MCP_NAV_RIGHT)); }

   virtual void setNDNav2ModeSwitch(NDNavModeSwitch value)
   { this->setDataObject<DWORD>(VADDR_MCP_NAV_RIGHT, value); }

private:

   inline void sendCommand(Wilco_Command cmd, void* args)
   {
      auto send_cmd = 
            this->getFunction<Wilco_SendCommand>(VADDR_FN_SEND_COMMAND);
      send_cmd(cmd, args);
   }
};

class FlightControlUnitImpl : 
      public CockpitFront::FlightControlUnit, 
      public DllInspector {
public:

   FlightControlUnitImpl(const DllInfo& dll_info, HINSTANCE dll_instance) : 
         DllInspector(dll_info, dll_instance)
   {}

   virtual SpeedUnits getSpeedDisplayUnits() const
   { return SpeedUnits(this->getDataObject<DWORD>(VADDR_FCU_SPD_DISPLAY)); }

   virtual void pushSpeedUnitsButton()
   {
      auto units = this->getDataObject<DWORD>(VADDR_FCU_SPD_DISPLAY);
      this->setDataObject<DWORD>(VADDR_FCU_SPD_DISPLAY, units ^ 1);
   }

   virtual GuidanceDisplayMode getGuidanceDisplayMode() const 
   {
      return this->access<GuidanceDisplayMode>([](const Wilco_FCU& fcu) {
         return GuidanceDisplayMode(fcu.hdg_track_display_mode &&
            fcu.vs_fpa_display_mode);
      });
   }

   virtual void pushGuidanceModeButton()
   {
      this->mutate([](Wilco_FCU& fcu) {
         fcu.hdg_track_display_mode ^= 1;
         fcu.vs_fpa_display_mode ^= 1;
      });
   }

   virtual AltitudeUnits getAltitudeDisplayUnits() const
   {
      return this->access<AltitudeUnits>([](const Wilco_FCU& fcu) {
         return AltitudeUnits(fcu.metric_altitude);
      });
   }

   virtual void pushAltitudeUnitsButton() 
   {
      this->mutate([](Wilco_FCU& fcu) {
         fcu.metric_altitude ^= 1;
      });
   }

   virtual BinarySwitch getSwitch(FCUSwitch sw) const
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
               return BinarySwitch(fcu.autopilot & AP_1);
            default:
               return SWITCHED_OFF;
         }
      });
   }

   virtual void pushSwitch(FCUSwitch sw)
   {
      this->mutate([&sw](Wilco_FCU& fcu) {
         switch (sw)
         {
            case FCU_SWITCH_LOC:
               fcu.armed_lateral_mode = LAT_MOD_LOC; break;
            case FCU_SWITCH_ATHR:
               fcu.auto_thrust ^= 2; break;
            case FCU_SWITCH_EXPE:
               fcu.expedite ^= 1; break;
            case FCU_SWITCH_APPR:
               fcu.armed_lateral_mode = LAT_MOD_LOC; 
               fcu.armed_vertical_mode = VER_MOD_GS;
               break;
            case FCU_SWITCH_AP1:
               fcu.autopilot ^= AP_1; break;
            case FCU_SWITCH_AP2:
               fcu.autopilot ^= AP_1; break;
         }
      });
   }

   virtual FCUManagementMode getSpeedMode() const
   {
      return this->access<FCUManagementMode>([](const Wilco_FCU& fcu) {
         return FCUManagementMode(fcu.speed_knob);
      });
   }

   virtual FCUManagementMode getLateralMode() const
   {
      return this->access<FCUManagementMode>([](const Wilco_FCU& fcu) {
         return FCUManagementMode(fcu.heading_knob);
      });
   }

   virtual FCUManagementMode getVerticalMode() const
   {
      return this->access<FCUManagementMode>([](const Wilco_FCU& fcu) {
         return FCUManagementMode(fcu.vertical_speed_knob);    
      });
   }

   virtual Knots getSpeedValue() const
   {
      return 0; // TODO: implement this
   }

   virtual void setSpeedValue(Knots value)
   {
      // TODO: implement this
   }

   virtual Mach100 getMachValue() const
   {
      return 0; // TODO: implement this
   }

   virtual void setMachValue(Mach100 value)
   {
      // TODO: implement this
   }
   
   virtual Degrees getHeadingValue() const 
   {
      return 0; // TODO: implement this
   }

   virtual void setHeadingValue(Degrees value) 
   {
      // TODO: implement this
   }

   virtual Degrees getTrackValue() const
   {
      return this->access<Degrees>([](const Wilco_FCU& fcu) {
         return Degrees(fcu.selected_track);
      });
   }

   virtual void setTrackValue(Degrees value)
   {
      this->mutate([value](Wilco_FCU& fcu) {
         fcu.selected_track = value;
      });
   }
   
   virtual Feet getTargetAltitude() const
   {
      return this->access<Feet>([](const Wilco_FCU& fcu) {
         return fcu.target_altitude;
      });
   }

   virtual void setTargetAltitude(Feet value)
   {
      this->mutate([value](Wilco_FCU& fcu) {
         fcu.target_altitude = value;
      });
   }
   
   virtual FeetPerMin getVerticalSpeedValue() const
   {
      return this->access<FeetPerMin>([](const Wilco_FCU& fcu) {
         return fcu.selected_vertical_speed;
      });
   }

   virtual void setVerticalSpeedValue(FeetPerMin value)
   {
      this->mutate([value](Wilco_FCU& fcu) {
         fcu.selected_vertical_speed = value;
      });
   }
   
   virtual Degrees100 getFPAValue() const
   {
      return this->access<Degrees>([](const Wilco_FCU& fcu) {
         return Degrees100(fcu.selected_fpa * 100);
      });
   }

   virtual void setFPAValue(Degrees100 value)
   {
      this->mutate([value](Wilco_FCU& fcu) {
         fcu.selected_fpa = (value / 100.0f);
      });
   }

   virtual void pushKnob(FCUKnob knob)
   {
      // TODO: implement this
   }

   virtual void pullKnob(FCUKnob knob)
   {
      // TODO: implement this
   }
   
private:

   inline void mutate(std::function<void(Wilco_FCU&)> mutator)
   {
      auto wilco_fcu = this->getDataObject<Wilco_FCU*>(VADDR_FCU);
      mutator(*wilco_fcu);
   }

   template <typename T>
   inline T access(std::function<T(const Wilco_FCU&)> accessor) const
   {
      auto wilco_fcu = this->getDataObject<Wilco_FCU*>(VADDR_FCU);
      return accessor(*wilco_fcu);
   }
};


class WilcoCockpitImpl : public WilcoCockpit, public DllInspector
{
public:

   WilcoCockpitImpl(AircraftType aircraft)
   throw (InvalidInputException) :
         DllInspector(DLL_INFO[aircraft], LoadDLLForAircraft(aircraft)),
         _aircraft_type(aircraft)
   {
      _efis_ctrl_panel = std::shared_ptr<EFISControlPanel>(
            new EFISControlPanelImpl(
                  DLL_INFO[aircraft], this->getDLLInstance()));
      _fcu = std::shared_ptr<FlightControlUnit>(
            new FlightControlUnitImpl(
                  DLL_INFO[aircraft], this->getDLLInstance()));
   }

   virtual AircraftType aircraftType() const
   { return _aircraft_type; }

   virtual GPUSwitch getGpuSwitch() const
   {
      auto hp = this->getDataObject<Wilco_HeadPanel*>(VADDR_HEAD_PANEL);
      auto gpu = this->getDataObject<Wilco_GPU*>(VADDR_GPU);
      if (hp && hp->gpu_energy)
         return GPU_ON;
      if (gpu && gpu->vtable->isGpuAvailable(gpu))
         return GPU_AVAILABLE;
      return GPU_OFF;
   }

   virtual void getAPUSwitches(APUSwitches& sw) const
   {
      auto apu = this->getDataObject<Wilco_APU*>(VADDR_APU);
      sw.master = apu->master_switch ? APU_MASTER_ON : APU_MASTER_OFF;

      if (!apu->unused_08 && apu->unused_0c == 0x40590000)
         sw.start = APU_START_AVAILABLE;
      else if (apu->master_switch && apu->unused_0c > 0)
         sw.start = APU_START_ON;
      else
         sw.start = APU_START_OFF;
   }

   virtual SDPageButton getSDPageButton() const
   {
      auto pedestal = this->getDataObject<Wilco_Pedestal*>(VADDR_PEDESTAL);
      return (SDPageButton) pedestal->sd_page_selected;
   }

   virtual const FlightControlUnit& getFlightControlUnit() const 
   { return *_fcu; }

   virtual FlightControlUnit& getFlightControlUnit() 
   { return *_fcu; }

   virtual const EFISControlPanel& getEFISControlPanel() const
   { return *_efis_ctrl_panel; }

   virtual EFISControlPanel& getEFISControlPanel()
   { return *_efis_ctrl_panel; }

   virtual void debug() const
   {
      auto data = (DWORD*) this->getActualAddress(0x1012AA40);
      TrackChangesOnMemory(data, 4*sizeof(DWORD));
   }

private:

   inline static BinarySwitch toBinarySwitch(bool expr)
   { return expr ? SWITCHED_ON : SWITCHED_OFF; }

   inline static BinarySwitch toBinarySwitch(DWORD expr)
   { return toBinarySwitch(expr > 0); }

   AircraftType _aircraft_type;
   std::shared_ptr<EFISControlPanel> _efis_ctrl_panel;
   std::shared_ptr<FlightControlUnit> _fcu;
};

}; // anonymous namespace

AircraftType
ResolveAircraftTypeFromTitle(const std::string& title)
throw (InvalidInputException)
{
   if (boost::contains(title, "Feelthere"))
   {
      if (boost::contains(title, "A320 CFM"))
         return A320_CFM;
   }
   throw InvalidInputException(boost::format(
         "cannot determine the aircraft type from given title '%s'") % title);
}

const std::string&
AircraftTypeToString(AircraftType aircraft)
{ return AIRCRAFT_NAME[aircraft]; }

WilcoCockpit*
WilcoCockpit::newCockpit(AircraftType aircraft)
{
   return new WilcoCockpitImpl(aircraft);
}

WilcoCockpit*
WilcoCockpit::newCockpit(const std::string& aircraft_title)
throw (InvalidInputException)
{
   return WilcoCockpit::newCockpit(
            ResolveAircraftTypeFromTitle(aircraft_title));
}

}}; // namespace oac::we
