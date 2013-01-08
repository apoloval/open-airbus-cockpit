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

#ifndef OAC_COCKPIT_FSUIPC_H
#define OAC_COCKPIT_FSUIPC_H

#include "buffer.h"
#include "cockpit.h"
#include "fsuipc.h"
#include "pointer.h"

namespace oac {

class FSUIPCCockpitBack : public CockpitBack
{
public:

   inline FSUIPCCockpitBack(const Ptr<FSUIPC::Factory>& fsuipc_fact) :
      _fsuipc_fact(fsuipc_fact)
   {}

   virtual void syncUp() throw (SyncException);
   virtual void syncDown() throw (SyncException);
   virtual FlightControlUnit& getFlightControlUnit() throw (SyncException);
   virtual EFISControlPanel& getEFISControlPanel() throw (SyncException);

private:

   Ptr<DoubleBuffer> _buffer;
   Ptr<FSUIPC::Factory> _fsuipc_fact;
   Ptr<FSUIPC> _fsuipc;
   Ptr<FlightControlUnit> _fcu;
   Ptr<EFISControlPanel> _efis_ctrl_panel;

   void initBuffer();
   void initFSUIPC() throw (SyncException);
   void initFCU() throw (SyncException);
   void initEFISControlPanel() throw (SyncException);
   bool isSync() const;
};

}; // namespace oac

#endif
