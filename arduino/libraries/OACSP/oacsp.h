#ifndef OAC_OACSP_H
#define OAC_OACSP_H

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

};

}

oac::SerialProtocol OACSP;

#endif
