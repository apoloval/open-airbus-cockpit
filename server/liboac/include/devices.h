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

#ifndef OAC_SERVER_DEVICES_H
#define OAC_SERVER_DEVICES_H

#include "components.h"
#include "config.h"
#include "oacsp.h"
#include "oacsp-utils.h"
#include "serial.h"

namespace oac {

class LIBOAC_EXPORT FCUDeviceManager
{
public:

   FCUDeviceManager(const Ptr<SerialDevice>& serialDevice,
                    const Ptr<FlightControlUnit>& fcu);

private:

   Ptr<SerialDevice> _serialDevice;
   Ptr<FlightControlUnit> _fcu;
   SerialProtocolManager _protocolManager;

   void onSpeedModeToggled(
         const FlightControlUnit::EventSpeedModeToggled& ev);
   
   void onSpeedValueChanged(
         const FlightControlUnit::EventSpeedValueChanged& ev);
   
   void onCourseModeToggled(
         const FlightControlUnit::EventCourseModeToggled& ev);
   
   void onCourseValueChanged(
         const FlightControlUnit::EventCourseValueChanged& ev);
      
   void onTargetAltitudeChanged(
         const FlightControlUnit::EventTargetAltitudeValueChanged& ev);
   
   void onVerticalSpeedModeToggled(
         const FlightControlUnit::EventVerticalSpeedModeToggled& ev);
   
   void onVerticalSpeedValueChanged(
         const FlightControlUnit::EventVerticalSpeedValueChanged& ev);

   word status();
   
};

}; // namespace oac

#endif
