void reportBluetooth() {
  Serial1.print("RA: ");
  Serial1.print(ra, 3);
  Serial1.print(", DEC: ");
  Serial1.print(dec, 3);
  Serial1.print(", ALT: ");
  Serial1.print(alt, 3);
  Serial1.print(", AZM: ");
  Serial1.print(azm, 3);
  Serial1.print(", CAL: ");
  Serial1.print(calibrated);
  Serial1.println();
  Serial1.print("M AZM POS:  ");
  Serial1.print(horizontalMotor->getCurrentPosition());
  Serial1.print(", M ALT POS: ");
  Serial1.print(verticalMotor->getCurrentPosition());
  Serial1.println();
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
