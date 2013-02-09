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

#include <test.h>

#include "ctrl-main.h"

namespace oac { namespace testutil {

MainController::MainController(SerialDeviceManager *serialDeviceManager)
   : _devManager(serialDeviceManager
                          ? serialDeviceManager
                          : &SerialDeviceManager::getDefault())
{
}

void
MainController::connectFcu(const SerialDeviceName &devName)
{
   if (!_fcuDevInfo.connected)
   {
      SerialDeviceInfo devInfo = { devName };
      _fcuDevInfo.serialDevice =_devManager->open(devInfo);
      _fcuDevInfo.fcu = new TestFlightControlUnit();
      _fcuDevInfo.connected = true;
   }
}

void
MainController::disconnectFcu()
{
   if (_fcuDevInfo.connected)
   {
      delete _fcuDevInfo.serialDevice;
      delete _fcuDevInfo.fcu;
      _fcuDevInfo.serialDevice = nullptr;
      _fcuDevInfo.fcu = nullptr;
      _fcuDevInfo.connected = false;
   }
}

}}; // namespace oac::testutil
