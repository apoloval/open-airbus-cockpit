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

#include "icm7218.h"
#include "Arduino.h"

#define BANKSELECT_PIN(ci) (ci->_idPin[3])
#define SHUTDOWN_PIN(ci)   (ci->_idPin[4])
#define DECODE_PIN(ci)     (ci->_idPin[5])
#define HEXCODEB_PIN(ci)   (ci->_idPin[6])
#define DATACOMING_PIN(ci) (ci->_idPin[7])

ICM7218::ICM7218() : _isDirty(false)
{
   for (int i = 0; i < 8; i++)
      _cache[i] = 0xff;
}

void
ICM7218::setIdPins(int idPin[8])
{
   for (int i = 0; i < 8; i++)
   {
      _idPin[i] = idPin[i];
      pinMode(_idPin[i], OUTPUT);
   }
}

void
ICM7218::setModePin(int pin)
{
   _modePin = pin;
   pinMode(_modePin, OUTPUT);
}

void
ICM7218::setWritePin(int pin)
{
   _writePin = pin;
   pinMode(_writePin, OUTPUT);
  
   // WRITE must remain high from now on
   digitalWrite(_writePin, HIGH);
}

void
ICM7218::loadSingleDigit(unsigned int digit, unsigned int digitValue)
{
   if (digit < 8)
      _cache[digit] = digitValue;
   _isDirty = true;
}

void
ICM7218::loadDigits(byte digits[8])
{
   for (int i = 0; i < 8; i++)
      _cache[i] = digits[i];
   _isDirty = true;
}

void
ICM7218::displaySingleDigit(unsigned int  digit, unsigned int  digitValue)
{
   if (digit < 1 || digit > 8)
      return;
   digitalWrite(_modePin, HIGH);
   digitalWrite(DATACOMING_PIN(this), LOW);
   digitalWrite(SHUTDOWN_PIN(this), HIGH);
   digitalWrite(DECODE_PIN(this), LOW);
   digitalWrite(HEXCODEB_PIN(this), LOW);
   digitalWrite(_idPin[0], B0001 & (digit - 1));
   digitalWrite(_idPin[1], B0010 & (digit - 1));
   digitalWrite(_idPin[2], B0100 & (digit - 1));
   digitalWrite(_idPin[3], B1000 & (digit - 1));
   sendWrite();
  
   digitalWrite(_modePin, LOW);
   digitalWrite(_idPin[0], B0001 & digitValue);
   digitalWrite(_idPin[1], B0010 & digitValue);
   digitalWrite(_idPin[2], B0100 & digitValue);
   digitalWrite(_idPin[3], B1000 & digitValue);
   sendWrite();  
}

void
ICM7218::displayDigits(byte digits[8])
{
   sendFullDataComing();
  
   digitalWrite(_modePin, LOW);
   for (int i = 0; i < 8; i++)
   {
      digitalWrite(_idPin[0], B0001 & digits[i]);
      digitalWrite(_idPin[1], B0010 & digits[i]);
      digitalWrite(_idPin[2], B0100 & digits[i]);
      digitalWrite(_idPin[3], B1000 & digits[i]);
      digitalWrite(_idPin[7], HIGH);
      sendWrite();  
   }
}

void
ICM7218::display()
{
   if (_isDirty)
   {
      displayDigits(_cache);
      _isDirty = false;
   }
}

void
ICM7218::sendWrite()
{
   digitalWrite(_writePin, LOW);
   digitalWrite(_writePin, HIGH);
}

void
ICM7218::sendFullDataComing()
{
   digitalWrite(_modePin, HIGH);
   digitalWrite(DATACOMING_PIN(this), HIGH);
   digitalWrite(BANKSELECT_PIN(this), HIGH);
   digitalWrite(SHUTDOWN_PIN(this), HIGH);
   digitalWrite(DECODE_PIN(this), LOW);
   digitalWrite(HEXCODEB_PIN(this), LOW);
   sendWrite();
}

ICM7218::DisplayGroup::DisplayGroup()
 : _parent(NULL)
{
   for (int i = 0; i < 8; i++)
      _displays[i] = i + 1;
}

void
ICM7218::DisplayGroup::setParent(ICM7218* parent)
{ _parent = parent; }

void
ICM7218::DisplayGroup::setDisplays(int displays[8])
{
   for (int i = 0; i < 8; i++)
      _displays[i] = byte(displays[i]);
}

void
ICM7218::DisplayGroup::loadNumber(unsigned long number)
{
   byte digits[8];

   byte values[8];
   for (int i = 0; i < 8; i++)
   {
      values[i] = number % 10;
      number /= 10;
   }
   
   for (int i = 0; i < 8; i++)
   {
      if (_displays[i] <= 8)
         digits[i] = values[_displays[i]];
      else
         digits[i] = _parent->_cache[i];
   }
   _parent->loadDigits(digits);
}

void
ICM7218::DisplayGroup::loadDash()
{
   byte digits[8];
   for (int i = 0; i < 8; i++)
      digits[i] = (_displays[i] <= 8) ? 0x0a : _parent->_cache[i];
   _parent->loadDigits(digits);
}


