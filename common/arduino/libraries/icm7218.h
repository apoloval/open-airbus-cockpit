#ifndef OAC_ICM7218
#define OAC_ICM7218

#include "Arduino.h"

class ICM7218
{
public:

  class DisplayGroup {
  public:
  
    DisplayGroup();
    
    void setParent(ICM7218* parent);
    
    void setDisplays(int displays[8]);
    
    void displayNumber(unsigned long number);
    
  private:
  
    byte _displays[8];
    ICM7218* _parent;
  
  };

  ICM7218();
  
  void setIdPins(int idPin[8]);
  
  void setWritePin(int pin);
  
  void setModePin(int pin);
  
  void displaySingleDigit(unsigned int  digit, unsigned int  digitValue);
  
private:

  byte _idPin[8];  
  byte _writePin;
  byte _modePin;
  
  byte _cache[8];
  
  void sendWrite();  

};

#endif

