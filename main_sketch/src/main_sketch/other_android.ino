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
  doc["acc-y"] = AccY;
  doc["acc-z"] = AccZ;
  doc["acc-x"] = AccX;
  doc["hor-motor"] = horizontalMotor->getCurrentPosition();
  doc["ver-motor"] = verticalMotor->getCurrentPosition();
  doc["hor-encoder"] = horizontalEncoderPosition;
  doc["ver-encoder"] = verticalEncoderPosition;
  serializeJson(doc, Serial1);
}

// JsonDocument bluetoothDoc;
// DeserializationError lastDeserializationStatus;
void bluetoothSerialAvailable() {
  ReadLoggingStream loggingStream(Serial1, Serial);
  lastDeserializationStatus = deserializeJson(bluetoothDoc, loggingStream);
  const char* command = bluetoothDoc["command"];
  Serial.print(" ...bluetooth command: ");
  Serial.print(command);
  Serial.print(" ... ");
  if (strcmp("time", command) == 0) {
    Serial.print("time");
    processTimeCommand();
  } else if (strcmp("master", command) == 0) {
    Serial.print("master");
    clearSlaveMode();
  } else if (strcmp("lights-on", command) == 0) {
    Serial.print("lights-on");
    digitalWrite(LCD_LIGHT_CONTROL, HIGH);
  } else if (strcmp("lights-off", command) == 0) {
    Serial.print("lights-off");
    digitalWrite(LCD_LIGHT_CONTROL, LOW);
  } else if (strcmp("slave", command) == 0) {
    Serial.print("slave");
    setSlaveMode();
  } else if (strcmp("c-start", command) == 0) {
    Serial.print("c-start");
    processCalibrateStartCommand();
  } else if (strcmp("c-move", command) == 0) {
    Serial.print("c-move");
    processCalibrateMoveCommand();
  } else if (strcmp("c-save", command) == 0) {
    Serial.print("c-save");
    processCalibrateSaveCommand();
  } else if (strcmp("c-stop", command) == 0) {
    Serial.print("c-stop");
    processCalibrateStopCommand();
  } else if (strcmp("c-done", command) == 0) {
    Serial.print("c-done");
    processCalibrateDoneCommand();
  } else if (strcmp("c-save", command) == 0) {
    Serial.print("c-save");
    processCalibrateSaveCommand();
  } else if (strcmp("menu-main", command) == 0) {
    Serial.print("menu-main");
    processMenuMainCommand();
  } else if (strcmp("find-star", command) == 0) {
    Serial.print("find-star");
    processFindStarCommand();
  } else if (strcmp("go", command) == 0) {
    Serial.print("go");
    processGoCommand();
  } else if (strcmp("move-start", command) == 0) {
    Serial.print("move-start");
    processMoveStartCommand();
  } else if (strcmp("move", command) == 0) {
    Serial.print("move");
    processMoveCommand();
  } else if (strcmp("move-done", command) == 0) {
    Serial.print("move-done");
    processMoveDoneCommand();
  } else {
    Serial.print("***");
  }
  Serial.println(" ... command processed");
}

void processMenuMainCommand() {
  activeMode = MODE_MENU;
  clearSlaveMode();
}

void processTimeCommand() {
  const char* time = bluetoothDoc["time"];
  Serial.print(", parsing time: ");
  Serial.print(time);
  parseReceivedTimeString(time);
}

void processCalibrateStartCommand() {
  resetSpeeds();
  setSlaveMode();
  calibrated = 0; 
  int index = bluetoothDoc["index"];
  int starIndex = bluetoothDoc["starIndex"];
  Serial.print(", index: ");
  Serial.print(index);
  Serial.print(", star index: ");
  Serial.print(starIndex);
  startCalibration();

  calibratingStarIndex = index;

  target* wantedStar = &targets[starIndex];
  calibratingTarget = wantedStar;
  ra = wantedStar->ra;
  dec = wantedStar->dec;
  special = wantedStar->special;
  prepareToMoveWithCalibration();
  calculateEverything();
  prepareStarCoordinates();
  activeMode = MODE_CALIBRATE_MOVING;
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
}

void processCalibrateDoneCommand() {
  Serial.print(", calibratingStarIndex: ");
  Serial.print(calibratingStarIndex);

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

void processGoCommand() {
  int raHours = bluetoothDoc["ra-h"];
  int raMinutes = bluetoothDoc["ra-m"];
  float raSeconds  = bluetoothDoc["ra-s"];
  int decHours = bluetoothDoc["dec-h"];
  int decMinutes = bluetoothDoc["dec-m"];
  float decSeconds  = bluetoothDoc["dec-s"];
  ra = Ephemeris::hoursMinutesSecondsToFloatingHours(raHours, raMinutes, raSeconds);  
  dec = Ephemeris::hoursMinutesSecondsToFloatingHours(decHours, decMinutes, decSeconds);
  Serial.print(", go to: ");
  Serial.print(ra);
  Serial.print(", ");
  Serial.print(dec);
  special = -1;  
}

void processMoveStartCommand() {
  setSlaveMode();
  if(calibrated) {
    activeMode = MODE_MOVE_COORDINATES;
    special = -1;
  } else {
    activeMode = MODE_MOVE_MOTOR;
  } 
}

void processMoveCommand() {
  setSlaveMode();
  androidHorizontalSpeed = bluetoothDoc["x"];
  androidVerticalSpeed = bluetoothDoc["y"];
  int speed = bluetoothDoc["speed"];
  leftJoystickSpeed = speed;
  rightJoystickSpeed = speed;
}

void processMoveDoneCommand() {
  activeMode = MODE_MENU;
  slaveMode = 0;
}