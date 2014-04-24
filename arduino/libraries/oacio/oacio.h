/*
 * Open Airbus Cockpit - Arduino IO library
 * Copyright (c) 2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OAC_IO_H
#define OAC_IO_H

namespace OAC {

class AnalogInput {
public:

   AnalogInput(int pin = 0) : _pin(pin), _state(0) {}

   void setPin(int pin) {
      _pin = pin;
   }

   long read() {
      _state = analogRead(_pin);
      return _state;
   }

   long map(long from = 0, long to = 1023) {
      return ::map(read(), 0, 1023, from, to);
   }

   bool isChanged(long tolerance = 16) {
      long prevState = _state;
      long newState = analogRead(_pin);
      if ((prevState <= newState && (newState - prevState) > tolerance) ||
          (prevState > newState && (prevState - newState) > tolerance)) {
         _state = newState;
         return true;
      }
      return false;
   }

private:

   int _pin;
   long _state;
};

}

#endif
