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

#include "serial.h"

#ifdef __WIN32
#include <cstdio>
#endif

namespace oac { namespace server {

SerialDeviceManager&
SerialDeviceManager::getDefault()
{
   static Win32SerialDeviceManager* singleton = NULL;
   if (!singleton)
   {
#ifdef __WIN32
      singleton = new Win32SerialDeviceManager();
#endif
   }
   return *singleton;
}

#ifdef __WIN32

namespace {
   
std::string
deviceNameForPortNumer(unsigned int portNumber)
{
   char name[16];
   if (portNumber < 10)
      sprintf(name, "COM%d", portNumber);  
   else
      sprintf(name, "\\\\.\\COM%d", portNumber);   
   return name;
}

SerialDeviceInfo
deviceForName(const std::string& devName)
{
   SerialDeviceInfo dev;
   dev.name = devName;
   return dev;
}

}; // anonymous namespace

Win32SerialDevice::Win32SerialDevice(HANDLE handle) :
   _handle(handle)
{   
}

Win32SerialDevice::~Win32SerialDevice()
{
   CloseHandle(_handle);
}

void
Win32SerialDevice::setBlocking(bool active)
{
   SetCommMask(_handle, active ? EV_RXCHAR : 0);
   _blocking = active;
}

unsigned int
Win32SerialDevice::read(void* buf, unsigned int nbytes) 
throw (IOException)
{
   DWORD bytesRead;
   while (true)
   {
      if (!ReadFile(_handle, buf, nbytes, &bytesRead, NULL))
         throw IOException("error while reading WIN32 serial device");      
         
      if (!_blocking || (bytesRead > 0 && _blocking))
         break;

      DWORD event;
      if (!WaitCommEvent(_handle, &event, NULL))
         throw IOException(
            "error while blocked for incoming data from serial device");
   }
   return bytesRead;
}

void
Win32SerialDevice::write(void* buf, unsigned int nbytes) 
throw (IOException)
{
   DWORD bytesWritten;
   if (!WriteFile(_handle, buf, nbytes, &bytesWritten, NULL))
      throw IOException("error while writing WIN32 serial device");
}

void
Win32SerialDeviceManager::listSerialDevices(SerialDeviceInfoArray& devs) const
{
   devs.clear();
   for (unsigned int i = 0; i < 256; i++)
   {
      std::string devName = deviceNameForPortNumer(i);
      HANDLE handle = ::CreateFile(devName.c_str(), GENERIC_READ | GENERIC_WRITE,
                                  0, NULL, OPEN_EXISTING, 0, NULL);
      if (handle != INVALID_HANDLE_VALUE) {
         devs.push_back(deviceForName(devName));
         CloseHandle(handle);      
      }
   }
}

Win32SerialDevice*
Win32SerialDeviceManager::open(const SerialDeviceInfo& dev) 
throw (NotFoundException)
{
   HANDLE handle = ::CreateFile(dev.name.c_str(), GENERIC_READ | GENERIC_WRITE,
                                 0, NULL, OPEN_EXISTING, 0, NULL);
   if (handle != INVALID_HANDLE_VALUE) {
      return new Win32SerialDevice(handle);
   }
   else
      throw NotFoundException("cannot open given serial device");
}
   
#endif

}}; // namespace oac::server
