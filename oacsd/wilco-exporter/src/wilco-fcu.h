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

#ifndef OAC_WE_WILCO_FCU_H
#define OAC_WE_WILCO_FCU_H

#include <liboac/fsuipc.h>

#include "wilco.h"
#include "wilco-internal.h"

namespace oac { namespace we {

class FlightControlUnitImpl :
      public CockpitFront::FlightControlUnit,
      public DllInspector {
public:

   FlightControlUnitImpl(const DllInfo& dll_info, HINSTANCE dll_instance);

   virtual SpeedUnits getSpeedDisplayUnits() const;

   virtual void pushSpeedUnitsButton();

   virtual GuidanceDisplayMode getGuidanceDisplayMode() const;

   virtual void pushGuidanceModeButton();

   virtual AltitudeUnits getAltitudeDisplayUnits() const;

   virtual void pushAltitudeUnitsButton();

   virtual BinarySwitch getSwitch(FCUSwitch sw) const;

   virtual void pushSwitch(FCUSwitch sw);

   virtual FCUManagementMode getSpeedMode() const;

   virtual FCUManagementMode getLateralMode() const;

   virtual FCUManagementMode getVerticalMode() const;

   virtual Knots getSpeedValue() const;

   virtual void setSpeedValue(Knots value);

   virtual Mach100 getMachValue() const;

   virtual void setMachValue(Mach100 value);

   virtual Degrees getHeadingValue() const;

   virtual void setHeadingValue(Degrees value);

   virtual Degrees getTrackValue() const;

   virtual void setTrackValue(Degrees value);

   virtual Feet getTargetAltitude() const;

   virtual void setTargetAltitude(Feet value);

   virtual FeetPerMin getVerticalSpeedValue() const;

   virtual void setVerticalSpeedValue(FeetPerMin value);

   virtual Degrees100 getFPAValue() const;

   virtual void setFPAValue(Degrees100 value);

   virtual void pushKnob(FCUKnob knob);

   virtual void pullKnob(FCUKnob knob);

private:

   inline void mutate(std::function<void(Wilco_FCU&)> mutator)
   {
      auto wilco_fcu = this->getDataObject<Wilco_FCU*>(VADDR_FCU);
      mutator(*wilco_fcu);
   }

   template <typename T>
   inline T access(std::function<T(const Wilco_FCU&)> accessor) const
   {
      auto wilco_fcu = this->getDataObject<Wilco_FCU*>(VADDR_FCU);
      return accessor(*wilco_fcu);
   }

   Ptr<FSUIPC> _fsuipc;
};

}} // namespace oac

#endif // OAC_WE_WILCO_FCU_H
