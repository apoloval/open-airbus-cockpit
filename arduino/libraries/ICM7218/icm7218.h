/*
 * Open Airbus Cockpit - Arduino ICM7218 library
 * Copyright (c) 2012-2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

    void loadUnsigned(unsigned long number);

    void loadSigned(long number);

    void loadDash();

  private:

    byte _displays[8];
    ICM7218* _parent;

    void loadNumber(unsigned long number, bool isPositive, bool loadSign);

  };

  ICM7218();

  void setIdPins(int idPin[8]);

  void setWritePin(int pin);

  void setModePin(int pin);

  void loadSingleDigit(unsigned int digit, unsigned int digitValue);

  void loadDigits(byte digits[8]);

  void displaySingleDigit(unsigned int  digit, unsigned int  digitValue);

  void displayDigits(byte digits[8]);

  void display();

private:

  byte _idPin[8];
  byte _writePin;
  byte _modePin;

  byte _cache[8];
  byte _isDirty;

  void sendWrite();

  void sendFullDataComing();

};

#endif

