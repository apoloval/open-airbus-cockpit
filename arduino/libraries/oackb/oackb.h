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

/** 
 * A keypad controlled by OAC Keypad expansion cards. 
 * 
 * Objects of this class manage an array of Open Airbus Cockpit Keypad 
 * Expansion Cards. Each expansion card comprises two MM74C922 controllers,
 * each one handling 16 keys. These controllers are connected in a common
 * 4-bits data bus with independent control lines. Each card have up-link
 * connectors for data bus and power line, which makes possible to chain
 * several cards sharing the same data bus. The control line comprises two
 * connections: DAV (Data Available), which is HIGH when the controller
 * wants to send data, and OE (Output Enabled), which is LOW when the
 * Arduino board authorizes the controller to use the bus. 
 * 
 * The complexity of the array this array of expansion cards is hidden by
 * this class. The user code only have to deal with the following primitives.
 * 
 * <ul>
 *   <li>Bus configuration. Using configBus() function, the user indicates what
 *       pins correspond to the shared data bus.
 *   <li>Controller configuration. For each connected controller, the user
 *       indicates what pins correspond to the DAV and the OE signals using
 *       the configController() function. 
 *    <li>Read key. Using readKey() function, the user code can interrogate
 *        what's the currenty pressed key.
 *    <li>Read key type. Using readKeyType() function, the user code can
 *        interrogate what's the last key type (key pressed and then released).
 * </ul>
 *
 * Important note: the keys are numbered in consecutive ranges respect the
 * index of its controller. I.e., CONTROLLER_0 keys goes from 0 to 15, 
 * CONTROLLER_1 goes from 16 to 31, and so. 
 */
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

   /**
    * Read a key type. A key type means a key is pressed and then released. 
    * It returns the typed key, or -1 if no type is detected. 
    */
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
