#include "icm7218.h"
#include "Arduino.h"

ICM7218 ci;
ICM7218::DisplayGroup grp1;
ICM7218::DisplayGroup grp2;

void setup() {
  int idPins[] = { 2, 3, 4, 5, 6, 7, 8,  9 };
  ci.setWritePin(11);
  ci.setModePin(10);
  ci.setIdPins(idPins);
  
  int displays1[] = { 0, 1, 2, -1, -1, -1, -1, -1 };
  grp1.setParent(&ci);
  grp1.setDisplays(displays1);

  int displays2[] = { -1, -1, -1, 0, 1, 2, -1, -1 };
  grp2.setParent(&ci);
  grp2.setDisplays(displays2);
}

unsigned long number1 = 0;
unsigned long number2 = 999;

void loop() {
  grp1.displayNumber(number1);
  grp2.displayNumber(number2);
  delay(100);
  number1++;
  number2--;
}

