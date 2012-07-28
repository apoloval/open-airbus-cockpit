#include "icm7218.h"
#include "oacsp.h"

word status;
unsigned long selectedSpeed;
unsigned long selectedHeading;

word buttonState;

enum ButtonStateMask
{
   BTN_FD       = 0x0001,
   BTN_ILS      = 0x0002,
   BTN_LOC      = 0x0004,
   BTN_AP1      = 0x0008,
   BTN_AP2      = 0x0010,
   BTN_ATHR     = 0x0020,
   BTN_EXP      = 0x0040,
   BTN_APPR     = 0x0080,
   BTN_SEL_SPD  = 0x0100,
   BTN_SEL_HDG  = 0x0200,
   BTN_ALT      = 0x0400,
   BTN_SEL_VS   = 0x0800,
   BTN_SPD_DISP = 0x1000,
   BTN_HDG_DISP = 0x2000,
   BTN_VS_DISP  = 0x4000,
};

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
   status = value;
      
   setSpeed(selectedSpeed, status & MASK_FCU_SPD_MOD);
   setHeading(selectedHeading, status & MASK_FCU_HDG_MOD);
   
   // TODO: update the rest of displays
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

void reset()
{
   buttonState = 0;
   setStatus(0);
}

void processCommand(union Command& cmd)
{
   switch (cmd.type)
   {
      case CMD_RESET:
         reset();
         break;
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

