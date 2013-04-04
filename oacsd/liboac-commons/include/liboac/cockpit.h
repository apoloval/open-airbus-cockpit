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

#ifndef OAC_COCKPIT_H
#define OAC_COCKPIT_H

#include <list>

#include <Windows.h>

#include "exception.h"

namespace oac {

enum AltitudeUnits {
   ALT_FEET,
   ALT_METERS,
};

enum SpeedUnits {
   SPEED_KNOTS,
   SPEED_MACH,
};

typedef UINT32 Knots;
typedef UINT32 Mach100;
typedef UINT32 Degrees;
typedef UINT32 Degrees100;
typedef UINT32 Feet;
typedef INT32 FeetPerMin;

enum BinarySwitch
{
   SWITCHED_OFF = 0,
   SWITCHED_ON = 1,
};

enum GuidanceDisplayMode
{
   GUI_MOD_HDG_VS = 0,
   GUI_MOD_TRK_FPA = 1,
};

enum FCUManagementMode
{
   FCU_MNGT_SELECTED = 0,
   FCU_MNGT_MANAGED = 1,
};

enum FCUSwitch {
   FCU_SWITCH_LOC,
   FCU_SWITCH_ATHR,
   FCU_SWITCH_EXPE,
   FCU_SWITCH_APPR,
   FCU_SWITCH_AP1,
   FCU_SWITCH_AP2,      
};

enum FCUKnob {
   FCU_KNOB_SPD,
   FCU_KNOB_HDG,
   FCU_KNOB_ALT,
   FCU_KNOB_VS,
};

enum MCPSwitch
{
   MCP_CONSTRAINT,
   MCP_WAYPOINT,
   MCP_VORD,
   MCP_NDB,
   MCP_AIRPORT,
};

enum NDModeSwitch {
   ND_MOD_ILS,
   ND_MOD_VOR,
   ND_MOD_NAV,
   ND_MOD_ARC,
   ND_MOD_PLAN,
};

enum NDRangeSwitch {
   ND_RNG_10,
   ND_RNG_20,
   ND_RNG_40,
   ND_RNG_80,
   ND_RNG_160,
   ND_RNG_320,
};

enum NDNavModeSwitch {
   ND_NAV_ADF,
   ND_NAV_OFF,
   ND_NAV_VOR,
};

enum BarometricMode
{
   BARO_SELECTED,
   BARO_STANDARD,
};

enum BarometricFormat
{
   BARO_FMT_IN_HG,
   BARO_FMT_H_PA,
};

class CockpitBack;

/**
 * Cockpit front class. This class provides an abstraction for the cockpit
 * front side. That means the capabilities that make the cockpit usable
 * for a pilot (e.g., push a button, check wheter a light is on...). In the 
 * counterpart, there is a cockpit back side, which provides the opposite 
 * capabilities (e.g., check whether a button was pushed, turn on a light...).
 */
class CockpitFront {
public:

   class FlightControlUnit {
   public:
      inline virtual ~FlightControlUnit() {}
      virtual SpeedUnits getSpeedDisplayUnits() const = 0;
      virtual void pushSpeedUnitsButton() = 0;
      virtual GuidanceDisplayMode getGuidanceDisplayMode() const = 0;
      virtual void pushGuidanceModeButton() = 0;
      virtual AltitudeUnits getAltitudeDisplayUnits() const = 0;
      virtual void pushAltitudeUnitsButton() = 0;
      virtual BinarySwitch getSwitch(FCUSwitch sw) const = 0;
      virtual void pushSwitch(FCUSwitch sw) = 0;
      virtual FCUManagementMode getSpeedMode() const = 0;
      virtual FCUManagementMode getLateralMode() const = 0;
      virtual FCUManagementMode getVerticalMode() const = 0;
      virtual Knots getSpeedValue() const = 0;
      virtual void setSpeedValue(Knots value) = 0;
      virtual Mach100 getMachValue() const = 0;
      virtual void setMachValue(Mach100 value) = 0;
      virtual Degrees getHeadingValue() const = 0;
      virtual void setHeadingValue(Degrees value) = 0;
      virtual Degrees getTrackValue() const = 0;
      virtual void setTrackValue(Degrees value) = 0;
      virtual Feet getTargetAltitude() const = 0;
      virtual void setTargetAltitude(Feet value) = 0;
      virtual FeetPerMin getVerticalSpeedValue() const = 0;
      virtual void setVerticalSpeedValue(FeetPerMin value) = 0;
      virtual Degrees100 getFPAValue() const = 0;
      virtual void setFPAValue(Degrees100 value) = 0;
      virtual void pushKnob(FCUKnob knob) = 0;
      virtual void pullKnob(FCUKnob knob) = 0;
   };

   class EFISControlPanel {
   public:
      inline virtual ~EFISControlPanel() {}
      virtual BarometricMode getBarometricMode() const = 0;
      virtual void setBarometricMode(BarometricMode mode) = 0;
      virtual BarometricFormat getBarometricFormat() const = 0;
      virtual void setBarometricFormat(BarometricFormat fmt) = 0;
      virtual BinarySwitch getFDButton() const = 0;
      virtual void pushFDButton() = 0;
      virtual BinarySwitch getILSButton() const = 0;
      virtual void pushILSButton() = 0;
      virtual BinarySwitch getMCPSwitch(MCPSwitch sw) const = 0;
      virtual void pushMCPSwitch(MCPSwitch sw) = 0;
      virtual NDModeSwitch getNDModeSwitch() const = 0;
      virtual void setNDModeSwitch(NDModeSwitch mode) = 0;
      virtual NDRangeSwitch getNDRangeSwitch() const = 0;
      virtual void setNDRangeSwitch(NDRangeSwitch range) = 0;
      virtual NDNavModeSwitch getNDNav1ModeSwitch() const = 0;
      virtual void setNDNav1ModeSwitch(NDNavModeSwitch value) = 0;
      virtual NDNavModeSwitch getNDNav2ModeSwitch() const = 0;
      virtual void setNDNav2ModeSwitch(NDNavModeSwitch value) = 0;
   };

   inline virtual ~CockpitFront() {}

   void map(CockpitBack& back);

   virtual const FlightControlUnit& getFlightControlUnit() const = 0;
   virtual FlightControlUnit& getFlightControlUnit() = 0;

   virtual const EFISControlPanel& getEFISControlPanel() const = 0;
   virtual EFISControlPanel& getEFISControlPanel() = 0;
};

/**
 * Cockpit back class. This class provides an abstraction for the cockpit
 * back side. That means the capabilities that make it possible to govern
 * the cockpit (e.g., check whether a button was pressed, turn on a light...).
 * In the counterpart, there is a cockpit front side, which provides the
 * opposite capabilities (e.g., press a button, check whether a light is on...).
 */
class CockpitBack {
public:

   DECL_RUNTIME_ERROR(SyncError);

   template <typename T>
   struct Event
   {
      T type;
      DWORD value;
   };

   inline virtual ~CockpitBack() {}

   class FlightControlUnit {
   public:

      enum EventType
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

      typedef std::list<Event<EventType>> EventList;

      inline virtual ~FlightControlUnit() {}
      virtual void pollEvents(EventList& events) = 0;
      virtual void setSpeedDisplayUnits(SpeedUnits units) = 0;
      virtual void setGuidanceDisplayMode(GuidanceDisplayMode mode) = 0;
      virtual void setAltitudeDisplayUnits(AltitudeUnits units) = 0;
      virtual void setSwitch(FCUSwitch sw, BinarySwitch value) = 0;
      virtual void setSpeedMode(FCUManagementMode mode) = 0;
      virtual void setLateralMode(FCUManagementMode mode) = 0;
      virtual void setVerticalMode(FCUManagementMode mode) = 0;
      virtual void setSpeedValue(Knots value) = 0;
      virtual void setMachValue(Mach100 value) = 0;
      virtual void setHeadingValue(Degrees value) = 0;
      virtual void setTrackValue(Degrees value) = 0;
      virtual void setTargetAltitude(Feet value) = 0;
      virtual void setVerticalSpeedValue(FeetPerMin value) = 0;
      virtual void setFPAValue(Degrees100 value) = 0;
   };

   class EFISControlPanel {
   public:

      enum EventType
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

      typedef std::list<Event<EventType>> EventList;

      inline virtual ~EFISControlPanel() {}
      virtual void pollEvents(EventList& events) = 0;
      virtual void setBarometricMode(BarometricMode mode) = 0;
      virtual void setBarometricFormat(BarometricFormat fmt) = 0;
      virtual void setILSButton(BinarySwitch btn) = 0;
      virtual void setMCPSwitch(MCPSwitch sw, BinarySwitch value) = 0;
      virtual void setNDModeSwitch(NDModeSwitch mode) = 0;
      virtual void setNDRangeSwitch(NDRangeSwitch range) = 0;
      virtual void setNDNav1ModeSwitch(NDNavModeSwitch value) = 0;
      virtual void setNDNav2ModeSwitch(NDNavModeSwitch value) = 0;
   };

   void map(CockpitFront& front) throw (SyncError);

   /**
    * Move the status of this cockpit back to its counterpart. 
    */
   virtual void syncUp() throw (SyncError) = 0;

   /**
    * Move the status of the counterpart to this cockpit back.
    */
   virtual void syncDown() throw (SyncError) = 0;

   virtual FlightControlUnit& getFlightControlUnit() throw (SyncError) = 0;

   virtual EFISControlPanel& getEFISControlPanel() throw (SyncError) = 0;
};

}; // namespace oac

#endif
