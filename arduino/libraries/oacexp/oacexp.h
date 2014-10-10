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

   ExpansionCard() : prevInput(0), currentInput(0) {}

   /** Set the pins to be used by this expansion card. */
   void setPins(int i0, int i1, int i2, int o0, int o1, int o2) {
      input.setPins(i0, i1, i2);
      output.setPins(o0, o1, o2);
   }

   /** 
    * Read the input from the shift in register.
    * 
    * This reads one byte from the shift-in register, store that byte 
    * internally and returns it.
    */
   byte readInput() {
      input.parallelIn();
      prevInput = currentInput;
      currentInput = input.shiftByteIn();
      return currentInput;
   }

   /** The input lines that have been activated in last read. */
   byte inputActivated() {
      return (prevInput ^ currentInput) & currentInput;
   }

   /** The input lines that have been deactivated in last read. */
   byte inputDeactivated() {
      return (prevInput ^ currentInput) & prevInput;
   }

   /** Write the given byte to the shift-out register. */
   void writeOutput(byte out) {
      output.shiftByteOut(out);
   }

private:

   Shift4021 input;
   Shift595 output;

   byte prevInput;
   byte currentInput;
};

} // namespace OAC

#endif
