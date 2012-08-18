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

template <typename Event, typename ValueType>
void
TestFlightControlUnit::setParameterUnits(
      ValueType& parameter, const ValueType& units)
{
   parameter = units;
   Event ev;
   ev.newUnits = units;
   this->sendEvent(&ev);
}


TestFlightControlUnit::TestFlightControlUnit() :
   _speed(180), _speedMode(PARAM_MANAGED),
   _course(0), _courseMode(PARAM_MANAGED),
   _targetAltitude(7000)
{}

FlightControlUnit::ParameterMode
TestFlightControlUnit::speedMode() const
{ return _speedMode; }
   
void
TestFlightControlUnit::setSpeedMode(ParameterMode mode)
{ this->setParameterMode<EventSpeedModeToggled>(_speedMode, mode); }

Speed::Units
TestFlightControlUnit::speedUnits() const
{ return _speedUnits; }

void
TestFlightControlUnit::setSpeedUnits(Speed::Units units)
{ 
   this->setParameterUnits<EventSpeedUnitsToggled, Speed::Units>(
         _speedUnits, units);
}
   
Speed
TestFlightControlUnit::speedValue() const
{ return _speed; }

void
TestFlightControlUnit::setSpeedValue(const Speed& speed)
{ this->setParameterValue<EventSpeedValueChanged, Speed>(_speed, speed); }
   
FlightControlUnit::ParameterMode
TestFlightControlUnit::courseMode() const
{ return _courseMode; }
   
void
TestFlightControlUnit::setCourseMode(ParameterMode mode)
{ this->setParameterMode<EventCourseModeToggled>(_courseMode, mode); }
   
Course
TestFlightControlUnit::courseValue() const
{ return _course; }

void
TestFlightControlUnit::setCourseValue(const Course& course)
{ this->setParameterValue<EventCourseValueChanged, Course>(_course, course); }
   
unsigned int
TestFlightControlUnit::targetAltitudeValue() const
{ return _targetAltitude; }

void
TestFlightControlUnit::setTargetAltitudeValue(unsigned int targetAltitude)
{ 
   this->setParameterValue<EventTargetAltitudeValueChanged, unsigned int>(
         _targetAltitude, targetAltitude);
}

FlightControlUnit::ParameterMode
TestFlightControlUnit::verticalSpeedMode() const
{ return _verticalSpeedMode; }
   
void
TestFlightControlUnit::setVerticalSpeedMode(ParameterMode mode)
{ 
   this->setParameterMode<EventVerticalSpeedModeToggled>(
         _verticalSpeedMode, mode);
}
   
int
TestFlightControlUnit::verticalSpeedValue() const
{ return _verticalSpeed; }
   
void
TestFlightControlUnit::setVerticalSpeedValue(int verticalSpeed)
{ 
   this->setParameterValue<EventVerticalSpeedValueChanged, int>(
         _verticalSpeed, verticalSpeed);
}

}}; // namespace oac::server
