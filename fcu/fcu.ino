#include "icm7218.h"
#include "oacsp.h"

word status;
unsigned long selectedSpeed;
unsigned long selectedHeading;

ICM7218 leftDisplayController;
ICM7218::DisplayGroup speedDisplayGroup;
ICM7218::DisplayGroup headingDisplayGroup;

Command cmd;

void setSpeed(word value, bool selected = true)
{
   selectedSpeed = value;
   if (selected)
      speedDisplayGroup.displayNumber(selectedSpeed);
   else
      speedDisplayGroup.displayDash();
}

void setHeading(word value, bool selected = true)
{
   selectedHeading = value;
   if (selected)
      headingDisplayGroup.displayNumber(selectedHeading);
   else
      headingDisplayGroup.displayDash();
}

void setStatus(word value)
{
   if (status != value)
   {
      status = value;
      
      setSpeed(selectedSpeed, status & MASK_FCU_SPD_MOD);
      
      // TODO: update the rest of displays
   }
}

void writeVariable(struct WriteVarCommand& cmd)
{
   switch (cmd.offset)
   {
      case VAR_FCU_STATUS:
         setStatus(cmd.data);
         break;
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

void reset()
{
   setSpeed(0, false);
   setHeading(0, false);
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

   reset();  
  
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

