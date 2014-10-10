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
  
  void processInputs() {
    card0.readInput();
    card1.readInput();

    // First check what buttons have been activated
    word activated = card0.inputActivated() | 
      (word(card1.inputActivated()) << 8);
    sendButtonState(activated, 1);
    
    // Now check what buttons are deactivated
    word deactivated = card0.inputDeactivated() | 
      (word(card1.inputDeactivated()) << 8);
    sendButtonState(deactivated, 0);
    
    // Finally check the bright controls
    if (upperBright.isChanged()) {
      OACSP.writeLVar("AB_MPL_ECAMU_Power", upperBright.map(20, 0));
    }
    if (lowerBright.isChanged()) {
      OACSP.writeLVar("AB_MPL_ECAML_Power", lowerBright.map(20, 0));
    }
  }
  
  void processOutputs() {
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
      lights = ev->value == 13 ? (lights | ECAM_STS_LGT) : (lights & ~ECAM_STS_LGT);
    }
    card0.writeOutput(lights);
    card1.writeOutput(lights >> 8);
  }

private:

  void sendButtonState(word button, byte state) {
    switch (button) {
      case ECAM_TOCFG_BTN:
        OACSP.writeLVar("AB_ECAM_TOCFG", state);
        if (state) {
          OACSP.writeLVar("AB_ECAM_TOconf", state); 
        }
        break;
      case ECAM_ENG_BTN: 
        OACSP.writeLVar("AB_ECAM_page01", state); break;
      case ECAM_BLEED_BTN:
        OACSP.writeLVar("AB_ECAM_page02", state); break;
      case ECAM_PRESS_BTN:
        OACSP.writeLVar("AB_ECAM_page03", state); break;
      case ECAM_ELEC_BTN:
        OACSP.writeLVar("AB_ECAM_page04", state); break;
      case ECAM_HYD_BTN:
        OACSP.writeLVar("AB_ECAM_page05", state); break;
      case ECAM_FUEL_BTN:
        OACSP.writeLVar("AB_ECAM_page06", state); break;
      case ECAM_APU_BTN:
        OACSP.writeLVar("AB_ECAM_page07", state); break;
      case ECAM_COND_BTN:
        OACSP.writeLVar("AB_ECAM_page08", state); break;
      case ECAM_DOOR_BTN:
        OACSP.writeLVar("AB_ECAM_page09", state); break;
      case ECAM_WHEEL_BTN:
        OACSP.writeLVar("AB_ECAM_page10", state); break;
      case ECAM_FCTL_BTN:
        OACSP.writeLVar("AB_ECAM_page11", state); break;
      case ECAM_ALL_BTN:
        OACSP.writeLVar("AB_ECAM_page12", state); break;
      case ECAM_CLR_BTN:
        OACSP.writeLVar("AB_ECAM_CLR", state); break;
      case ECAM_STS_BTN:
        OACSP.writeLVar("AB_ECAM_page13", state); break;
      case ECAM_RCL_BTN:
        OACSP.writeLVar("AB_ECAM_RCL", state); break;
    }

  }

} ecam;

