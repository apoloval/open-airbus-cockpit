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

#ifndef OAC_SERVER_COMPONENTS_H
#define OAC_SERVER_COMPONENTS_H

#include "types.h"
#include "events.h"

namespace oac {

/**
 * The interface of a cockpit component.
 */
class CockpitComponent : public EventSender
{
public:
   
};

/**
 * The interface of a flight control unit cockpit component. 
 */
class FlightControlUnit : public CockpitComponent
{
public:

   enum ParameterMode
   {
      PARAM_MANAGED,
      PARAM_SELECTED,
   };
   
   enum VerticalLateralUnits
   {
      UNITS_HDG_VS,
      UNITS_TRACK_FPA,
   };
   
   /**
    * Obtain speed mode. 
    * */
   virtual ParameterMode speedMode() const = 0;
   
   /**
    * Set speed mode. 
    */
   virtual void setSpeedMode(ParameterMode mode) = 0;
   
   /**
    * Obtain speed display units.
    */
   virtual Speed::Units speedUnits() const = 0;
   
   /**
    * Set speed display units.
    */
   virtual void setSpeedUnits(Speed::Units units) = 0;      
   
   /**
    * Obtain selected speed value. If speed is managed, it
    * returns an unexpecified value.
    */
   virtual Speed speedValue() const = 0;
   
   /**
    * Set speed value. If speed is managed, it has no effect. 
    */
   virtual void setSpeedValue(const Speed& speed) = 0;
   
   /**
    * Obtain course mode. 
    */
   virtual ParameterMode courseMode() const = 0;
   
   /**
    * Set course mode. 
    */
   virtual void setCourseMode(ParameterMode mode) = 0;
   
   /**
    * Obtain course value. 
    */
   virtual Course courseValue() const = 0;
   
   /**
    * Set course value. If course is managed, it has no effect.
    */
   virtual void setCourseValue(const Course& course) = 0;
   
   /**
    * Obtain target altitude value. 
    */
   virtual unsigned int targetAltitudeValue() const = 0;
   
   /**
    * Set target altitude value. 
    */
   virtual void setTargetAltitudeValue(unsigned int value) = 0;
   
   /**
    * Obtain vertical speed mode. 
    */
   virtual ParameterMode verticalSpeedMode() const = 0;
   
   /**
    * Set vertical speed mode. 
    */
   virtual void setVerticalSpeedMode(ParameterMode mode) = 0;
   
   /**
    * Obtain vertical speed value. 
    */
   virtual int verticalSpeedValue() const = 0;
   
   /**
    * Set vertical speed value. If vertical speed is managed, it has no effect.
    */
   virtual void setVerticalSpeedValue(int verticalSpeed) = 0;
   
   /**
    * Toggle speed mode. 
    */
   inline void toggleSpeedMode()
   {
      setSpeedMode(
         (speedMode() == PARAM_MANAGED) ? PARAM_SELECTED : PARAM_MANAGED);
   }; 
   
   /**
    * Toggle course mode.
    */
   inline void toggleCourseMode()
   {
      setCourseMode(
         (courseMode() == PARAM_MANAGED) ? PARAM_SELECTED : PARAM_MANAGED);
   }; 
   
   /**
    * Toggle vertical speed mode.
    */
   inline void toggleVerticalSpeedMode()
   {
      setVerticalSpeedMode((verticalSpeedMode() == PARAM_MANAGED) 
            ? PARAM_SELECTED : PARAM_MANAGED);
   }
   
   DECL_EVENT(EventSpeedValueChanged, Speed newValue);
   DECL_EVENT(EventCourseValueChanged, Course newValue);
   DECL_EVENT(EventSpeedModeToggled, ParameterMode newMode);
   DECL_EVENT(EventSpeedUnitsToggled, Speed::Units newUnits);
   DECL_EVENT(EventCourseModeToggled, ParameterMode newMode);
   DECL_EVENT(EventTargetAltitudeValueChanged, unsigned int newValue);
   DECL_EVENT(EventVerticalSpeedModeToggled, ParameterMode newMode);
   DECL_EVENT(EventVerticalSpeedValueChanged, int newValue);

};

/**
 * The interface of any class capable of building cockpit components. 
 */
class ComponentBuilder
{
public:

   /**
    * Create a new flight control unit component. 
    */
   virtual FlightControlUnit* newFlightControlUnit();

};
   
}; //namespace oac

#endif
