/*
 * Open Airbus Cockpit - Arduino Pedestal Sketch
 * Copyright (c) 2012-2014 Alvaro Polo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oacbtn.h>
#include <oacexp.h>
#include <oacio.h>
#include <oacsp.h>
#include <oacshift.h>

#define DEVICE_NAME "PedestalMaster"

#include "ecam.h"
#include "engine.h"

void setup() {
  OACSP.begin(DEVICE_NAME);
  ecam.setup();
  engine.setup();
}

void loop() {
  OACSP.pollEvent();
  ecam.loop();
  engine.loop();
}
