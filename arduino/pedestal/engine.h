/*
 * Open Airbus Cockpit - Arduino Pedestal Sketch
 * Copyright (c) 2012-2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define ENG_MASTER1_SW_PIN  22
#define ENG_MASTER2_SW_PIN  23
#define ENG_MODE_SEL_PIN    24

struct Engine {
  OAC::Button<LOW> masterOneSw;
  OAC::Button<LOW> masterTwoSw;
  OAC::RotarySwitch<3, LOW> modeSel;
  
  Engine() :
    masterOneSw(ENG_MASTER1_SW_PIN),
    masterTwoSw(ENG_MASTER2_SW_PIN),
    modeSel(ENG_MODE_SEL_PIN) {}
    
  static void onMasterOneToggle(int isOn) {
    OACSP.writeLVar("AB_PDS_Eng1Master", isOn);
  }
  
  static void onMasterTwoToggle(int isOn) {
    OACSP.writeLVar("AB_PDS_Eng2Master", isOn);
  }
  
  static void onModeSelected(int pos) {
    OACSP.writeLVar("AB_PDS_ignition", pos);
  }
  
  void setup() {
    masterOneSw.setOnToggled(onMasterOneToggle);
    masterTwoSw.setOnToggled(onMasterTwoToggle);
    modeSel.setOnSelect(onModeSelected);
  }
  
  void loop() {
    masterOneSw.check();
    masterTwoSw.check();
    modeSel.check();
  }
} engine;

