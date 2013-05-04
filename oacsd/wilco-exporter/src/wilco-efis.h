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

#ifndef OAC_WE_WILCO_EFIS_H
#define OAC_WE_WILCO_EFIS_H

#include <liboac/fsuipc.h>

#include "wilco.h"
#include "wilco-internal.h"

namespace oac { namespace we {

class efis_control_panel_impl :
      public cockpit_front::efis_control_panel,
      public dll_inspector
{
public:

   efis_control_panel_impl(const dll_info& dll_info, HINSTANCE dll_instance);

   virtual barometric_mode get_barometric_mode() const;

   virtual void set_barometric_mode(barometric_mode mode);

   virtual barometric_format get_barometric_format() const;

   virtual void set_barometric_format(barometric_format fmt);

   virtual binary_switch get_fd_button() const;

   virtual void push_fd_button();

   virtual binary_switch get_ils_button() const;

   virtual void push_ils_button();

   virtual binary_switch get_mcp_switch(mcp_switch sw) const;

   virtual void push_mcp_switch(mcp_switch sw);

   virtual nd_mode_switch get_nd_mode_switch() const;

   virtual void set_nd_mode_switch(nd_mode_switch mode);

   virtual nd_range_switch get_nd_range_switch() const;

   virtual void set_nd_range_switch(nd_range_switch range);

   virtual nd_nav_mode_switch get_nd_nav1_mode_switch() const;

   virtual void set_nd_nav1_mode_switch(nd_nav_mode_switch value);

   virtual nd_nav_mode_switch get_nd_nav2_mode_switch() const;

   virtual void set_nd_nav2_mode_switch(nd_nav_mode_switch value);

private:

   inline binary_switch invert(binary_switch value)
   { return value == SWITCHED_ON ? SWITCHED_OFF : SWITCHED_ON; }

   ptr<local_fsuipc> _fsuipc;
};


}} // namespace oac

#endif // OAC_WE_WILCO_EFIS_H
