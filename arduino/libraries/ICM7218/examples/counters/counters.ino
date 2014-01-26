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

ICM7218 ci;
ICM7218::DisplayGroup grp1;
ICM7218::DisplayGroup grp2;

void setup() {
  int idPins[] = { 22, 23, 24, 25, 26, 27, 28,  29 };
  ci.setWritePin(30);
  ci.setModePin(31);
  ci.setIdPins(idPins);
  
  int displays1[] = { 0, 1, 2, -1, -1, -1, -1, -1 };
  grp1.setParent(&ci);
  grp1.setDisplays(displays1);

  int displays2[] = { -1, -1, -1, 0, 1, 2, -1, -1 };
  grp2.setParent(&ci);
  grp2.setDisplays(displays2);
}

unsigned long number1 = 0;
unsigned long number2 = 1000;

void loop() {
  grp1.loadNumber(number1);
  grp2.loadNumber(number2);
  ci.display();
  delay(100);
  number1++;
  number2--;
}
