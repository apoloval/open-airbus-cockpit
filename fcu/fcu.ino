#include "icm7218.h"
#include "oacsp.h"

unsigned long speedValue;
unsigned long headingValue;

ICM7218 leftDisplayController;
ICM7218::DisplayGroup speedDisplayGroup;
ICM7218::DisplayGroup headingDisplayGroup;

Command cmd;

void setSpeed(word value)
{
  speedValue = value;
  speedDisplayGroup.displayNumber(speedValue);
}

void setHeading(word value)
{
  headingValue = value;
  headingDisplayGroup.displayNumber(headingValue);
}

void writeVariable(struct WriteVarCommand& cmd)
{
  switch (cmd.offset)
  {
    case VAR_FCU_SEL_SPD:
      setSpeed(cmd.data);
      break;
    case VAR_FCU_SEL_HDG:
      setHeading(cmd.data);
      break;
    default:
      Serial.print("Invalid offset ");
      Serial.println(cmd.offset, HEX);
  }
}

void processCommand(union Command& cmd)
{
  switch (cmd.type)
  {
    case CMD_WRITE_VAR:
      writeVariable(cmd.writeVar);
      break;
    default:
      Serial.print("Unknown command code ");
      Serial.println(cmd.type, HEX);
  }
}

void setup()
{
  int idPins[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
  leftDisplayController.setWritePin(11);
  leftDisplayController.setModePin(10);
  leftDisplayController.setIdPins(idPins);
  
  int speedDisplays[] = { 0, 1, 2, -1, -1, -1, -1, -1 };
  speedDisplayGroup.setParent(&leftDisplayController);
  speedDisplayGroup.setDisplays(speedDisplays);

  int headingDisplays[] = { -1, -1, -1, 0, 1, 2, -1, -1 };
  headingDisplayGroup.setParent(&leftDisplayController);
  headingDisplayGroup.setDisplays(headingDisplays);
  
  setSpeed(0);
  setHeading(0);
  
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() >= sizeof(Command))
  {
    Serial.readBytes((char*) &cmd, sizeof(Command));
    processCommand(cmd);
  }
}


