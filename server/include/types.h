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

#ifndef OAC_SERVER_TYPES_H
#define OAC_SERVER_TYPES_H

namespace oac { namespace server {

class Speed
{
public:

   enum Units
   {
      UNITS_KT,
      UNITS_MACH,
   };
   
   inline Speed() : _value(0.0f), _units(UNITS_KT) {}
   
   inline Speed(float value, Units units = UNITS_KT) : 
      _value(value), _units(units)
   {
   }
   
   inline float asKnots() const
   { return (_units == UNITS_KT) ? _value : _value * 666.738661f; }

   inline float asMach() const
   { return (_units == UNITS_MACH) ? _value : _value / 666.738661f; }

private:

   float _value;
   Units _units;

};

class Course
{
public:

   enum Type
   {
      TYPE_HEADING,
      TYPE_TRACK,
   };

   inline Course() : Course(0.0f, TYPE_HEADING) {}

   inline Course(float value, Type type = TYPE_HEADING) : 
         _value(value), _type(type)
   { this->normalise(); }
   
   inline float value() const
   { return _value; }
   
   inline Type type() const
   { return _type; }
   
   inline operator float () const
   { return value(); }

private:

   float _value;
   Type  _type;
   
   inline void normalise()
   {
      while (_value > 360.0f)
         _value -= 360.0f;
      while (_value < 0.0f)
         _value += 360.0f;
   }
   
};
	
}}; // namespace oac::server

#endif
