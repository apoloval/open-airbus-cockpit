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

class flight_control_unit_impl :
      public cockpit_front::flight_control_unit,
      public dll_inspector {
public:

   flight_control_unit_impl(const dll_info& dll_info, HINSTANCE dll_instance);

   virtual speed_units get_speed_display_units() const;

   virtual void push_speed_units_button();

   virtual guidance_display_mode get_guidance_display_mode() const;

   virtual void push_guidance_display_mode();

   virtual altitude_units get_altitude_display_units() const;

   virtual void push_altitude_units_button();

   virtual binary_switch get_switch(fcu_switch sw) const;

   virtual void push_switch(fcu_switch sw);

   virtual fcu_management_mode get_speed_mode() const;

   virtual fcu_management_mode get_lateral_mode() const;

   virtual fcu_management_mode get_vertical_mode() const;

   virtual knots get_speed_value() const;

   virtual void set_speed_value(knots value);

   virtual mach100 get_mach_value() const;

   virtual void set_mach_value(mach100 value);

   virtual degrees get_heading_value() const;

   virtual void set_heading_value(degrees value);

   virtual degrees get_track_value() const;

   virtual void set_track_value(degrees value);

   virtual feet get_target_altitude() const;

   virtual void set_target_altitude(feet value);

   virtual feet_per_min get_vertical_speed_value() const;

   virtual void set_vertical_speed_value(feet_per_min value);

   virtual degrees100 get_fpa_value() const;

   virtual void set_fpa_value(degrees100 value);

   virtual void push_knob(fcu_knob knob);

   virtual void pull_knob(fcu_knob knob);

private:

   inline void mutate(std::function<void(wilco_fcu&)> mutator)
   {
      auto fcu = this->get_data_object<wilco_fcu*>(VADDR_FCU);
      mutator(*fcu);
   }

   template <typename T>
   inline T access(std::function<T(const wilco_fcu&)> accessor) const
   {
      auto fcu = this->get_data_object<wilco_fcu*>(VADDR_FCU);
      return accessor(*fcu);
   }

   ptr<local_fsuipc> _fsuipc;
};

}} // namespace oac

#endif // OAC_WE_WILCO_FCU_H
