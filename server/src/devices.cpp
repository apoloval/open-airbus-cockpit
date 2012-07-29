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

#include "devices.h"

namespace oac { namespace server {

FCUDeviceManager::FCUDeviceManager(
      SerialDevice* serialDevice, FlightControlUnit* fcu) :
   _serialDevice(serialDevice), _fcu(fcu)
{
   _fcu->subscribe(this, &FCUDeviceManager::onSpeedModeToggled);
   _fcu->subscribe(this, &FCUDeviceManager::onSpeedValueChanged);
   _fcu->subscribe(this, &FCUDeviceManager::onHeadingModeToggled);
   _fcu->subscribe(this, &FCUDeviceManager::onHeadingValueChanged);
   _fcu->subscribe(this, &FCUDeviceManager::onTargetAltitudeChanged);  

   Command cmd;
   cmd.type = CMD_RESET;
   sendCommand(cmd);
}

void
FCUDeviceManager::onSpeedModeToggled(
         const FlightControlUnit::EventSpeedModeToggled& ev)
{
   sendStatus();
   if (ev.newMode == FlightControlUnit::PARAM_SELECTED)
   {
      Command cmd;
      cmd.type = CMD_WRITE_VAR;
      cmd.writeVar.offset = VAR_FCU_SEL_SPD;
      cmd.writeVar.data = word(_fcu->speedValue().asKnots());
      sendCommand(cmd);      
   }
}
   
void 
FCUDeviceManager::onSpeedValueChanged(
         const FlightControlUnit::EventSpeedValueChanged& ev)
{
   if (_fcu->speedMode() == FlightControlUnit::PARAM_SELECTED)
   {
      Command cmd;
      cmd.type = CMD_WRITE_VAR;
      cmd.writeVar.offset = VAR_FCU_SEL_SPD;
      cmd.writeVar.data = word(ev.newValue.asKnots());
      sendCommand(cmd);      
   }
}
   
void 
FCUDeviceManager::onHeadingModeToggled(
         const FlightControlUnit::EventHeadingModeToggled& ev)
{
   sendStatus();
   if (ev.newMode == FlightControlUnit::PARAM_SELECTED)
   {
      Command cmd;
      cmd.type = CMD_WRITE_VAR;
      cmd.writeVar.offset = VAR_FCU_SEL_HDG;
      cmd.writeVar.data = word(_fcu->headingValue());
      sendCommand(cmd);      
   }
}
   
void 
FCUDeviceManager::onHeadingValueChanged(
      const FlightControlUnit::EventHeadingValueChanged& ev)
{            
   if (_fcu->headingMode() == FlightControlUnit::PARAM_SELECTED)
   {
      Command cmd;
      cmd.type = CMD_WRITE_VAR;
      cmd.writeVar.offset = VAR_FCU_SEL_HDG;
      cmd.writeVar.data = word(ev.newValue);
      sendCommand(cmd);      
   }
}
      
void 
FCUDeviceManager::onTargetAltitudeChanged(
      const FlightControlUnit::EventTargetAltitudeValueChanged& ev)
{            
}

void
FCUDeviceManager::sendCommand(const Command& cmd)
{
   _serialDevice->write(&(cmd.type), 1);
   switch (cmd.type)
   {
      case CMD_PING:
      case CMD_PONG:
         _serialDevice->write(&(cmd.ping.deviceType), 1);
         _serialDevice->write(&(cmd.ping.majorVersion), 1);
         _serialDevice->write(&(cmd.ping.minorVersion), 1);
         _serialDevice->write(&(cmd.ping.padding), 12);
         break;
      case CMD_RESET:
         _serialDevice->write(&(cmd.reset.padding), 15);
         break;
      case CMD_WRITE_VAR:
         _serialDevice->write(&(cmd.writeVar.offset), 2);
         _serialDevice->write(&(cmd.writeVar.data), 2);
         _serialDevice->write(&(cmd.writeVar.padding), 11);
         break;
      case CMD_NOTIFY_EVENT:
         _serialDevice->write(&(cmd.notifyEvent.event), 2);
         _serialDevice->write(&(cmd.notifyEvent.param1), 2);
         _serialDevice->write(&(cmd.notifyEvent.param2), 2);
         _serialDevice->write(&(cmd.notifyEvent.param3), 2);
         _serialDevice->write(&(cmd.notifyEvent.param4), 2);
         _serialDevice->write(&(cmd.notifyEvent.padding), 5);
         break;
   }
}

word
FCUDeviceManager::status()
{
   word val = 0;
   if (_fcu->speedMode() == FlightControlUnit::PARAM_SELECTED)
      val |= MASK_FCU_SPD_MOD;
   if (_fcu->headingMode() == FlightControlUnit::PARAM_SELECTED)
      val |= MASK_FCU_HDG_MOD;
   // TODO: complete the rest of state flags
   return val;
}

void
FCUDeviceManager::sendStatus()
{
   Command cmd;
   cmd.type = CMD_WRITE_VAR;
   cmd.writeVar.offset = VAR_FCU_STATUS;
   cmd.writeVar.data = status();
   sendCommand(cmd);
}

}}; // namespace oac::server
