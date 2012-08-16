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

#ifndef OAC_SERVER_TEST_H
#define OAC_SERVER_TEST_H

#include "components.h"
#include "types.h"

namespace oac { namespace server {

class TestFlightControlUnit : public FlightControlUnit {
public:

   TestFlightControlUnit();

   virtual ParameterMode speedMode() const;
   
   virtual void setSpeedMode(ParameterMode mode);
   
   virtual Speed speedValue() const;
   
   virtual void setSpeedValue(const Speed& speed);

   virtual ParameterMode headingMode() const;
   
   virtual void setHeadingMode(ParameterMode mode);
   
   virtual Heading headingValue() const;
   
   virtual void setHeadingValue(const Heading& heading);

   virtual unsigned int targetAltitudeValue() const;

   virtual void setTargetAltitudeValue(unsigned int targetAltitude);

   virtual ParameterMode verticalSpeedMode() const;
   
   virtual void setVerticalSpeedMode(ParameterMode mode);
   
   virtual int verticalSpeedValue() const;
   
   virtual void setVerticalSpeedValue(int verticalSpeed);

private:

   Speed          _speed;
   ParameterMode  _speedMode;
   Heading        _heading;
   ParameterMode  _headingMode;
   unsigned int   _targetAltitude;
   int            _verticalSpeed;
   ParameterMode  _verticalSpeedMode;
   
   template <typename Event>
   void setParameterMode(ParameterMode& parameter, ParameterMode mode);
   
   template <typename Event, typename ValueType>
   void setParameterValue(ValueType& parameter, const ValueType& value);

};
   
}}; // namespace oac::server

#endif
