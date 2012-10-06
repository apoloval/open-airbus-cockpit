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

#ifndef OAC_SERVER_SERIAL_H
#define OAC_SERVER_SERIAL_H

#include <string>
#include <vector>

#include <Windows.h>
#include <Winuser.h>

#include <KarenCore/platform.h>
#include <KarenCore/string.h>

#include "config.h"
#include "exception.h"
#include "types.h"

namespace oac {

typedef String SerialDeviceName;

struct LIBOAC_EXPORT SerialDeviceInfo
{
   SerialDeviceName name;
};

typedef std::vector<SerialDeviceInfo> SerialDeviceInfoArray;

class SerialDevice;

/**
 * The interface for any object capable of managing serial devices.
 */
class LIBOAC_EXPORT SerialDeviceManager
{
public:

   /**
    * Obtain default serial device manager object. 
    */
   static SerialDeviceManager& getDefault();

   /**
    * List the serial devices attached to the machine.
    */
   virtual void listSerialDevices(SerialDeviceInfoArray& devs) const = 0;
   
   /**
    * Open given serial device. Throws a NotFoundException if there is
    * no such device.
    */
   virtual SerialDevice* open(const SerialDeviceInfo& dev) 
         throw (NotFoundException) = 0;

};

class LIBOAC_EXPORT SerialDevice
{
public:

   inline virtual ~SerialDevice() {}

   /**
    * Set this serial device to use blocking IO.
    */
   virtual void setBlocking(bool active = true) = 0;
   
   /**
    * Read nbytes from serial device and copy them into buf. 
    */
   virtual unsigned int read(void* buf, unsigned int nbytes) 
         throw (IOException) = 0;

   /**
    * Write nbytes obtained from given buffer to this serial device.
    */
   virtual void write(const void* buf, unsigned int nbytes) 
         throw (IOException) = 0;
};

#if KAREN_PLATFORM == KAREN_PLATFORM_WINDOWS

class LIBOAC_EXPORT Win32SerialDevice : public SerialDevice
{
public:

   Win32SerialDevice(HANDLE handle);
   
   virtual ~Win32SerialDevice();

   virtual void setBlocking(bool active = true);

   virtual unsigned int read(void* buf, unsigned int nbytes) 
         throw (IOException);

   virtual void write(const void* buf, unsigned int nbytes) 
         throw (IOException);
   
private:

   HANDLE _handle;
   bool   _blocking;
};

class LIBOAC_EXPORT Win32SerialDeviceManager : public SerialDeviceManager
{
public:
   
   virtual void listSerialDevices(SerialDeviceInfoArray& devs) const;
   
   virtual Win32SerialDevice* open(const SerialDeviceInfo& dev) 
         throw (NotFoundException);
};
#endif

}; // namespace oac

#endif
