/*
 * Open Airbus Cockpit - Arduino Pedestal Sketch
 * Copyright (c) 2012-2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define ECAM_UPPER_BRIGHT 0
#define ECAM_LOWER_BRIGHT 1

#define ECAM_TOCFG_BTN    0x0001
#define ECAM_ENG_BTN      0x0002
#define ECAM_BLEED_BTN    0x0004
#define ECAM_PRESS_BTN    0x0008
#define ECAM_ELEC_BTN     0x0010
#define ECAM_HYD_BTN      0x0020
#define ECAM_FUEL_BTN     0x0040
#define ECAM_APU_BTN      0x0080
#define ECAM_COND_BTN     0x0100
#define ECAM_DOOR_BTN     0x0200
#define ECAM_WHEEL_BTN    0x0400
#define ECAM_FCTL_BTN     0x0800
#define ECAM_ALL_BTN      0x1000
#define ECAM_CLR_BTN      0x2000
#define ECAM_STS_BTN      0x4000
#define ECAM_RCL_BTN      0x8000

#define ECAM_ENG_LGT      0x0001
#define ECAM_BLEED_LGT    0x0002
#define ECAM_PRESS_LGT    0x0004
#define ECAM_ELEC_LGT     0x0008
#define ECAM_HYD_LGT      0x0010
#define ECAM_FUEL_LGT     0x0020
#define ECAM_APU_LGT      0x0040
#define ECAM_COND_LGT     0x0080
#define ECAM_DOOR_LGT     0x0100
#define ECAM_WHEEL_LGT    0x0200
#define ECAM_FCTL_LGT     0x0400
#define ECAM_LCLR_LGT     0x0800
#define ECAM_STS_LGT      0x1000
#define ECAM_RCLR_LGT     0x2000
#define ECAM_CLR_LGT      0x2800

struct {
  OAC::ExpansionCard card0;
  OAC::ExpansionCard card1;
  word lights;
  word buttons;
  OAC::AnalogInput upperBright;
  OAC::AnalogInput lowerBright;
  
  void setup() {
    lights = 0;
    buttons = 0;
    upperBright.setPin(ECAM_UPPER_BRIGHT);
    lowerBright.setPin(ECAM_LOWER_BRIGHT);
    card0.setPins(28, 30, 32, 34, 36, 38);
    card1.setPins(29, 31, 33, 35, 37, 39);
    
    OACSP.observeLVar("AB_ECAM_CLR_Light");
    OACSP.observeLVar("ECAM_MODE");
  }
  
  void loop() {
    processInputs();
    processOutputs();
  }
  
  inline void processInputs() {
    // First check what buttons have been actived
    word active = card0.readActive() | (word(card1.readActive()) << 8);
    switch (active) {
      case ECAM_ENG_BTN: 
        OACSP.writeLVar("ECAM_MODE", 1); break;
      case ECAM_BLEED_BTN:
        OACSP.writeLVar("ECAM_MODE", 2); break;
      case ECAM_PRESS_BTN:
        OACSP.writeLVar("ECAM_MODE", 3); break;
      case ECAM_ELEC_BTN:
        OACSP.writeLVar("ECAM_MODE", 4); break;
      case ECAM_HYD_BTN:
        OACSP.writeLVar("ECAM_MODE", 5); break;
      case ECAM_FUEL_BTN:
        OACSP.writeLVar("ECAM_MODE", 6); break;
      case ECAM_APU_BTN:
        OACSP.writeLVar("ECAM_MODE", 7); break;
      case ECAM_COND_BTN:
        OACSP.writeLVar("ECAM_MODE", 8); break;
      case ECAM_DOOR_BTN:
        OACSP.writeLVar("ECAM_MODE", 9); break;
      case ECAM_WHEEL_BTN:
        OACSP.writeLVar("ECAM_MODE", 10); break;
      case ECAM_FCTL_BTN:
        OACSP.writeLVar("ECAM_MODE", 11); break;
      case ECAM_CLR_BTN:
        OACSP.writeLVar("AB_ECAM_CLR", 1); break;
      case ECAM_STS_BTN:
        break; // not implemented
      case ECAM_RCL_BTN:
        OACSP.writeLVar("AB_ECAM_RCL", 1); break;
    }
    
    // Now check what buttons are pressed
    word prevButtons = buttons;
    buttons = card0.readInput() | (word(card1.readInput()) << 8);
    switch (buttons) {
      case 0:
        if (prevButtons & ECAM_TOCFG_BTN) {
          OACSP.writeLVar("AB_ECAM_TOCFG", 0); 
        }
        if (prevButtons & ECAM_ALL_BTN) {
          OACSP.writeLVar("AB_ECAM_page12", 0); 
        }
        break;
      case ECAM_TOCFG_BTN:
        if ((prevButtons & ECAM_TOCFG_BTN) == 0) { 
          OACSP.writeLVar("AB_ECAM_TOCFG", 1); 
          OACSP.writeLVar("AB_ECAM_TOconf", 1); 
        }
        break;
      case ECAM_ALL_BTN:
        OACSP.writeLVar("AB_ECAM_page12", 1); 
        OACSP.writeLVar("ECAM_MODE", 12); 
        break;
    }
    
    // Finally check the bright controls
    if (upperBright.isChanged()) {
      OACSP.writeLVar("AB_MPL_ECAMU_Power", upperBright.map(20, 0));
    }
    if (lowerBright.isChanged()) {
      OACSP.writeLVar("AB_MPL_ECAML_Power", lowerBright.map(20, 0));
    }
  }
  
  inline void processOutputs() {
    if (OAC::LVarUpdateEvent* ev = OACSP.lvarUpdateEvent("AB_ECAM_CLR_Light")) {
      lights = ev->value ? (lights | ECAM_CLR_LGT) : (lights & ~ECAM_CLR_LGT);
    } else if (OAC::LVarUpdateEvent* ev = OACSP.lvarUpdateEvent("ECAM_MODE")) {
      lights = ev->value == 1 ? (lights | ECAM_ENG_LGT) : (lights & ~ECAM_ENG_LGT);
      lights = ev->value == 2 ? (lights | ECAM_BLEED_LGT) : (lights & ~ECAM_BLEED_LGT);
      lights = ev->value == 3 ? (lights | ECAM_PRESS_LGT) : (lights & ~ECAM_PRESS_LGT);
      lights = ev->value == 4 ? (lights | ECAM_ELEC_LGT) : (lights & ~ECAM_ELEC_LGT);
      lights = ev->value == 5 ? (lights | ECAM_HYD_LGT) : (lights & ~ECAM_HYD_LGT);
      lights = ev->value == 6 ? (lights | ECAM_FUEL_LGT) : (lights & ~ECAM_FUEL_LGT);
      lights = ev->value == 7 ? (lights | ECAM_APU_LGT) : (lights & ~ECAM_APU_LGT);
      lights = ev->value == 8 ? (lights | ECAM_COND_LGT) : (lights & ~ECAM_COND_LGT);
      lights = ev->value == 9 ? (lights | ECAM_DOOR_LGT) : (lights & ~ECAM_DOOR_LGT);
      lights = ev->value == 10 ? (lights | ECAM_WHEEL_LGT) : (lights & ~ECAM_WHEEL_LGT);
      lights = ev->value == 11 ? (lights | ECAM_FCTL_LGT) : (lights & ~ECAM_FCTL_LGT);
    }
    card0.writeOutput(lights);
    card1.writeOutput(lights >> 8);
  }
} ecam;

