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

#ifdef __WIN32
/* Raw IO is not included in Windows versions previous to WinXP SP2. The
 * following macros should be declared in order to force the included
 * headers files to declare such a types. 
 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#ifndef _WINVER
#define _WINVER 0x0502
#endif
#include <Windows.h>
#include <Winuser.h>
#endif


#include "exception.h"

namespace oac { namespace server {

typedef std::string SerialDeviceName;

struct SerialDeviceInfo
{
   SerialDeviceName name;
};

typedef std::vector<SerialDeviceInfo> SerialDeviceInfoArray;

class SerialDevice;

/**
 * The interface for any object capable of managing serial devices.
 */
class SerialDeviceManager
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

class SerialDevice
{
public:

   inline virtual ~SerialDevice() {}
   
   virtual unsigned int read(void* buf, unsigned int nbytes) 
         throw (IOException) = 0;

   virtual void write(void* buf, unsigned int nbytes) 
         throw (IOException) = 0;
};

#ifdef __WIN32

class Win32SerialDevice : public SerialDevice
{
public:

   Win32SerialDevice(HANDLE handle);
   
   virtual ~Win32SerialDevice();

   virtual unsigned int read(void* buf, unsigned int nbytes) 
         throw (IOException);

   virtual void write(void* buf, unsigned int nbytes) 
         throw (IOException);
   
private:

   HANDLE _handle;
};

class Win32SerialDeviceManager : public SerialDeviceManager
{
public:
   
   virtual void listSerialDevices(SerialDeviceInfoArray& devs) const;
   
   virtual Win32SerialDevice* open(const SerialDeviceInfo& dev) 
         throw (NotFoundException);
};
#endif

}}; // namespace oac::server

#endif
