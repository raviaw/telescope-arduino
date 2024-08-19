void reportBluetooth() {
  JsonDocument doc;
  doc["activeMode"] = activeMode;
  doc["ra"] = ra;
  doc["dec"] = dec;
  doc["alt"] = alt;
  doc["azm"] = azm;
  doc["calibrated"] = calibrated;
  doc["slave"] = slaveMode;
  serializeJson(doc, Serial1);
  // Serial1.print("RA: ");
  // Serial1.print(ra, 3);
  // Serial1.print(", DEC: ");
  // Serial1.print(dec, 3);
  // Serial1.print(", ALT: ");
  // Serial1.print(alt, 3);
  // Serial1.print(", AZM: ");
  // Serial1.print(azm, 3);
  // Serial1.print(", CAL: ");
  // Serial1.print(calibrated);
  // Serial1.println();
  // Serial1.print("M AZM POS:  ");
  // Serial1.print(horizontalMotor->getCurrentPosition());
  // Serial1.print(", M ALT POS: ");
  // Serial1.print(verticalMotor->getCurrentPosition());
  // Serial1.println();
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
  } else if (strcmp("slave", command) == 0) {
    setSlaveMode();
  } else if (strcmp("c-start", command) == 0) {
    processCalibrateStartCommand();
  } else {
    Serial.println("Input from serial is unknown");
  }
}

void processTimeCommand() {
  const char* time = bluetoothDoc["time"];
  parseReceivedTimeString(time);
}

void processCalibrateStartCommand() {
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
  activeMode = MODE_MENU;
}

void processInput(char* serialBuffer) {
  String serialString = String(serialBuffer);
  if (serialString[0] == '?') {
    Serial1.print("Send RA h,m,s DEC h,m,s, for example 10,20,30.5 -20,10,50.1");
  } else {
    Serial1.print("Processing coordinates...");
    float raHour = atof(strtok(serialBuffer, " ,"));
    float raMinute = atof(strtok(NULL, " ,"));
    float raSecond = atof(strtok(NULL, " ,"));
    float decHour = atof(strtok(NULL, " ,"));
    float decMinute = atof(strtok(NULL, " ,"));
    float decSecond = atof(strtok(NULL, " ,"));
    
    ra = hourMinArcSecToDouble(raHour, raMinute, raSecond);
    dec = hourMinArcSecToDouble(decHour, decMinute, decSecond);
  }
}

void clearSlaveMode() {
  slaveMode = 0;
  digitalWrite(SLAVE_LED, LOW);
  digitalWrite(MASTER_LED, HIGH);
}

void setSlaveMode() {
  slaveMode = 1;
  digitalWrite(SLAVE_LED, HIGH);
  digitalWrite(MASTER_LED, LOW);
}