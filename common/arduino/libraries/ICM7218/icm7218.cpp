#include "icm7218.h"
#include "Arduino.h"

#define BANKSELECT_PIN(ci) (ci->_idPin[3])
#define SHUTDOWN_PIN(ci)   (ci->_idPin[4])
#define DECODE_PIN(ci)     (ci->_idPin[5])
#define HEXCODEB_PIN(ci)   (ci->_idPin[6])
#define DATACOMING_PIN(ci) (ci->_idPin[7])

ICM7218::ICM7218() {}

void
ICM7218::setIdPins(int idPin[8])
{
  for (int i = 0; i < 8; i++)
  {
    _idPin[i] = idPin[i];
    pinMode(_idPin[i], OUTPUT);
  }
}

void
ICM7218::setModePin(int pin)
{
  _modePin = pin;
  pinMode(_modePin, OUTPUT);
}

void
ICM7218::setWritePin(int pin)
{
  _writePin = pin;
  pinMode(_writePin, OUTPUT);
  
  // WRITE must remain high from now on
  digitalWrite(_writePin, HIGH);
}

void
ICM7218::displaySingleDigit(unsigned int  digit, unsigned int  digitValue)
{
  if (digit < 1 || digit > 8)
    return;
  digitalWrite(_modePin, HIGH);
  digitalWrite(DATACOMING_PIN(this), LOW);
  digitalWrite(SHUTDOWN_PIN(this), HIGH);
  digitalWrite(DECODE_PIN(this), LOW);
  digitalWrite(HEXCODEB_PIN(this), LOW);
  digitalWrite(_idPin[0], B0001 & (digit - 1));
  digitalWrite(_idPin[1], B0010 & (digit - 1));
  digitalWrite(_idPin[2], B0100 & (digit - 1));
  digitalWrite(_idPin[3], B1000 & (digit - 1));
  sendWrite();
  
  digitalWrite(_modePin, LOW);
  digitalWrite(_idPin[0], B0001 & digitValue);
  digitalWrite(_idPin[1], B0010 & digitValue);
  digitalWrite(_idPin[2], B0100 & digitValue);
  digitalWrite(_idPin[3], B1000 & digitValue);
  sendWrite();  
}

void
ICM7218::sendWrite()
{
  digitalWrite(_writePin, LOW);
  digitalWrite(_writePin, HIGH);
}

ICM7218::DisplayGroup::DisplayGroup()
 : _parent(NULL)
{
  for (int i = 0; i < 8; i++)
    _displays[i] = i + 1;
}

void
ICM7218::DisplayGroup::setParent(ICM7218* parent)
{ _parent = parent; }

void
ICM7218::DisplayGroup::setDisplays(int displays[8])
{
  for (int i = 0; i < 8; i++)
    _displays[i] = byte(displays[i]);
}

void
ICM7218::DisplayGroup::displayNumber(unsigned long number)
{
  digitalWrite(_parent->_modePin, HIGH);
  digitalWrite(DATACOMING_PIN(_parent), HIGH);
  digitalWrite(BANKSELECT_PIN(_parent), HIGH);
  digitalWrite(SHUTDOWN_PIN(_parent), HIGH);
  digitalWrite(DECODE_PIN(_parent), LOW);
  digitalWrite(HEXCODEB_PIN(_parent), LOW);
  _parent->sendWrite();
  
  digitalWrite(_parent->_modePin, LOW);
  for (int i = 0; i < 8; i++)
  {
    byte digit = 0x0f;
    if (_displays[i] <= 8)
      digit = (number / long(pow(10, _displays[i]))) % 10;
    digitalWrite(_parent->_idPin[0], B0001 & digit);
    digitalWrite(_parent->_idPin[1], B0010 & digit);
    digitalWrite(_parent->_idPin[2], B0100 & digit);
    digitalWrite(_parent->_idPin[3], B1000 & digit);
    digitalWrite(_parent->_idPin[7], HIGH);
    _parent->sendWrite();    
  }
}


