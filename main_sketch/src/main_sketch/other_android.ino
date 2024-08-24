void reportBluetooth() {
  JsonDocument doc;
  doc["activeMode"] = activeMode;
  doc["ra"] = ra;
  doc["dec"] = dec;
  doc["last-star-ra"] = lastStarRa;
  doc["last-star-dec"] = lastStarDec;
  doc["c-m-alt"] = currentMotorAlt;
  doc["c-m-azm"] = currentMotorAzm;
  doc["alt"] = alt;
  doc["azm"] = azm;
  doc["calibrated"] = calibrated;
  doc["slave"] = slaveMode;
  doc["hor-motor"] = horizontalMotor->getCurrentPosition();
  doc["ver-motor"] = verticalMotor->getCurrentPosition();
  serializeJson(doc, Serial1);
}

// JsonDocument bluetoothDoc;
// DeserializationError lastDeserializationStatus;
void bluetoothSerialAvailable() {
  ReadLoggingStream loggingStream(Serial1, Serial);
  lastDeserializationStatus = deserializeJson(bluetoothDoc, loggingStream);
  const char* command = bluetoothDoc["command"];
  if (strcmp("time", command) == 0) {
    processTimeCommand();
  } else if (strcmp("master", command) == 0) {
    clearSlaveMode();
  } else if (strcmp("lights-on", command) == 0) {
    digitalWrite(LCD_LIGHT_CONTROL, HIGH);
  } else if (strcmp("lights-off", command) == 0) {
    digitalWrite(LCD_LIGHT_CONTROL, LOW);
  } else if (strcmp("slave", command) == 0) {
    setSlaveMode();
  } else if (strcmp("c-start", command) == 0) {
    processCalibrateStartCommand();
  } else if (strcmp("c-move", command) == 0) {
    processCalibrateMoveCommand();
  } else if (strcmp("c-save", command) == 0) {
    processCalibrateSaveCommand();
  } else if (strcmp("c-stop", command) == 0) {
    processCalibrateStopCommand();
  } else if (strcmp("c-done", command) == 0) {
    processCalibrateDoneCommand();
  } else if (strcmp("c-save", command) == 0) {
    processCalibrateSaveCommand();
  } else if (strcmp("menu-main", command) == 0) {
    processMenuMainCommand();
  } else if (strcmp("find-star", command) == 0) {
    processFindStarCommand();
  } else {
    Serial.println("Input from serial is unknown");
  }
}

void processMenuMainCommand() {
  resetSpeeds();
  activeMode = MODE_MENU;
  clearSlaveMode();
}

void processTimeCommand() {
  const char* time = bluetoothDoc["time"];
  parseReceivedTimeString(time);
}

void processCalibrateStartCommand() {
  resetSpeeds();
  setSlaveMode();
  calibrated = 0; 
  int index = bluetoothDoc["index"];
  int starIndex = bluetoothDoc["starIndex"];
  startCalibration();

  calibratingStarIndex = index;

  target* wantedStar = &targets[starIndex];
  ra = wantedStar->ra;
  dec = wantedStar->dec;
  special = wantedStar->special;
  prepareToMoveWithCalibration();
  calculateEverything();
  prepareStarCoordinates();
  activeMode = MODE_CALIBRATE_MOVING;
  
  Serial.println("Want to start calibration now");
}

void processCalibrateMoveCommand() {
  androidHorizontalSpeed = bluetoothDoc["x"];
  androidVerticalSpeed = bluetoothDoc["y"];
  int speed = bluetoothDoc["speed"];
  leftJoystickSpeed = speed;
  rightJoystickSpeed = speed;
}

void processCalibrateSaveCommand() {
  storeCalibrateCoordinates();
  activeMode = MODE_CALIBRATE_STAR_COMPLETE;
  resetSpeeds();
}

void processCalibrateStopCommand() {
  resetSpeeds();
  clearSlaveMode();
}

void processCalibrateDoneCommand() {
  resetSpeeds();
  storeCalibrateCoordinates();
  storeCalibrationData();
  prepareToMoveWithCalibration();
  activeMode = MODE_CALIBRATION_COMPLETE;
}

void clearSlaveMode() {
  resetSpeeds();
  slaveMode = 0;
  digitalWrite(SLAVE_LED, LOW);
  digitalWrite(MASTER_LED, HIGH);
}

void setSlaveMode() {
  resetSpeeds();
  slaveMode = 1;
  digitalWrite(SLAVE_LED, HIGH);
  digitalWrite(MASTER_LED, LOW);
}

void processFindStarCommand() {
  resetSpeeds();
  setSlaveMode();
  int index = bluetoothDoc["index"];
  int starIndex = bluetoothDoc["starIndex"];

  target* wantedStar = &targets[starIndex];

  ra = wantedStar->ra;
  dec = wantedStar->dec;
  special = wantedStar->special;
  prepareToMoveWithCalibration();
  calculateEverything();
  activeMode = MODE_MENU;
}
