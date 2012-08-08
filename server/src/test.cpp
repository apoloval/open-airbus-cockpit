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

#include "test.h"

#include <OACSP/oacsp.h>

namespace oac { namespace server {

template<typename Event>
void
TestFlightControlUnit::setParameterMode(
      ParameterMode& parameter, ParameterMode mode)
{
   if (parameter != mode)
   {
      parameter = mode;
      Event ev;
      ev.newMode = mode;
      this->sendEvent(&ev);
   }
}

template <typename Event, typename ValueType>
void
TestFlightControlUnit::setParameterValue(
      ValueType& parameter, const ValueType& value)
{
   parameter = value;
   Event ev;
   ev.newValue = value;
   this->sendEvent(&ev);
}


TestFlightControlUnit::TestFlightControlUnit() :
   _speed(180), _speedMode(PARAM_MANAGED),
   _heading(0), _headingMode(PARAM_MANAGED),
   _targetAltitude(7000)
{}

FlightControlUnit::ParameterMode
TestFlightControlUnit::speedMode() const
{ return _speedMode; }
   
void
TestFlightControlUnit::setSpeedMode(ParameterMode mode)
{ this->setParameterMode<EventSpeedModeToggled>(_speedMode, mode); }
   
Speed
TestFlightControlUnit::speedValue() const
{ return _speed; }

void
TestFlightControlUnit::setSpeedValue(const Speed& speed)
{ this->setParameterValue<EventSpeedValueChanged, Speed>(_speed, speed); }
   
FlightControlUnit::ParameterMode
TestFlightControlUnit::headingMode() const
{ return _headingMode; }
   
void
TestFlightControlUnit::setHeadingMode(ParameterMode mode)
{ this->setParameterMode<EventHeadingModeToggled>(_headingMode, mode); }
   
Heading
TestFlightControlUnit::headingValue() const
{ return _heading; }

void
TestFlightControlUnit::setHeadingValue(const Heading& heading)
{ this->setParameterValue<EventHeadingValueChanged, Heading>(_heading, heading); }
   
unsigned int
TestFlightControlUnit::targetAltitudeValue() const
{ return _targetAltitude; }

void
TestFlightControlUnit::setTargetAltitudeValue(unsigned int value)
{ 
   this->setParameterValue<EventTargetAltitudeValueChanged, unsigned int>(
         _targetAltitude, value);
}

}}; // namespace oac::server
