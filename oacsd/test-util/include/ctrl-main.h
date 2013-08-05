/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012, 2013 Alvaro Polo
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

#ifndef OAC_TESTUTIL_CTRLMAIN_H
#define OAC_TESTUTIL_CTRLMAIN_H

#include <devices.h>
#include <list>
#include <serial.h>

namespace oac { namespace testutil {

class MainController
{
public:

   struct DevInfo
   {
      bool connected;
      SerialDevice* serialDevice;

      inline DevInfo() : connected(false), serialDevice(nullptr) {}
   };

   struct FCUDevInfo : DevInfo
   {
      FlightControlUnit* fcu;

      inline FCUDevInfo() : DevInfo(), fcu(nullptr) {}
   };

   MainController(SerialDeviceManager* serialDeviceManager = NULL);

   inline const FCUDevInfo& fcuDevInfo() const
   { return _fcuDevInfo; }

   void connectFcu(const SerialDeviceName& devName);

   void disconnectFcu();

private:

   struct FCUDevInfo _fcuDevInfo;
   SerialDeviceManager* _devManager;

};

}}; // namespace oac::testutil

#endif
