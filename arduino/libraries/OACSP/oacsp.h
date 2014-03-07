#ifndef OAC_OACSP_H
#define OAC_OACSP_H

#include "Arduino.h"

#define OACSP_PROTOCOL_VERSION 0x01
#define OACSP_BUFFER_LEN 256

namespace oac {

enum EventType {
  LVAR_UPDATE
};

struct LVarUpdateEvent {
  EventType type;
  char name[64];
  long value;
};

union Event {
  EventType type;
  LVarUpdateEvent lvar;
};

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

  void observeLVar(const char* lvar) {
    Serial.print("OBS_LVAR ");
    Serial.print(lvar);
    Serial.print('\n');
  }

  bool readEvent(Event& e) {
    char buf[OACSP_BUFFER_LEN];
    int nread = Serial.readBytesUntil('\n', buf, OACSP_BUFFER_LEN - 1);
    if (nread) {
      buf[nread] = '\0';
      String line((const char*) buf);
      if (line.startsWith("EVENT_LVAR")) {
        return readLVarEvent(line, e);
      }
    }
    return false;
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

  bool readLVarEvent(String& line, Event& e) {
    line.replace("EVENT_LVAR", "");
    line.trim();
    int sep = line.indexOf(' ');
    if (sep == -1)
      return false;
    String lvar = line.substring(0, sep);
    String value = line.substring(sep + 1, line.length());
    e.type = LVAR_UPDATE;
    lvar.toCharArray(e.lvar.name, 64);
    e.lvar.value = value.toInt();
    return true;
  }
};

}

oac::SerialProtocol OACSP;

#endif
