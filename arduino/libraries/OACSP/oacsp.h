#ifndef OAC_OACSP_H
#define OAC_OACSP_H

#include "Arduino.h"

#define OACSP_PROTOCOL_VERSION 0x01

namespace oac {

class SerialProtocol {
public:

  SerialProtocol() {}

  void begin(const char* clientName, int baudRate = 9600) {
    Serial.begin(baudRate);
    Serial.print("BEGIN ");
    Serial.print(OACSP_PROTOCOL_VERSION, HEX);
    Serial.print(" ");
    Serial.print(clientName);
    Serial.print('\n');
  }
  
  void end() {
    Serial.print("END\n");
  }
  
  void writeLVar(const char* lvar, int value) {
    Serial.print("WRITE_LVAR ");
    Serial.print(lvar);
    Serial.print(" ");
    Serial.print(value, DEC);
    Serial.print('\n');
  }

  void writeOffset(unsigned int offset, byte value) {
    writeOffset(offset, "UB", value);
  }

  void writeOffset(unsigned int offset, char value) {
    writeOffset(offset, "SB", value);
  }

  void writeOffset(unsigned int offset, unsigned int value) {
    writeOffset(offset, "UW", value);
  }

  void writeOffset(unsigned int offset, int value) {
    writeOffset(offset, "SW", value);
  }

  void writeOffset(unsigned int offset, unsigned long value) {
    writeOffset(offset, "UD", value);
  }

  void writeOffset(unsigned int offset, long value) {
    writeOffset(offset, "SD", value);
  }

private:

  template <typename T>
  int writeOffset(unsigned int offset, const char* len, T value) {
    Serial.print("WRITE_OFFSET ");
    Serial.print(offset, HEX);
    Serial.print(":");
    Serial.print(len);
    Serial.print(" ");
    Serial.print(value, DEC);
    Serial.print('\n');
  }
};

}

oac::SerialProtocol OACSP;

#endif
