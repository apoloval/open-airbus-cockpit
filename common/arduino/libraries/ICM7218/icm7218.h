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

#ifndef OAC_ICM7218
#define OAC_ICM7218

#include "Arduino.h"

class ICM7218
{
public:

  class DisplayGroup {
  public:
  
    DisplayGroup();
    
    void setParent(ICM7218* parent);
    
    void setDisplays(int displays[8]);
    
    void displayNumber(unsigned long number);
    
    void displayDash();
    
  private:
  
    byte _displays[8];
    ICM7218* _parent;
  
  };

  ICM7218();
  
  void setIdPins(int idPin[8]);
  
  void setWritePin(int pin);
  
  void setModePin(int pin);
  
  void displaySingleDigit(unsigned int  digit, unsigned int  digitValue);
  
  void displayDigits(byte digits[8]);
  
private:

  byte _idPin[8];  
  byte _writePin;
  byte _modePin;
  
  byte _cache[8];
  
  void sendWrite();  
  
  void sendFullDataComing();

};

#endif

