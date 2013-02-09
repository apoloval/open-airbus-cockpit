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

EFISControlPanelImpl::EFISControlPanelImpl(
      const DllInfo& dll_info, HINSTANCE dll_instance) :
   DllInspector(dll_info, dll_instance), _fsuipc(new LocalFSUIPC())
{}

BarometricMode
EFISControlPanelImpl::getBarometricMode() const
{ return BarometricMode(this->getDataObject<DWORD>(VADDR_BARO_STD)); }

void
EFISControlPanelImpl::setBarometricMode(BarometricMode mode)
{
   switch (mode)
   {
      case BARO_SELECTED:
         this->sendCommand(CMD_EFIS_CTRL_PUSH_BARO_KNOB);
         break;
      case BARO_STANDARD:
         this->sendCommand(CMD_EFIS_CTRL_PULL_BARO_KNOB);
         break;
   }
}

BarometricFormat
EFISControlPanelImpl::getBarometricFormat() const
{ return BarometricFormat(this->getDataObject<DWORD>(VADDR_BARO_FORMAT)); }

void
EFISControlPanelImpl::setBarometricFormat(BarometricFormat fmt)
{
   switch (fmt)
   {
      case BARO_FMT_IN_HG:
         this->sendCommand(CMD_EFIS_CTRL_SET_INHG_BARO_KNOB);
         break;
      case BARO_FMT_H_PA:
         this->sendCommand(CMD_EFIS_CTRL_SET_HPA_BARO_KNOB);
         break;
   }
}

BinarySwitch
EFISControlPanelImpl::getFDButton() const
{
   return BinarySwitch(_fsuipc->readAs<DWORD>(0x2EE0));
}

void
EFISControlPanelImpl::pushFDButton()
{
   _fsuipc->writeAs<DWORD>(0x2EE0, this->invert(this->getFDButton()));
}

BinarySwitch
EFISControlPanelImpl::getILSButton() const
{ return BinarySwitch(this->getDataObject<DWORD>(VADDR_ILS_SWITCH)); }

void
EFISControlPanelImpl::pushILSButton()
{
   this->sendCommand(CMD_EFIS_CTRL_PRESS_ILS_BTN);
}

BinarySwitch
EFISControlPanelImpl::getMCPSwitch(MCPSwitch sw) const
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

void
EFISControlPanelImpl::pushMCPSwitch(MCPSwitch sw)
{
   switch (sw)
   {
      case MCP_CONSTRAINT:
         this->sendCommand(CMD_EFIS_CTRL_PRESS_CSTR_BTN);
         break;
      case MCP_WAYPOINT:
         this->sendCommand(CMD_EFIS_CTRL_PRESS_WPT_BTN);
         break;
      case MCP_VORD:
         this->sendCommand(CMD_EFIS_CTRL_PRESS_VORD_BTN);
         break;
      case MCP_NDB:
         this->sendCommand(CMD_EFIS_CTRL_PRESS_NDB_BTN);
         break;
      case MCP_AIRPORT:
         this->sendCommand(CMD_EFIS_CTRL_PRESS_ARPT_BTN);
         break;
   }
}

NDModeSwitch
EFISControlPanelImpl::getNDModeSwitch() const
{ return NDModeSwitch(this->getDataObject<DWORD>(VADDR_ND_MODE)); }

void
EFISControlPanelImpl::setNDModeSwitch(NDModeSwitch mode)
{ this->sendCommand<NDModeSwitch>(CMD_EFIS_CTRL_SET_ND_MODE, mode); }

NDRangeSwitch
EFISControlPanelImpl::getNDRangeSwitch() const
{ return NDRangeSwitch(this->getDataObject<DWORD>(VADDR_ND_RANGE)); }

void
EFISControlPanelImpl::setNDRangeSwitch(NDRangeSwitch range)
{ this->sendCommand<NDRangeSwitch>(CMD_EFIS_CTRL_SET_ND_RANGE, range); }

NDNavModeSwitch
EFISControlPanelImpl::getNDNav1ModeSwitch() const
{ return NDNavModeSwitch(this->getDataObject<DWORD>(VADDR_MCP_NAV_LEFT)); }

void
EFISControlPanelImpl::setNDNav1ModeSwitch(NDNavModeSwitch value)
{
   switch (value)
   {
      case ND_NAV_ADF:
         this->sendCommand(CMD_EFIS_CTRL_LEFT_NAV_SW_ADF);
         break;
      case ND_NAV_OFF:
         this->sendCommand(CMD_EFIS_CTRL_LEFT_NAV_SW_OFF);
         break;
      case ND_NAV_VOR:
         this->sendCommand(CMD_EFIS_CTRL_LEFT_NAV_SW_VOR);
         break;
   }
}

NDNavModeSwitch
EFISControlPanelImpl::getNDNav2ModeSwitch() const
{ return NDNavModeSwitch(this->getDataObject<DWORD>(VADDR_MCP_NAV_RIGHT)); }

void
EFISControlPanelImpl::setNDNav2ModeSwitch(NDNavModeSwitch value)
{
   switch (value)
   {
      case ND_NAV_ADF:
         this->sendCommand(CMD_EFIS_CTRL_RIGHT_NAV_SW_ADF);
         break;
      case ND_NAV_OFF:
         this->sendCommand(CMD_EFIS_CTRL_RIGHT_NAV_SW_OFF);
         break;
      case ND_NAV_VOR:
         this->sendCommand(CMD_EFIS_CTRL_RIGHT_NAV_SW_VOR);
         break;
   }
}

}} // namespace oac
