void resetEEPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

int eepromPointer = 0;
byte eepromBuffer[8];

void storeEEPROMData() {
  return;

  // eepromPointer = 0;
  // eepromPointer += EEPROM_writeAnything(eepromPointer, calibrated);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, ledIncrement);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, calibrationPoint0);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, calibrationPoint1);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, equatorialCoordinates);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, horizontalCoordinates);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, currentMotorAlt);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, currentMotorAzm);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, lastStarRa);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, lastStarDec);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, horizontalSpeed);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, verticalSpeed);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, verticalMotorSpeed);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, horizontalMotorSpeed);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, horizontalCoordinateSpeed);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, verticalCoordinateSpeed);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, horizontalEncoderPositionDelta);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, verticalEncoderPositionDelta);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, horizontalMotorPositionDelta);
  // eepromPointer += EEPROM_writeAnything(eepromPointer, verticalMotorPositionDelta);
  reportLog("Wrote EEPROM state, size: ");
  Serial.println(eepromPointer);
  finishLog();
}

void readEEPROMState() {
  return;

  // eepromPointer = 0;
  // eepromPointer += EEPROM_readAnything(eepromPointer, calibrated);
  // eepromPointer += EEPROM_readAnything(eepromPointer, ledIncrement);
  // eepromPointer += EEPROM_readAnything(eepromPointer, calibrationPoint0);
  // eepromPointer += EEPROM_readAnything(eepromPointer, calibrationPoint1);
  // eepromPointer += EEPROM_readAnything(eepromPointer, equatorialCoordinates);
  // eepromPointer += EEPROM_readAnything(eepromPointer, horizontalCoordinates);
  // eepromPointer += EEPROM_readAnything(eepromPointer, currentMotorAlt);
  // eepromPointer += EEPROM_readAnything(eepromPointer, currentMotorAzm);
  // eepromPointer += EEPROM_readAnything(eepromPointer, lastStarRa);
  // eepromPointer += EEPROM_readAnything(eepromPointer, lastStarDec);
  // eepromPointer += EEPROM_readAnything(eepromPointer, horizontalSpeed);
  // eepromPointer += EEPROM_readAnything(eepromPointer, verticalSpeed);
  // eepromPointer += EEPROM_readAnything(eepromPointer, verticalMotorSpeed);
  // eepromPointer += EEPROM_readAnything(eepromPointer, horizontalMotorSpeed);
  // eepromPointer += EEPROM_readAnything(eepromPointer, horizontalCoordinateSpeed);
  // eepromPointer += EEPROM_readAnything(eepromPointer, verticalCoordinateSpeed);
  // eepromPointer += EEPROM_readAnything(eepromPointer, horizontalEncoderPositionDelta);
  // eepromPointer += EEPROM_readAnything(eepromPointer, verticalEncoderPositionDelta);
  // eepromPointer += EEPROM_readAnything(eepromPointer, horizontalMotorPositionDelta);
  // eepromPointer += EEPROM_readAnything(eepromPointer, verticalMotorPositionDelta);
  if (calibrated) {
    storeCalibrationData();
  }
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
