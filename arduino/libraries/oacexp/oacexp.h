/*
 * Open Airbus Cockpit - Arduino Expansion Card library
 * Copyright (c) 2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OACEXP_H
#define OACEXP_H

#include <oacshift.h>

namespace OAC {

class ExpansionCard {

public:

   ExpansionCard() : lastInput(0) {}

   void setPins(int i0, int i1, int i2, int o0, int o1, int o2) {
      input.setPins(i0, i1, i2);
      output.setPins(o0, o1, o2);
   }

   byte readInput() {
      input.parallelIn();
      lastInput = input.shiftByteIn();
      return lastInput;
   }

   byte readActive() {      
      byte prevInput = lastInput;
      readInput();
      return (lastInput ^ prevInput) & lastInput;
   }

   void writeOutput(byte out) {
      output.shiftByteOut(out);
   }

private:

   Shift4021 input;
   Shift595 output;

   byte lastInput;
};

} // namespace OAC

#endif
