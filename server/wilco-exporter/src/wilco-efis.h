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

class EFISControlPanelImpl :
      public CockpitFront::EFISControlPanel,
      public DllInspector
{
public:

   EFISControlPanelImpl(const DllInfo& dll_info, HINSTANCE dll_instance);

   virtual BarometricMode getBarometricMode() const;

   virtual void setBarometricMode(BarometricMode mode);

   virtual BarometricFormat getBarometricFormat() const;

   virtual void setBarometricFormat(BarometricFormat fmt);

   virtual BinarySwitch getFDButton() const;

   virtual void pushFDButton();

   virtual BinarySwitch getILSButton() const;

   virtual void pushILSButton();

   virtual BinarySwitch getMCPSwitch(MCPSwitch sw) const;

   virtual void pushMCPSwitch(MCPSwitch sw);

   virtual NDModeSwitch getNDModeSwitch() const;

   virtual void setNDModeSwitch(NDModeSwitch mode);

   virtual NDRangeSwitch getNDRangeSwitch() const;

   virtual void setNDRangeSwitch(NDRangeSwitch range);

   virtual NDNavModeSwitch getNDNav1ModeSwitch() const;

   virtual void setNDNav1ModeSwitch(NDNavModeSwitch value);

   virtual NDNavModeSwitch getNDNav2ModeSwitch() const;

   virtual void setNDNav2ModeSwitch(NDNavModeSwitch value);

private:

   inline BinarySwitch invert(BinarySwitch value)
   { return value == SWITCHED_ON ? SWITCHED_OFF : SWITCHED_ON; }

   Ptr<FSUIPC> _fsuipc;
};


}} // namespace oac

#endif // OAC_WE_WILCO_EFIS_H
