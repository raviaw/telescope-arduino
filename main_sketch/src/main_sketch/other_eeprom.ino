void resetEEPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

int eepromPointer = 0;
void writeEEPROMState() {
  resetEEPROM();
  eepromPointer = 0;
}

byte eepromBuffer[8];

void writeEEPROMInt(int value) {
  int size = sizeof(int);
  memcpy(value, eepromBuffer, size);
  for (;eepromPointer < eepromPointer + size; eepromPointer++) {
  }
}

void readEEPROMState() {
}

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
        EEPROM.update(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}
