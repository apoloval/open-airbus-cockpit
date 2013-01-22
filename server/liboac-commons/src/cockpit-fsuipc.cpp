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

#include <functional>

#include <Boost/format.hpp>

#include "cockpit.h"
#include "cockpit-fsuipc.h"
#include "fsuipc.h"
#include "logging.h"

namespace oac {

namespace {

template <DWORD offset, typename Data, typename EventType>
inline void ImportNullaryEvent(
      DoubleBuffer& buffer,
      std::list<CockpitBack::Event<EventType>>& events,
      EventType event_type,
      DWORD event_value = 0)
{
   auto push = buffer.readAs<Data>(offset);
   if (push)
   {
      CockpitBack::Event<EventType> ev = { event_type, event_value };
      events.push_back(ev);
      buffer.writeAs<Data>(offset, 0);
   }
}

template <DWORD offset, typename Data, typename EventType>
inline static void ImportUnaryEvent(
      DoubleBuffer& buffer,
      std::list<CockpitBack::Event<EventType>>& events,
      EventType event_type)
{
   if (buffer.isModifiedAs<Data>(offset))
   {
      CockpitBack::Event<EventType> ev = 
      { 
         event_type, 
         buffer.readAs<Data>(offset)
      };
      events.push_back(ev);
   }
}

class FlightControlUnitBack : public CockpitBack::FlightControlUnit {
public:

   inline FlightControlUnitBack(const Ptr<DoubleBuffer>& buffer) :
      _buffer(buffer)
   {}

   virtual void pollEvents(EventList& events)
   {
      ImportNullaryEvent<0x5620, BYTE, EventType>(
            *_buffer, events, FCU_SPD_BTN_PRESSED);
      ImportNullaryEvent<0x5621, BYTE, EventType>(
            *_buffer, events, FCU_GUI_MODE_BTN_PRESSED);
      ImportNullaryEvent<0x5622, BYTE, EventType>(
            *_buffer, events, FCU_ALT_UNITS_BTN_PRESSED);
      ImportNullaryEvent<0x5608, BYTE, EventType>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_LOC);
      ImportNullaryEvent<0x560B, BYTE, EventType>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_ATHR);
      ImportNullaryEvent<0x560C, BYTE, EventType>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_EXPE);
      ImportNullaryEvent<0x5609, BYTE, EventType>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_AP1);
      ImportNullaryEvent<0x560A, BYTE, EventType>(
            *_buffer, events, FCU_SW_PRESSED, FCU_SWITCH_AP2);
      ImportNullaryEvent<0x5627, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_SPD);
      ImportNullaryEvent<0x5628, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_SPD);
      ImportNullaryEvent<0x5629, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_HDG);
      ImportNullaryEvent<0x562A, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_HDG);
      ImportNullaryEvent<0x562B, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_ALT);
      ImportNullaryEvent<0x562C, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_ALT);
      ImportNullaryEvent<0x562D, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PRESSED, FCU_KNOB_VS);
      ImportNullaryEvent<0x562E, BYTE, EventType>(
            *_buffer, events, FCU_KNOB_PULLED, FCU_KNOB_VS);
      ImportUnaryEvent<0x5638, DWORD, EventType>(
            *_buffer, events, FCU_SPD_VALUE_CHANGED);
      ImportUnaryEvent<0x563C, DWORD, EventType>(
            *_buffer, events, FCU_MACH_VALUE_CHANGED);
      ImportUnaryEvent<0x5640, DWORD, EventType>(
            *_buffer, events, FCU_HDG_VALUE_CHANGED);
      ImportUnaryEvent<0x5644, DWORD, EventType>(
            *_buffer, events, FCU_TRACK_VALUE_CHANGED);
      ImportUnaryEvent<0x5648, DWORD, EventType>(
            *_buffer, events, FCU_ALT_VALUE_CHANGED);
      ImportUnaryEvent<0x564C, DWORD, EventType>(
            *_buffer, events, FCU_VS_VALUE_CHANGED);
      ImportUnaryEvent<0x5654, DWORD, EventType>(
            *_buffer, events, FCU_FPA_VALUE_CHANGED);
   }

   virtual void setSpeedDisplayUnits(SpeedUnits units)
   { _buffer->writeAs<BYTE>(0x562F, units); }

   virtual void setGuidanceDisplayMode(GuidanceDisplayMode mode)
   { _buffer->writeAs<BYTE>(0x5630, mode); }
   
   virtual void setAltitudeDisplayUnits(AltitudeUnits units)
   { _buffer->writeAs<BYTE>(0x5631, units); }
   
   virtual void setSwitch(FCUSwitch sw, BinarySwitch value)
   {
      switch (sw)
      {
         case FCU_SWITCH_LOC:
            _buffer->writeAs<BYTE>(0x5614, value); break;
         case FCU_SWITCH_ATHR:
            _buffer->writeAs<BYTE>(0x5617, value); break;
         case FCU_SWITCH_EXPE:
            _buffer->writeAs<BYTE>(0x5618, value); break;
         case FCU_SWITCH_APPR:
            _buffer->writeAs<BYTE>(0x5619, value); break;
         case FCU_SWITCH_AP1:
            _buffer->writeAs<BYTE>(0x5615, value); break;
         case FCU_SWITCH_AP2:
            _buffer->writeAs<BYTE>(0x5616, value); break;
      }
   }
   
   virtual void setSpeedMode(FCUManagementMode mode)
   { _buffer->writeAs<BYTE>(0x5632, mode); }
   
   virtual void setLateralMode(FCUManagementMode mode)
   { _buffer->writeAs<BYTE>(0x5633, mode); }
   
   virtual void setVerticalMode(FCUManagementMode mode)
   { _buffer->writeAs<BYTE>(0x5634, mode); }
   
   virtual void setSpeedValue(Knots value)
   { _buffer->writeAs<DWORD>(0x5638, DWORD(value)); }

   virtual void setMachValue(Mach100 value)
   { _buffer->writeAs<DWORD>(0x563C, DWORD(value)); }

   virtual void setHeadingValue(Degrees value)
   { _buffer->writeAs<DWORD>(0x5640, DWORD(value)); }

   virtual void setTrackValue(Degrees value)
   { _buffer->writeAs<DWORD>(0x5644, DWORD(value)); }
   
   virtual void setTargetAltitude(Feet value)
   { _buffer->writeAs<DWORD>(0x5648, value); }
   
   virtual void setVerticalSpeedValue(FeetPerMin value)
   { _buffer->writeAs<DWORD>(0x564C, value); }
   
   virtual void setFPAValue(Degrees value)
   { _buffer->writeAs<DWORD>(0x5650, DWORD(value / 100.0f)); }

private:

   Ptr<DoubleBuffer> _buffer;
};

class EFISControlPanelBack : public CockpitBack::EFISControlPanel {
public:

   inline EFISControlPanelBack(const Ptr<DoubleBuffer>& buffer) :
      _buffer(buffer)
   {}

   virtual void pollEvents(EventList& events)
   {
      ImportNullaryEvent<0x5601, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_FD_BTN_PRESSED);
      ImportNullaryEvent<0x5602, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_ILS_BTN_PRESSED);
      ImportNullaryEvent<0x5603, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_CONSTRAINT);
      ImportNullaryEvent<0x5604, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_WAYPOINT);
      ImportNullaryEvent<0x5605, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_VORD);
      ImportNullaryEvent<0x5606, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_NDB);
      ImportNullaryEvent<0x5607, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_MCP_SW_PRESSED, MCP_AIRPORT);
      ImportNullaryEvent<0x561A, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_BARO_FMT_SELECTED, BARO_FMT_IN_HG);
      ImportNullaryEvent<0x561B, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_BARO_FMT_SELECTED, BARO_FMT_H_PA);
      ImportNullaryEvent<0x561D, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_BARO_MODE_SELECTED, BARO_SELECTED);
      ImportNullaryEvent<0x561E, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_BARO_MODE_SELECTED, BARO_STANDARD);
      ImportUnaryEvent<0x561F, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_BARO_MODE_SELECTED);
      ImportUnaryEvent<0x5623, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_ND_MODE_SELECTED);
      ImportUnaryEvent<0x5624, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_ND_RANGE_SELECTED);
      ImportUnaryEvent<0x5625, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_ND_NAV1_MODE_SELECTED);
      ImportUnaryEvent<0x5626, BYTE, EventType>(
            *_buffer, events, EFIS_CTRL_ND_NAV2_MODE_SELECTED);
   }

   virtual void setBarometricMode(BarometricMode mode)
   { _buffer->writeAs<BYTE>(0x561F, mode); }
   
   virtual void setBarometricFormat(BarometricFormat fmt)
   { _buffer->writeAs<BYTE>(0x561C, fmt); }
   
   virtual void setILSButton(BinarySwitch btn)
   { _buffer->writeAs<BYTE>(0x560E, btn); }
   
   virtual void setMCPSwitch(MCPSwitch sw, BinarySwitch value)
   { 
      switch (sw)
      {
         case MCP_CONSTRAINT:
            _buffer->writeAs<BYTE>(0x560F, value); break;
         case MCP_WAYPOINT:
            _buffer->writeAs<BYTE>(0x5610, value); break;
         case MCP_VORD:
            _buffer->writeAs<BYTE>(0x5611, value); break;
         case MCP_NDB:
            _buffer->writeAs<BYTE>(0x5612, value); break;
         case MCP_AIRPORT:
            _buffer->writeAs<BYTE>(0x5613, value); break;
      }
   }
   
   virtual void setNDModeSwitch(NDModeSwitch mode)
   { _buffer->writeAs<BYTE>(0x5623, mode); }
   
   virtual void setNDRangeSwitch(NDRangeSwitch range)
   { _buffer->writeAs<BYTE>(0x5624, range); }
   
   virtual void setNDNav1ModeSwitch(NDNavModeSwitch value)
   { _buffer->writeAs<BYTE>(0x5625, value); }
   
   virtual void setNDNav2ModeSwitch(NDNavModeSwitch value)
   { _buffer->writeAs<BYTE>(0x5626, value); }

private:

   Ptr<DoubleBuffer> _buffer;   
};

}; // anonymous namespace

void
FSUIPCCockpitBack::syncUp()
throw (SyncException) 
{ 
   if (!_buffer)
      return;
   if (!_fsuipc)
      this->initFSUIPC();
   _buffer->writeAs<BYTE>(0x5600, 1);
   _fsuipc->copy(*_buffer, 0x5600, 0x5600, 512);
}

void
FSUIPCCockpitBack::syncDown()
throw (SyncException) 
{
   try 
   {
      if (!_fsuipc)
         this->initFSUIPC();
      if (!_buffer) 
      {
         this->initBuffer();
         _buffer->copy(*_fsuipc, 0x5600, 0x5600, 512);
      }
      _buffer->swap();
      _buffer->copy(*_fsuipc, 0x5600, 0x5600, 512);
   } 
   catch (std::exception& e) 
   {
      _fsuipc.reset();
      throw SyncException(str(boost::format(
            "cannot synchonize FSUIPC cockpit back: %s") % e.what()));
   } 
}

CockpitBack::FlightControlUnit&
FSUIPCCockpitBack::getFlightControlUnit() 
throw (SyncException)
{
   if (!_fcu)
      this->initFCU();
   return *_fcu;
}

CockpitBack::EFISControlPanel& 
FSUIPCCockpitBack::getEFISControlPanel() 
throw (SyncException)
{
   if (!_efis_ctrl_panel)
      this->initEFISControlPanel();
   return *_efis_ctrl_panel;
}

void
FSUIPCCockpitBack::initBuffer()
{
   Ptr<DoubleBuffer::Factory> fact = new DoubleBuffer::Factory(
         new ShiftedBuffer::Factory(new FixedBuffer::Factory(512), 0x5600));
   _buffer = fact->createBuffer();
}

void
FSUIPCCockpitBack::initFSUIPC()
throw (SyncException)
{
   try 
   {
      _fsuipc = _fsuipc_fact->createFSUIPC();
   }
   catch (IllegalStateException& e)
   {
      throw SyncException(str(boost::format(
            "cannot init FSUIPC cockpit back; FSUIPC state error: %s")
            % e.what()));
   }
}

void
FSUIPCCockpitBack::initFCU()
throw (SyncException)
{
   if (!this->isSync())
      throw SyncException(
            "cannot init FCU for FSUIPC cockpit back: not synchronized");
   _fcu = new FlightControlUnitBack(_buffer);
}

void
FSUIPCCockpitBack::initEFISControlPanel()
throw (SyncException)
{
   if (!this->isSync())
      throw SyncException(
            "cannot init EFIS control panel for FSUIPC "
            "cockpit back: not synchronized");
   _efis_ctrl_panel = new EFISControlPanelBack(_buffer);
}

bool
FSUIPCCockpitBack::isSync() const
{ return _buffer; }

}; // namespace oac
