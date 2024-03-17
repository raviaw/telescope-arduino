/////////////////////////////////////////////////////////////////////////////
//
// Log utilities
//
//

void reportStatus() {
  int horizontalSpeed = horizontalMotor.speed();
  Serial.print("H: ");
  Serial.print(horizontalMotorSpeed);
  Serial.print("(");
  Serial.print(horizontalSpeed);
  Serial.print("), pos: ");
  Serial.print(horizontalMotor.currentPosition());
  Serial.print(", ");

  int verticalSpeed = verticalMotor.speed();
  Serial.print("V: ");
  Serial.print(verticalMotorSpeed);
  Serial.print("(");
  Serial.print(verticalSpeed);
  Serial.print("), pos: ");
  Serial.print(verticalMotor.currentPosition());
  Serial.print(", ");

  Serial.print("Pot H: ");
  Serial.print(potHorizontal);
  Serial.print(", Pot V: ");
  Serial.print(potVertical);
  Serial.print(", ");

  Serial.print("S[");
  Serial.print(startYear);
  Serial.print("/");
  printPadding(startMonth, 2);
  Serial.print("/");
  printPadding(startDay, 2);
  Serial.print(" ");
  printPadding(startHour, 2);
  Serial.print(":");
  printPadding(startMinute, 2);
  Serial.print(":");
  printPadding(startSecond, 2);
  Serial.print(".");
  printPadding(startMs, 3);
  Serial.print("], ");

  Serial.print("C[");
  Serial.print(currentYear);
  Serial.print("/");
  printPadding(currentMonth, 2);
  Serial.print("/");
  printPadding(currentDay, 2);
  Serial.print(" ");
  printPadding(currentHour, 2);
  Serial.print(":");
  printPadding(currentMinute, 2);
  Serial.print(":");
  printPadding(currentSecond, 2);
  Serial.print(".");
  printPadding(currentMs, 3);
  Serial.print("], ");

  Serial.print("target: ");
  Serial.print("ra [");
  Serial.print(ra);
  Serial.print("], ");
  Serial.print("dec [");
  Serial.print(dec);
  Serial.print("], ");
  Serial.print("julianDate [");
  Serial.print(julianDate);
  Serial.print("], ");
  Serial.print("startSecOfDay [");
  Serial.print(startSecOfDay);
  Serial.print("], ");
  Serial.print("currentSecOfDay [");
  Serial.print(currentSecOfDay);
  Serial.print("], ");
  Serial.print("timeOfDay [");
  Serial.print(timeOfDay);
  Serial.print("], ");
  Serial.print("gstTime [");
  Serial.print(gstTime);
  Serial.print("], ");
  Serial.print("lst [");
  Serial.print(lst);
  Serial.print("], ");
  Serial.print("ha [");
  Serial.print(ha);
  Serial.print("], ");
  Serial.print("azm [");
  Serial.print(azm);
  Serial.print("], ");

  Serial.println();
}

void printPadding(int value, int padding) {
  int len;
  if (value == 0) {
    len = 1;
  } else {
    len = floor(log10(value)) + 1;
  }
  for (int i = 0; i < padding - len; i++) {
    Serial.print("0");
  }
  Serial.print(value);
}

void reportObject(target obj) {
  Serial.print(obj.name);
  Serial.print(": ra [");
  Serial.print(obj.ra);
  Serial.print("] dec [");
  Serial.print(obj.dec);
  Serial.print("]");
}