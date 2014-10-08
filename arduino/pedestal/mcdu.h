/*
 * Open Airbus Cockpit - Arduino Pedestal Sketch
 * Copyright (c) 2012-2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Set the pin layout using these macros
#define MCDU_BUS_PINS      2, 3, 4, 5
#define MCDU_CTRL0_PINS    40, 41
#define MCDU_CTRL1_PINS    42, 43
#define MCDU_CTRL2_PINS    44, 45
#define MCDU_CTRL3_PINS    46, 47
#define MCDU_CTRL4_PINS    48, 49


#define MCDU_KEY_L1        0
#define MCDU_KEY_L2        1
#define MCDU_KEY_L3        2
#define MCDU_KEY_L4        3
#define MCDU_KEY_L5        4
#define MCDU_KEY_L6        5
#define MCDU_KEY_R1        6
#define MCDU_KEY_R2        7
#define MCDU_KEY_R3        8
#define MCDU_KEY_R4        9
#define MCDU_KEY_R5        10
#define MCDU_KEY_R6        11

#define MCDU_KEY_DIR       16
#define MCDU_KEY_PROG      17
#define MCDU_KEY_PERF      18
#define MCDU_KEY_INIT      19
#define MCDU_KEY_DATA      20
#define MCDU_KEY_FPLN      21
#define MCDU_KEY_RADNAV    22
#define MCDU_KEY_FUELPRED  23
#define MCDU_KEY_SECFPLN   24
#define MCDU_KEY_FIX       25
#define MCDU_KEY_MCDUMENU  26
#define MCDU_KEY_AIRPORT   27
#define MCDU_KEY_UP        28
#define MCDU_KEY_NEXTPAGE  29
#define MCDU_KEY_DOWN      30

#define MCDU_KEY_1         32
#define MCDU_KEY_2         33
#define MCDU_KEY_3         34
#define MCDU_KEY_4         35
#define MCDU_KEY_5         36
#define MCDU_KEY_6         37
#define MCDU_KEY_7         38
#define MCDU_KEY_8         39
#define MCDU_KEY_9         40
#define MCDU_KEY_DOT       41
#define MCDU_KEY_0         42
#define MCDU_KEY_SLASH     43

#define MCDU_KEY_A         48
#define MCDU_KEY_B         49
#define MCDU_KEY_C         50
#define MCDU_KEY_D         51
#define MCDU_KEY_E         52
#define MCDU_KEY_F         53
#define MCDU_KEY_G         54
#define MCDU_KEY_H         55
#define MCDU_KEY_I         56
#define MCDU_KEY_J         57
#define MCDU_KEY_K         58
#define MCDU_KEY_L         59
#define MCDU_KEY_M         60
#define MCDU_KEY_N         61
#define MCDU_KEY_O         62

#define MCDU_KEY_P         64
#define MCDU_KEY_Q         65
#define MCDU_KEY_R         66
#define MCDU_KEY_S         67
#define MCDU_KEY_T         68
#define MCDU_KEY_U         69
#define MCDU_KEY_V         70
#define MCDU_KEY_W         71
#define MCDU_KEY_X         72
#define MCDU_KEY_Y         73
#define MCDU_KEY_Z         74
#define MCDU_KEY_MINUS     75
#define MCDU_KEY_PLUS      76
#define MCDU_KEY_OVFY      77
#define MCDU_KEY_CLR       78

struct Mcdu {

   OAC::Keyboard kb;

   void setup() {
      kb.configBus(MCDU_BUS_PINS);
      kb.configController(OAC::Keypad::CONTROLLER_0, MCDU_CTRL0_PINS);
      kb.configController(OAC::Keypad::CONTROLLER_1, MCDU_CTRL1_PINS);
      kb.configController(OAC::Keypad::CONTROLLER_2, MCDU_CTRL2_PINS);
      kb.configController(OAC::Keypad::CONTROLLER_3, MCDU_CTRL3_PINS);
      kb.configController(OAC::Keypad::CONTROLLER_4, MCDU_CTRL4_PINS);
   }

   void loop() {
      short key = kb.readKeyType();
      switch (key) {
         case MCDU_KEY_L1: OACSP.writeLVar("MCDU_LSK1L", 1); break;
         case MCDU_KEY_L2: OACSP.writeLVar("MCDU_LSK2L", 1); break;
         case MCDU_KEY_L3: OACSP.writeLVar("MCDU_LSK3L", 1); break;
         case MCDU_KEY_L4: OACSP.writeLVar("MCDU_LSK4L", 1); break;
         case MCDU_KEY_L5: OACSP.writeLVar("MCDU_LSK5L", 1); break;
         case MCDU_KEY_L6: OACSP.writeLVar("MCDU_LSK6L", 1); break;
         case MCDU_KEY_R1: OACSP.writeLVar("MCDU_LSK1R", 1); break;
         case MCDU_KEY_R2: OACSP.writeLVar("MCDU_LSK2R", 1); break;
         case MCDU_KEY_R3: OACSP.writeLVar("MCDU_LSK3R", 1); break;
         case MCDU_KEY_R4: OACSP.writeLVar("MCDU_LSK4R", 1); break;
         case MCDU_KEY_R5: OACSP.writeLVar("MCDU_LSK5R", 1); break;
         case MCDU_KEY_R6: OACSP.writeLVar("MCDU_LSK6R", 1); break;

         case MCDU_KEY_DIR: OACSP.writeLVar("MCDU1_DIR_key", 1); break;
         case MCDU_KEY_PROG: OACSP.writeLVar("MCDU1_PROG_key", 1); break;
         case MCDU_KEY_PERF: OACSP.writeLVar("MCDU1_PERF_key", 1); break;
         case MCDU_KEY_INIT: OACSP.writeLVar("MCDU1_INIT_key", 1); break;
         case MCDU_KEY_DATA: OACSP.writeLVar("MCDU1_DATA_key", 1); break;
         case MCDU_KEY_FPLN: OACSP.writeLVar("MCDU1_FPLN_key", 1); break;
         case MCDU_KEY_RADNAV: OACSP.writeLVar("MCDU1_RAD_key", 1); break;
         case MCDU_KEY_FUELPRED: OACSP.writeLVar("MCDU1_FUEL_key", 1); break;
         case MCDU_KEY_SECFPLN: OACSP.writeLVar("MCDU1_SPLN_key", 1); break;
         case MCDU_KEY_FIX: break;
         case MCDU_KEY_MCDUMENU: OACSP.writeLVar("MCDU1_MENU_key", 1); break;
         case MCDU_KEY_AIRPORT: OACSP.writeLVar("MCDU1_ARPRT_key", 1); break;
         case MCDU_KEY_UP: OACSP.writeLVar("MCDU_arrowup", 1); break;
         case MCDU_KEY_NEXTPAGE: OACSP.writeLVar("MCDU_arrowright", 1); break;
         case MCDU_KEY_DOWN: OACSP.writeLVar("MCDU_arrowdn", 1); break;

         case MCDU_KEY_1: OACSP.writeLVar("MCDU_1", 1); break;
         case MCDU_KEY_2: OACSP.writeLVar("MCDU_2", 1); break;
         case MCDU_KEY_3: OACSP.writeLVar("MCDU_3", 1); break;
         case MCDU_KEY_4: OACSP.writeLVar("MCDU_4", 1); break;
         case MCDU_KEY_5: OACSP.writeLVar("MCDU_5", 1); break;
         case MCDU_KEY_6: OACSP.writeLVar("MCDU_6", 1); break;
         case MCDU_KEY_7: OACSP.writeLVar("MCDU_7", 1); break;
         case MCDU_KEY_8: OACSP.writeLVar("MCDU_8", 1); break;
         case MCDU_KEY_9: OACSP.writeLVar("MCDU_9", 1); break;
         case MCDU_KEY_DOT: OACSP.writeLVar("MCDU_ST", 1); break;
         case MCDU_KEY_0: OACSP.writeLVar("MCDU_0", 1); break;
         case MCDU_KEY_SLASH: OACSP.writeLVar("MCDU_SL", 1); break;

         case MCDU_KEY_A: OACSP.writeLVar("MCDU_A", 1); break;
         case MCDU_KEY_B: OACSP.writeLVar("MCDU_B", 1); break;
         case MCDU_KEY_C: OACSP.writeLVar("MCDU_C", 1); break;
         case MCDU_KEY_D: OACSP.writeLVar("MCDU_D", 1); break;
         case MCDU_KEY_E: OACSP.writeLVar("MCDU_E", 1); break;
         case MCDU_KEY_F: OACSP.writeLVar("MCDU_F", 1); break;
         case MCDU_KEY_G: OACSP.writeLVar("MCDU_G", 1); break;
         case MCDU_KEY_H: OACSP.writeLVar("MCDU_H", 1); break;
         case MCDU_KEY_I: OACSP.writeLVar("MCDU_I", 1); break;
         case MCDU_KEY_J: OACSP.writeLVar("MCDU_J", 1); break;
         case MCDU_KEY_K: OACSP.writeLVar("MCDU_K", 1); break;
         case MCDU_KEY_L: OACSP.writeLVar("MCDU_L", 1); break;
         case MCDU_KEY_M: OACSP.writeLVar("MCDU_M", 1); break;
         case MCDU_KEY_N: OACSP.writeLVar("MCDU_N", 1); break;
         case MCDU_KEY_O: OACSP.writeLVar("MCDU_O", 1); break;

         case MCDU_KEY_P: OACSP.writeLVar("MCDU_P", 1); break;
         case MCDU_KEY_Q: OACSP.writeLVar("MCDU_Q", 1); break;
         case MCDU_KEY_R: OACSP.writeLVar("MCDU_R", 1); break;
         case MCDU_KEY_S: OACSP.writeLVar("MCDU_S", 1); break;
         case MCDU_KEY_T: OACSP.writeLVar("MCDU_T", 1); break;
         case MCDU_KEY_U: OACSP.writeLVar("MCDU_U", 1); break;
         case MCDU_KEY_V: OACSP.writeLVar("MCDU_V", 1); break;
         case MCDU_KEY_W: OACSP.writeLVar("MCDU_W", 1); break;
         case MCDU_KEY_X: OACSP.writeLVar("MCDU_X", 1); break;
         case MCDU_KEY_Y: OACSP.writeLVar("MCDU_Y", 1); break;
         case MCDU_KEY_Z: OACSP.writeLVar("MCDU_Z", 1); break;
         case MCDU_KEY_MINUS: OACSP.writeLVar("MCDU_SGN", 1); break;
         case MCDU_KEY_PLUS: OACSP.writeLVar("MCDU_SGN", 1); break;
         case MCDU_KEY_OVFY: break;
         case MCDU_KEY_CLR: OACSP.writeLVar("MCDU_CLR", 1); break;
         default: 
           if (key >= 0) {
             OACSP.writeLVar("MCDU_UNKNOWN", key); 
           }
           break;
      }
   }

} mcdu;
