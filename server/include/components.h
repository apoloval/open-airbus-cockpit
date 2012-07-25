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

namespace oac { namespace server {

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
   
   /**
    * Obtain speed mode. 
    * */
   virtual ParameterMode speedMode() const = 0;
   
   /**
    * Set speed mode. 
    */
   virtual void setSpeedMode(ParameterMode mode) = 0;
   
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
    * Obtain heading mode. 
    */
   virtual ParameterMode headingMode() const = 0;
   
   /**
    * Set heading mode. 
    */
   virtual void setHeadingMode(ParameterMode mode) = 0;
   
   /**
    * Obtain heading value. 
    */
   virtual Heading headingValue() const = 0;
   
   /**
    * Set heading value. If heading is managed, it has no effect.
    */
   virtual void setHeadingValue(const Heading& heading) = 0;
   
   /**
    * Obtain target altitude value. 
    */
   virtual unsigned int targetAltitudeValue() const = 0;
   
   /**
    * Set target altitude value. 
    */
   virtual void setTargetAltitudeValue(unsigned int value) = 0;
   
   DECL_EVENT(EventSpeedValueChanged, Speed newValue);
   DECL_EVENT(EventHeadingValueChanged, Heading newValue);
   DECL_EVENT(EventSpeedModeToggled, ParameterMode newMode);
   DECL_EVENT(EventHeadingModeToggled, ParameterMode newMode);
   DECL_EVENT(EventTargetAltitudeValueChanged, unsigned int newValue);

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
   
}}; //namespace oac::server

#endif
