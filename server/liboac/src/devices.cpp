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

namespace oac {

FCUDeviceManager::FCUDeviceManager(
      SerialDevice* serialDevice, FlightControlUnit* fcu) :
   _serialDevice(serialDevice), _fcu(fcu),
   _protocolManager(new SerialProtocolManager(serialDevice))
{
   // _fcu->subscribe(this, &FCUDeviceManager::onSpeedModeToggled);
   _fcu->subscribe(this, &FCUDeviceManager::onSpeedValueChanged);
   _fcu->subscribe(this, &FCUDeviceManager::onCourseModeToggled);
   _fcu->subscribe(this, &FCUDeviceManager::onCourseValueChanged);
   _fcu->subscribe(this, &FCUDeviceManager::onTargetAltitudeChanged);  
   _fcu->subscribe(this, &FCUDeviceManager::onVerticalSpeedModeToggled);
   _fcu->subscribe(this, &FCUDeviceManager::onVerticalSpeedValueChanged);
   
   _fcu->subscribe<FlightControlUnit::EventSpeedModeToggled>(
         [this](const FlightControlUnit::EventSpeedModeToggled& ev)
         {
            this->_protocolManager->sendWriteVar(VAR_FCU_STATUS, status());
            if (ev.newMode == FlightControlUnit::PARAM_SELECTED)
               this->_protocolManager->sendWriteVar(
                     VAR_FCU_SEL_SPD, word(_fcu->speedValue().asKnots()));
         }
   );

   _protocolManager->sendReset();
   
   // Set the altitude to its current value to trigger the event
   // and send the proper command to the serial device. 
   _fcu->setTargetAltitudeValue(_fcu->targetAltitudeValue());
}

void
FCUDeviceManager::onSpeedModeToggled(
         const FlightControlUnit::EventSpeedModeToggled& ev)
{
   _protocolManager->sendWriteVar(VAR_FCU_STATUS, status());
   if (ev.newMode == FlightControlUnit::PARAM_SELECTED)
      _protocolManager->sendWriteVar(
            VAR_FCU_SEL_SPD, word(_fcu->speedValue().asKnots()));
}
   
void 
FCUDeviceManager::onSpeedValueChanged(
         const FlightControlUnit::EventSpeedValueChanged& ev)
{
   if (_fcu->speedMode() == FlightControlUnit::PARAM_SELECTED)
      _protocolManager->sendWriteVar(
            VAR_FCU_SEL_SPD, word(ev.newValue.asKnots()));
}
   
void 
FCUDeviceManager::onCourseModeToggled(
         const FlightControlUnit::EventCourseModeToggled& ev)
{
   _protocolManager->sendWriteVar(VAR_FCU_STATUS, status());
   if (ev.newMode == FlightControlUnit::PARAM_SELECTED)
      _protocolManager->sendWriteVar(VAR_FCU_SEL_HDG,
                                     word(_fcu->courseValue()));
}
   
void 
FCUDeviceManager::onCourseValueChanged(
      const FlightControlUnit::EventCourseValueChanged& ev)
{            
   if (_fcu->courseMode() == FlightControlUnit::PARAM_SELECTED)
         _protocolManager->sendWriteVar(VAR_FCU_SEL_HDG, word(ev.newValue));
}
      
void 
FCUDeviceManager::onTargetAltitudeChanged(
      const FlightControlUnit::EventTargetAltitudeValueChanged& ev)
{
   _protocolManager->sendWriteVar(VAR_FCU_TGT_ALT, word(ev.newValue));
}

void 
FCUDeviceManager::onVerticalSpeedModeToggled(
      const FlightControlUnit::EventVerticalSpeedModeToggled& ev)
{
   _protocolManager->sendWriteVar(VAR_FCU_STATUS, status());
   if (ev.newMode == FlightControlUnit::PARAM_SELECTED)
         _protocolManager->sendWriteVar(
               VAR_FCU_SEL_VS, _fcu->verticalSpeedValue());
}
   
void
FCUDeviceManager::onVerticalSpeedValueChanged(
      const FlightControlUnit::EventVerticalSpeedValueChanged& ev)
{
   if (_fcu->verticalSpeedMode() == FlightControlUnit::PARAM_SELECTED)
      _protocolManager->sendWriteVar(VAR_FCU_SEL_VS, ev.newValue);         
}

word
FCUDeviceManager::status()
{
   word val = 0;
   if (_fcu->speedMode() == FlightControlUnit::PARAM_SELECTED)
      val |= MASK_FCU_SPD_MOD;
   if (_fcu->courseMode() == FlightControlUnit::PARAM_SELECTED)
      val |= MASK_FCU_HDG_MOD;
   // TODO: complete the rest of state flags
   return val;
}

}; // namespace oac
