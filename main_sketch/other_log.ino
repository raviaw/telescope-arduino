/////////////////////////////////////////////////////////////////////////////
//
// Log utilities
//
//

void reportStatus() {
  Serial.print("T: [");
  Serial.print(loopsPerSec);
  Serial.print("], ");

  int currentButtonValue = analogRead(7);
  Serial.print("B: [");
  Serial.print(currentButtonValue);
  Serial.print("], ");
  
  Serial.print("H: float m. speed: ");
  Serial.print(horizontalMotorSpeed);
  Serial.print(", speed: ");
  Serial.print(horizontalSpeed);
  Serial.print(", m. speed(): ");
  Serial.print(horizontalMotor->getCurrentSpeedInMilliHz());
  Serial.print(", pos: ");
  Serial.print(horizontalMotor->getCurrentPosition());
  Serial.print(", ");

  Serial.print("V: float m. speed: ");
  Serial.print(verticalMotorSpeed);
  Serial.print(", speed: ");
  Serial.print(verticalSpeed);
  Serial.print(", m. speed(): ");
  Serial.print(verticalMotor->getCurrentSpeedInMilliHz());
  Serial.print(", pos: ");
  Serial.print(verticalMotor->getCurrentPosition());
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

  Serial.print("Calibration: ");
  Serial.print("ha1 [");
  Serial.print(ha1);
  Serial.print("], ");
  Serial.print("ha2 [");
  Serial.print(ha2);
  Serial.print("], ");
  Serial.print("haMotor1 [");
  Serial.print(haMotor1);
  Serial.print("], ");
  Serial.print("haMotor2 [");
  Serial.print(haMotor2);
  Serial.print("], ");
  Serial.print("azm1 [");
  Serial.print(azm1);
  Serial.print("], ");
  Serial.print("azm2 [");
  Serial.print(azm2);
  Serial.print("], ");
  Serial.print("azmMotor1 [");
  Serial.print(azmMotor1);
  Serial.print("], ");
  Serial.print("azmMotor2 [");
  Serial.print(azmMotor2);
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