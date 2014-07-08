/*
 * Open Airbus Cockpit - Arduino IO library
 * Copyright (c) 2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OAC_KB_H
#define OAC_KB_H

#include <arduino.h>

namespace OAC {

class Keypad {
public:

   enum ControllerIndex {
      CONTROLLER_0 = 0,
      CONTROLLER_1 = 1,
      CONTROLLER_2 = 2,
      CONTROLLER_3 = 3,
      CONTROLLER_4 = 4,
      CONTROLLER_5 = 5,
      CONTROLLER_6 = 6,
      CONTROLLER_7 = 7,
   };

   Keypad() : status(0), lastKey(-1) {
      bus.a = bus.b = bus.c = bus.d = -1;
   }

   /** Return true if the bus was successfully configured. */
   bool busIsConfigured() const {
      return bus.configured;
   }

   /** 
    * Configure the bus to use the given pins as A, B, C & D data lines. 
    * 
    * Return true if configuration was successful, false otherwise. 
    */
   bool configBus(short a, short b, short c, short d) {
      if (a < 0 || b < 0 || c < 0 || d < 0) {
         return false;
      }
      bus.configured = true;
      bus.a = a;
      bus.b = b;
      bus.c = c;
      bus.d = d;

      pinMode(bus.a, INPUT);
      pinMode(bus.b, INPUT);
      pinMode(bus.c, INPUT);
      pinMode(bus.d, INPUT);

      return true;
   }

   /** 
    * Configure the bus to use the pins from firstPin onwards. 
    * 
    * Return true if configuration was successful, false otherwise. 
    */
   bool configBus(short firstPin) {
      return configBus(firstPin, firstPin + 1, firstPin + 2, firstPin + 3);
   }

   /**
    * Configure the controller at given index.
    * 
    * Configure the controller placed at given index to use the given DAV (data
    * available) and OE (output enabled) pins. Return true if the controller
    * cannot be configured (e.g. due to invalid index or pins), false otherwise. 
    */
   bool configController(byte index, short dav, short oe) {
      if (index > 8 || dav < 0 || oe < 0) {
        return false;
      }
      status |= (1 << index);
      controller[index].davPin = dav;
      controller[index].oePin = oe;

      pinMode(dav, INPUT);
      pinMode(oe, OUTPUT);
      digitalWrite(oe, HIGH);

      return true;
   }

   /**
    * Read the currently pressed key, or -1 if no key is pressed or 
    * the bus is not properly configured. 
    */
   short readKey() {
      if (busIsConfigured()) {
         for (short i = 0; i < 8; i++) {
            boolean isActive = status & (1 << i);
            if (isActive) {
               int dav = digitalRead(controller[i].davPin);
               if (dav == HIGH) {
                  digitalWrite(controller[i].oePin, LOW);
                  delay(50); // give time to the controller to activate the bus
                  
                  short result = 0;
                  result |= digitalRead(bus.a) << 0;
                  result |= digitalRead(bus.b) << 1;
                  result |= digitalRead(bus.c) << 2;
                  result |= digitalRead(bus.d) << 3;
                  result += i * 0x10;
                  
                  digitalWrite(controller[i].oePin, HIGH);
                  // delay(50);

                  return result;
               }
            }
         }
      }
      return -1;
   }

   short readKeyType() {
      short key = readKey();
      short result = (key < 0 && lastKey >= 0) ? lastKey : -1;
      lastKey = key;
      return result;
   }

private:

   struct Controller {
      short davPin;
      short oePin;
   };

   struct Bus {
      boolean configured;
      short a;
      short b;
      short c;
      short d;
   };

   byte status;
   Bus bus;
   Controller controller[8];
   short lastKey;
};

typedef Keypad Keyboard;

}

#endif
