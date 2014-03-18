/*
 * Open Airbus Cockpit - Arduino Pedestal Sketch
 * Copyright (c) 2012-2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <button.h>
#include <oacsp.h>

struct Engine {
  Button<LOW> masterOneSw;
  Button<LOW> masterTwoSw;
  RotarySwitch<3, LOW> modeSel;

  Engine() :
    masterOneSw(2),
    masterTwoSw(3),
    modeSel(4) {}

  static void onMasterOneToggle(int isOn) {
    OACSP.writeLVar("AB_PDS_Eng1Master", isOn);
  }

  static void onMasterTwoToggle(int isOn) {
    OACSP.writeLVar("AB_PDS_Eng2Master", isOn);
  }

  static void onModeSelected(int pos) {
    OACSP.writeLVar("AB_PDS_ignition", pos);
  }

} engine;

void setup() {
  OACSP.begin("PedestalMaster");
  engine.masterOneSw.setOnToggled(engine.onMasterOneToggle);
  engine.masterTwoSw.setOnToggled(engine.onMasterTwoToggle);
  engine.modeSel.setOnSelect(engine.onModeSelected);
}

void loop() {
  engine.masterOneSw.check();
  engine.masterTwoSw.check();
  engine.modeSel.check();
}
