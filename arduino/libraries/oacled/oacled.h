/*
 * Open Airbus Cockpit - Arduino LED Driver library
 * Copyright (c) 2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OAC_LED_H
#define OAC_LED_H

#include <arduino.h>

#define MAX7219_SIGNAL_DELAY       1
#define MAX7219_NOOP_ADDR          0x00
#define MAX7219_DIGIT0_ADDR        0x01
#define MAX7219_DIGIT1_ADDR        0x02
#define MAX7219_DIGIT2_ADDR        0x03
#define MAX7219_DIGIT3_ADDR        0x04
#define MAX7219_DIGIT4_ADDR        0x05
#define MAX7219_DIGIT5_ADDR        0x06
#define MAX7219_DIGIT6_ADDR        0x07
#define MAX7219_DIGIT7_ADDR        0x08
#define MAX7219_DECODE_MODE_ADDR   0x09
#define MAX7219_INTENSITY_ADDR     0x0a
#define MAX7219_SCAN_LIMIT_ADDR    0x0b
#define MAX7219_SHUTDOWN_ADDR      0x0c
#define MAX7219_DISPLAY_TEST_ADDR  0x0f

#define MAX7219_NO_DECODE          0
#define MAX7219_CODE_B             1

namespace OAC {

template <int NCHIPS>
class Max7219 {
public:

   class Chip {
   public:
     
      friend class Max7219;
     
      void setDigit(byte digit, byte value, boolean dotPoint = false) {
         if (digit > 8) { digit = 8; }
         if (bitRead(_decode_mode, digit) && dotPoint) { value |= 0x80; }
         _parent->writeRegister(MAX7219_DIGIT0_ADDR + digit, value, _index);
      }
       
      void writeInt(unsigned long num, byte from = 0, byte digits = 8) {
         for (int i = from + digits - 1; i >= from; i--) {
           byte digit = num % 10;
           setDigit(i, digit);
           num /= 10;
         }
      }
     
      void writeFloat(float num, byte precision, byte from = 0, byte digits = 8) {
         unsigned long dec = num * pow(10, precision);
         for (int i = from + digits - 1; i >= from; i--) {
           byte digit = dec % 10;
           boolean require_dot = i == (from + digits - precision - 1);
           setDigit(i, digit, require_dot);
           dec /= 10;
         }
      }
       
      void setIntensity(float value) {
         if (value > 1.0f) { value = 1.0f; }
         if (value < 0.0f) { value = 0.0f; }
         byte data = 0x0f * value;
         _parent->writeRegister(MAX7219_INTENSITY_ADDR, data, _index);
      }
       
      void setScanLimit(byte limit) {
         if (limit > 8) { limit = 8; }
         _parent->writeRegister(MAX7219_SCAN_LIMIT_ADDR, limit, _index);
      }
       
      void setDecodeMode(byte digit, byte mode) {
         setDigitDecodeMode(digit, mode);
         _parent->writeRegister(MAX7219_DECODE_MODE_ADDR, _decode_mode, _index);
      }
       
      void setAllDecodeMode(byte mode) {
         for (int digit = 0; digit < 8; digit++) {
           setDigitDecodeMode(digit, mode);
         }
         _parent->writeRegister(MAX7219_DECODE_MODE_ADDR, _decode_mode, _index);
      }
       
      void start() {
         _parent->writeRegister(MAX7219_SHUTDOWN_ADDR, 1, _index);
      }
       
      void shutdown() {
         _parent->writeRegister(MAX7219_SHUTDOWN_ADDR, 0, _index);
      }
       
      void displayTest(boolean active = true) {
         _parent->writeRegister(MAX7219_DISPLAY_TEST_ADDR, active ? 1 : 0, _index);
      }
     
   private:
  
      Chip() : _parent(0), _index(0), _decode_mode(0) {}
       
      void init(Max7219* parent, byte index) { 
         _parent = parent;
         _index = index; 
      }
       
      void setDigitDecodeMode(byte digit, byte mode) {
         if (mode) { bitSet(_decode_mode, digit); } 
         else { bitClear(_decode_mode, digit); }
      }
       
      Max7219* _parent;
      byte     _index;
      byte     _decode_mode;
   };

   Max7219() : _din(-1), _load(-1), _clock(-1) {
      for (int i = 0; i < NCHIPS; i++) {
         _chips[i].init(this, i);
      }
   }
  
   void setPins(byte din, byte load, byte clock) {
      _din = din;
      _load = load;
      _clock = clock;
    
      pinMode(_din, OUTPUT);
      pinMode(_load, OUTPUT);
      pinMode(_clock, OUTPUT);
    
      digitalWrite(_clock, LOW);
      digitalWrite(_load, HIGH);
    
      for (int i = 0; i < NCHIPS; i++) {
         _chips[i].setIntensity(1.0f);
         _chips[i].setScanLimit(7);
         _chips[i].setAllDecodeMode(MAX7219_CODE_B);
         _chips[i].displayTest(false);
         _chips[i].shutdown();
      }
   }
    
   boolean isConfigured() const {
      return _din != -1 && _load != -1 && _clock != -1;
   }
  
   Chip& get(byte index) {
      return _chips[index];
   }
  
   void serialWrite(word data, byte chip = 0) {
      digitalWrite(_load, LOW);
      serialWriteNoop(NCHIPS - chip - 1);
      shiftOut(_din, _clock, MSBFIRST, (data >> 8));
      shiftOut(_din, _clock, MSBFIRST, data);
      serialWriteNoop(chip);
      digitalWrite(_load, HIGH);
   }
  
   void writeRegister(byte reg, byte data, byte chip = 0) {
      word cmd = (word(reg) << 8) | word(data);
      serialWrite(cmd, chip);
   }
    
private:

   byte _din;
   byte _load;
   byte _clock;
   Chip _chips[NCHIPS];
  
   void serialWriteNoop(byte times) {
      for (int i = 0; i < times; i++) {
         shiftOut(_din, _clock, MSBFIRST, 0); // register 0x00
         shiftOut(_din, _clock, MSBFIRST, 0); // null data
      }
   }
};

}

#endif
