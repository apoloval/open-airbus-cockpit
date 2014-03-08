#ifndef OAC_OACSP_H
#define OAC_OACSP_H

#include "Arduino.h"

#define OACSP_PROTOCOL_VERSION 0x01
#define OACSP_BUFFER_LEN 256
#define OACSP_MAX_NAME_LEN 64

namespace oac {

enum OffsetLength {
  OFFSET_UINT8,
  OFFSET_SINT8,
  OFFSET_UINT16,
  OFFSET_SINT16,
  OFFSET_UINT32,
  OFFSET_SINT32,
};

const char* OffsetLengthCode[]  = { "UB", "SB", "UW", "SW", "UD", "SD" };

enum EventType {
  LVAR_UPDATE,
  OFFSET_UPDATE,
};

struct LVarUpdateEvent {
  EventType type;
  char name[OACSP_MAX_NAME_LEN];
  long value;
};

struct OffsetUpdateEvent {
  EventType type;
  word address;
  long value;
};

union Event {
  EventType type;
  LVarUpdateEvent lvar;
  OffsetUpdateEvent offset;
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

  void writeOffset(word offset, unsigned char value) {
    writeOffset(offset, OFFSET_UINT8, value);
  }

  void writeOffset(word offset, char value) {
    writeOffset(offset, OFFSET_SINT8, value);
  }

  void writeOffset(word offset, unsigned int value) {
    writeOffset(offset, OFFSET_UINT16, value);
  }

  void writeOffset(word offset, int value) {
    writeOffset(offset, OFFSET_SINT16, value);
  }

  void writeOffset(word offset, unsigned long value) {
    writeOffset(offset, OFFSET_UINT32, value);
  }

  void writeOffset(word offset, long value) {
    writeOffset(offset, OFFSET_SINT32, value);
  }

  template <typename T>
  void writeOffset(word offset, OffsetLength len, T value) {
    Serial.print("WRITE_OFFSET ");
    Serial.print(offset, HEX);
    Serial.print(":");
    Serial.print(OffsetLengthCode[len]);
    Serial.print(" ");
    Serial.print(value, DEC);
    Serial.print('\n');
  }

  void observeLVar(const char* lvar) {
    Serial.print("OBS_LVAR ");
    Serial.print(lvar);
    Serial.print('\n');
  }

  void observeOffset(word offset, OffsetLength len) {
    Serial.print("OBS_OFFSET ");
    Serial.print(offset, HEX);
    Serial.print(":");
    Serial.print(OffsetLengthCode[len]);
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
      } else if (line.startsWith("EVENT_OFFSET")) {
        return readOffsetEvent(line, e);
      }
    }
    return false;
  }

private:

  bool readLVarEvent(String& line, Event& e) {
    line.replace("EVENT_LVAR", "");
    line.trim();
    String lvar, value;
    if (!parseTuple2(line, lvar, value))
      return false;
    e.type = LVAR_UPDATE;
    lvar.toCharArray(e.lvar.name, 64);
    e.lvar.value = value.toInt();
    return true;
  }

  bool readOffsetEvent(String& line, Event& e) {
    line.replace("EVENT_OFFSET", "");
    line.trim();
    String offset, value;
    if (!parseTuple2(line, offset, value))
      return false;
    e.type = OFFSET_UPDATE;
    e.offset.address = offset.toInt();
    e.offset.value = value.toInt();
    return true;
  }

  bool parseTuple2(const String& line, String& tk1, String& tk2) {
    int sep = line.indexOf(' ');
    if (sep == -1)
      return false;
    tk1 = line.substring(0, sep);
    tk2 = line.substring(sep + 1, line.length());
    return true;
  }
};

}

oac::SerialProtocol OACSP;

#endif
