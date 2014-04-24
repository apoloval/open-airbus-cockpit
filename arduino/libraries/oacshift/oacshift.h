/*
 * Open Airbus Cockpit - Arduino Shift Register library
 * Copyright (c) 2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OACSHIFT_H
#define OACSHIFT_H

#include <arduino.h>

namespace OAC {

class Shift4021 {
public:

  void setPins(int clock, int latch, int data) {
    _clockPin = clock;
    _latchPin = latch;
    _dataPin = data;
    pinMode(_clockPin, OUTPUT);
    pinMode(_latchPin, OUTPUT);
    pinMode(_dataPin, INPUT);
  }
  
  void parallelIn() {
    digitalWrite(_latchPin, HIGH);
    delayMicroseconds(20);
    digitalWrite(_latchPin, LOW);
  }

  template <typename Data>
  Data shiftBitsIn(int nbits) {
    Data data = 0;
    for (int i = nbits - 1; i >= 0; i--) {
      data |= Data(digitalRead(_dataPin)) << i;
  
      digitalWrite(_clockPin, HIGH);
      delayMicroseconds(2);
      digitalWrite(_clockPin, LOW);
      delayMicroseconds(2);
    }
    return data;
  }  

  byte shiftByteIn() {
    return shiftBitsIn<byte>(8);
  }
  
  word shiftWordIn() {
    return shiftBitsIn<word>(16);
  }
  
private:

  int _clockPin;
  int _latchPin;
  int _dataPin;  
};

class Shift595 {
public:

  void setPins(int clock, int latch, int data) {
    _clockPin = clock;
    _latchPin = latch;
    _dataPin = data;
    pinMode(_clockPin, OUTPUT);
    pinMode(_latchPin, OUTPUT);
    pinMode(_dataPin, OUTPUT);
  }
  
  void shiftByteOut(byte data) {
    digitalWrite(_latchPin, LOW);
    ::shiftOut(_dataPin, _clockPin, MSBFIRST, data);
    digitalWrite(_latchPin, HIGH);
  }
  
  void shiftWordOut(word data) {
    digitalWrite(_latchPin, LOW);
    ::shiftOut(_dataPin, _clockPin, MSBFIRST, ((byte*)&data)[0]);
    ::shiftOut(_dataPin, _clockPin, MSBFIRST, ((byte*)&data)[1]);
    digitalWrite(_latchPin, HIGH);
  }
  
private:

  int _clockPin;
  int _latchPin;
  int _dataPin;  
};

} // namespace OAC

#endif
