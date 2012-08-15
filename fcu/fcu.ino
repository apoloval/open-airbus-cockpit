#include "icm7218.h"
#include "oacsp.h"

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

enum OutputPins
{
   PIN_ICM7218_1_ID0       = 22,
   PIN_ICM7218_1_ID1       = 23,
   PIN_ICM7218_1_ID2       = 24,
   PIN_ICM7218_1_ID3       = 25,
   PIN_ICM7218_1_ID4       = 26,
   PIN_ICM7218_1_ID5       = 27,
   PIN_ICM7218_1_ID6       = 28,
   PIN_ICM7218_1_ID7       = 29,
   PIN_ICM7218_1_WRITE     = 30,
   PIN_ICM7218_1_MODE      = 31,
   PIN_ICM7218_2_ID0       = 32,
   PIN_ICM7218_2_ID1       = 33,
   PIN_ICM7218_2_ID2       = 34,
   PIN_ICM7218_2_ID3       = 35,
   PIN_ICM7218_2_ID4       = 36,
   PIN_ICM7218_2_ID5       = 37,
   PIN_ICM7218_2_ID6       = 38,
   PIN_ICM7218_2_ID7       = 39,
   PIN_ICM7218_2_WRITE     = 40,
   PIN_ICM7218_2_MODE      = 41,
   PIN_SPD_MACH_MODE       = 42,
   PIN_HDG_TRK_VS_FPA_MODE = 43,
   PIN_SPD_MANAGED_MODE    = 44,
   PIN_HDG_MANAGED_MODE    = 45,
   PIN_ALT_MANAGED_MODE    = 46,
};

enum ParameterMode
{
   PARAM_SELECTED,
   PARAM_MANAGED,
};

enum SpeedMachMode
{
   MODE_SPEED,
   MODE_MACH,
};

word status;
unsigned long selectedSpeed;
unsigned long selectedHeading;
unsigned long selectedAltitude;
unsigned long selectedVerticalSpeed;

word buttonState;

ICM7218 displayController[2];
ICM7218::DisplayGroup speedDisplayGroup;
ICM7218::DisplayGroup headingDisplayGroup;
ICM7218::DisplayGroup altitudeDisplayGroup;
ICM7218::DisplayGroup verticalSpeedDisplayGroup;

Command cmd;

void setSpeed(word value, 
              ParameterMode paramMode = PARAM_SELECTED,
              SpeedMachMode displayMode = MODE_SPEED)
{
   selectedSpeed = value;
   if (displayMode == MODE_SPEED)
      digitalWrite(PIN_SPD_MACH_MODE, 0);
   else // displayMode == MODE_MACH
   {      
      digitalWrite(PIN_SPD_MACH_MODE, 1);
      value = (word)((float) value / 573.0f);
   }
   if (paramMode == PARAM_SELECTED)
   {
      speedDisplayGroup.loadNumber(selectedSpeed);
      digitalWrite(PIN_SPD_MANAGED_MODE, 0);
   }
   else // paramMode == PARAM_MANAGED
   {
      speedDisplayGroup.loadDash();
      digitalWrite(PIN_SPD_MANAGED_MODE, 1);
   }
}

void setHeading(word value, 
                ParameterMode paramMode = PARAM_SELECTED)
{
   selectedHeading = value;
   if (paramMode == PARAM_SELECTED)
   {
      headingDisplayGroup.loadNumber(selectedHeading);
      digitalWrite(PIN_HDG_MANAGED_MODE, 0);
   }
   else // paramMode == PARAM_MANAGED
   {
      headingDisplayGroup.loadDash();
      digitalWrite(PIN_HDG_MANAGED_MODE, 1);
   }
}

void setAltitude(word value)
{
   selectedAltitude = value;
   altitudeDisplayGroup.loadNumber(selectedAltitude);
   
}

void setVerticalSpeed(word value,
                      ParameterMode paramMode = PARAM_SELECTED)
{
   if (paramMode == PARAM_SELECTED)
   {
      verticalSpeedDisplayGroup.loadNumber(selectedHeading);
   }
   else // paramMode == PARAM_MANAGED
   {
      verticalSpeedDisplayGroup.loadDash();
   }
}

void setStatus(word value)
{
   status = value;
      
   setSpeed(selectedSpeed, 
         (status & MASK_FCU_SPD_MOD) ? PARAM_SELECTED : PARAM_MANAGED,
         (status & MASK_FCU_SPD_DISP) ? MODE_MACH : MODE_SPEED);
   setHeading(selectedHeading, 
         (status & MASK_FCU_HDG_MOD) ? PARAM_SELECTED : PARAM_MANAGED);
   setVerticalSpeed(selectedVerticalSpeed,
         (status & MASK_FCU_VS_MOD) ? PARAM_SELECTED : PARAM_MANAGED);
   
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
   setAltitude(18000);
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
   int icm7218Pins[2][8] = {
      {
         PIN_ICM7218_1_ID0, PIN_ICM7218_1_ID1, PIN_ICM7218_1_ID2, 
         PIN_ICM7218_1_ID3, PIN_ICM7218_1_ID4, PIN_ICM7218_1_ID5, 
         PIN_ICM7218_1_ID6, PIN_ICM7218_1_ID7, 
      },
      {
         PIN_ICM7218_2_ID0, PIN_ICM7218_2_ID1, PIN_ICM7218_2_ID2, 
         PIN_ICM7218_2_ID3, PIN_ICM7218_2_ID4, PIN_ICM7218_2_ID5, 
         PIN_ICM7218_2_ID6, PIN_ICM7218_2_ID7, 
      },
   };
   displayController[0].setIdPins(icm7218Pins[0]);
   displayController[0].setWritePin(PIN_ICM7218_1_WRITE);
   displayController[0].setModePin(PIN_ICM7218_1_MODE);
  
   displayController[1].setIdPins(icm7218Pins[1]);
   displayController[1].setWritePin(PIN_ICM7218_2_WRITE);
   displayController[1].setModePin(PIN_ICM7218_2_MODE);
   
   int leftDisplays[] = { 0, 1, 2, -1, -1, -1, -1, -1 };
   int rightDisplays[] = { -1, -1, -1, 0, 1, 2, 3, 4 };

   speedDisplayGroup.setParent(&displayController[0]);
   speedDisplayGroup.setDisplays(leftDisplays);

   headingDisplayGroup.setParent(&displayController[1]);
   headingDisplayGroup.setDisplays(leftDisplays);
   
   altitudeDisplayGroup.setParent(&displayController[0]);
   altitudeDisplayGroup.setDisplays(rightDisplays);
   
   verticalSpeedDisplayGroup.setParent(&displayController[1]);
   verticalSpeedDisplayGroup.setDisplays(rightDisplays);
   
   pinMode(PIN_SPD_MACH_MODE, OUTPUT);
   pinMode(PIN_HDG_TRK_VS_FPA_MODE, OUTPUT);
   pinMode(PIN_SPD_MANAGED_MODE, OUTPUT);
   pinMode(PIN_HDG_MANAGED_MODE, OUTPUT);
   pinMode(PIN_ALT_MANAGED_MODE, OUTPUT);

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
   displayController[0].display();
   displayController[1].display();
}

