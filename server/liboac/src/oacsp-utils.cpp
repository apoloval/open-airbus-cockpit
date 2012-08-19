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

#include "oacsp-utils.h"

namespace oac {

void
SerialProtocolManager::sendReset()
{
   Command cmd;
   cmd.type = CMD_RESET;
   sendCommand(cmd);   
}

void
SerialProtocolManager::sendWriteVar(word offset, word value)
{
   Command cmd;
   cmd.type = CMD_WRITE_VAR;
   cmd.writeVar.offset = offset;
   cmd.writeVar.data = value;
   sendCommand(cmd);   
}
   
void
SerialProtocolManager::sendCommand(const Command& cmd)
{
   _serialDevice->write(&(cmd.type), 1);
   switch (cmd.type)
   {
      case CMD_PING:
      case CMD_PONG:
         _serialDevice->write(&(cmd.ping.deviceType), 1);
         _serialDevice->write(&(cmd.ping.majorVersion), 1);
         _serialDevice->write(&(cmd.ping.minorVersion), 1);
         _serialDevice->write(&(cmd.ping.padding), 12);
         break;
      case CMD_RESET:
         _serialDevice->write(&(cmd.reset.padding), 15);
         break;
      case CMD_WRITE_VAR:
         _serialDevice->write(&(cmd.writeVar.offset), 2);
         _serialDevice->write(&(cmd.writeVar.data), 2);
         _serialDevice->write(&(cmd.writeVar.padding), 11);
         break;
      case CMD_NOTIFY_EVENT:
         _serialDevice->write(&(cmd.notifyEvent.event), 2);
         _serialDevice->write(&(cmd.notifyEvent.param1), 2);
         _serialDevice->write(&(cmd.notifyEvent.param2), 2);
         _serialDevice->write(&(cmd.notifyEvent.param3), 2);
         _serialDevice->write(&(cmd.notifyEvent.param4), 2);
         _serialDevice->write(&(cmd.notifyEvent.padding), 5);
         break;
   }
}

}; //namespace oac
