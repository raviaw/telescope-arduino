/////////////////////////////////////////////////////////////////////////////
//
// Log utilities
//
//

void reportStatus() {
  printTime = 0;
  int verticalSpeed = verticalMotor.speed();
  Serial.print("V: ");
  Serial.print(verticalSpeed);
  Serial.print("  ");
  Serial.print(verticalMotor.currentPosition());
  Serial.print(", ");
  
  int horizontalSpeed = horizontalMotor.speed();
  Serial.print("H: ");
  Serial.print(horizontalSpeed);
  Serial.print("  ");
  Serial.print(horizontalMotor.currentPosition());
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

  Serial.print("LST: ");
  Serial.print(LST_degrees);
  Serial.print("|");
  Serial.print(LST_hours);
  Serial.print("|");
  Serial.print(LST_minutes);
  Serial.print("|");
  Serial.print(LST_seconds);
  Serial.print(", ");

  reportObject(Vega);

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

void reportObject(target& obj) {
  Serial.print(obj.name);
  Serial.print(": dec deg [");
  Serial.print(obj.DEC_degrees);
  Serial.print("] arc [");
  Serial.print(obj.DEC_arcmin);
  Serial.print("] arcsec [");
  Serial.print(obj.DEC_arcsec);
  Serial.print("] ra hour [");
  Serial.print(obj.RA_hour);
  Serial.print("] min [");
  Serial.print(obj.RA_min);
  Serial.print("] sec [");
  Serial.print(obj.RA_sec);
  Serial.print("] [dec: ");
  Serial.print(obj.DEC_decimal);
  Serial.print("] [ra: ");
  Serial.print(obj.RA_decimal);
  Serial.print("] [ha: ");
  Serial.print(obj.HA_decimal);
  Serial.print("] [alt: ");
  Serial.print(obj.ALT_decimal);
  Serial.print("] [azm: ");
  Serial.print(obj.AZM_decimal);
  Serial.print("]");
}