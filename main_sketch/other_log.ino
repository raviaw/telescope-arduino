/////////////////////////////////////////////////////////////////////////////
//
// Log utilities
//
//

void reportStatus() {
  Serial.print("T: [");
  Serial.print(loopsPerSec);
  Serial.print("], ");

  Serial.print("alt: ");
  Serial.print(alt);
  Serial.print(", alt1: ");
  Serial.print(alt1);
  Serial.print(", alt2: ");
  Serial.print(alt2);
  Serial.print(", altMotor1: ");
  Serial.print(altMotor1);
  Serial.print(", altMotor2: ");
  Serial.print(altMotor2);
  Serial.print(", azm: ");
  Serial.print(azm);
  Serial.print(", azm1: ");
  Serial.print(azm1);
  Serial.print(", azm2: ");
  Serial.print(azm2);
  Serial.print(", azmMotor1: ");
  Serial.print(azmMotor1);
  Serial.print(", azmMotor2: ");
  Serial.print(azmMotor2);
  Serial.print(", ");

   Serial.print("R1: [");
   Serial.print(analogRead(2));
   Serial.print("], ");
   Serial.print("R2: [");
   Serial.print(analogRead(3));
   Serial.print("], ");

//   int currentButtonValue = analogRead(LCD_INPUT_BUTTON);
//   Serial.print("B: [");
//   Serial.print(currentButtonValue);
//   Serial.print("], ");
//   Serial.print("R: [");
//   Serial.print(analogRead(REFERENCE_INPUT_BUTTON));
//   Serial.print("], ");
//   Serial.print("RA: [");
//   Serial.print(digitalRead(ACTION_INPUT_BUTTON));
//   Serial.print("], ");
//   Serial.print("RE: [");
//   Serial.print(digitalRead(ENCODER_INPUT_BUTTON));
//   Serial.print("], ");
//   Serial.print("knob: [");
//   Serial.print(knob.read());
//   Serial.print("], ");
  
  Serial.print("RE: [");
  Serial.print(digitalRead(ENCODER_INPUT_BUTTON));
  Serial.print("], ");
  Serial.print("POT: [");
  Serial.print(digitalRead(ENABLE_POT_BUTTON));
  Serial.print("], ");

  Serial.print("pot H: ");
  Serial.print(potHorizontal);
  Serial.print(", Pot V: ");
  Serial.print(potVertical);
  Serial.print(", H: ");
  Serial.print(potHorizontalJoystickLeft);
  Serial.print(", V: ");  
  Serial.print(potVerticalJoystickRight);
  Serial.print(", H sp.: ");
  Serial.print(leftJoystickSpeed);
  Serial.print(", V sp.: ");  
  Serial.print(rightJoystickSpeed);
  Serial.print(", H dir.: ");
  Serial.print(leftJoystickDirection);
  Serial.print(", V dir.: ");  
  Serial.print(rightJoystickDirection);
  Serial.print(", left B: ");
  Serial.print(digitalRead(LEFT_JOYSTICK_BUTTON));
  Serial.print(", right B: ");
  Serial.print(digitalRead(RIGHT_JOYSTICK_BUTTON));
  Serial.print(", newHorizontalValue: ");
  Serial.print(newHorizontalValue);
  Serial.print(", newVerticalValue: ");
  Serial.print(newVerticalValue);
  Serial.print(", lastJoystickNavigationValue: ");
  Serial.print(lastJoystickNavigationValue);
  Serial.print(", ");

  Serial.print("RTC[");

  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(now.dayOfTheWeek());
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print("], ");

  Serial.print("calibration: ");
  Serial.print("0.ra [");
  Serial.print(calibrationPoint0.ra);
  Serial.print("], ");
  Serial.print("0.dec [");
  Serial.print(calibrationPoint0.dec);
  Serial.print("], ");
  Serial.print("0.horizontalPosition [");
  Serial.print(calibrationPoint0.horizontalPosition);
  Serial.print("], ");
  Serial.print("0.verticalPosition [");
  Serial.print(calibrationPoint0.verticalPosition);
  Serial.print("], ");
  Serial.print("1.ra [");
  Serial.print(calibrationPoint1.ra);
  Serial.print("], ");
  Serial.print("1.dec [");
  Serial.print(calibrationPoint1.dec);
  Serial.print("], ");
  Serial.print("1.horizontalPosition [");
  Serial.print(calibrationPoint1.horizontalPosition);
  Serial.print("], ");
  Serial.print("1.verticalPosition [");
  Serial.print(calibrationPoint1.verticalPosition);
  Serial.print("], ");

  Serial.print("calibration: ");
  Serial.print("alt1 [");
  Serial.print(alt1);
  Serial.print("], ");
  Serial.print("alt2 [");
  Serial.print(alt2);
  Serial.print("], ");
  Serial.print("altMotor1 [");
  Serial.print(altMotor1);
  Serial.print("], ");
  Serial.print("altMotor2 [");
  Serial.print(altMotor2);
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
  Serial.print("julianDate [");
  Serial.print(julianDate);
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
  Serial.print("ra [");
  Serial.print(ra);
  Serial.print("], ");
  Serial.print("dec [");
  Serial.print(dec);
  Serial.print("], ");
  Serial.print("alt [");
  Serial.print(alt);
  Serial.print("], ");
  Serial.print("azm [");
  Serial.print(azm);
  Serial.print("], ");

  Serial.print("calcRa [");
  Serial.print(calcRa);
  Serial.print("], ");
  Serial.print("calcLst [");
  Serial.print(calcLst);
  Serial.print("], ");
  Serial.print("sinAlt [");
  Serial.print(sinAlt);
  Serial.print("], ");
  Serial.print("alt [");
  Serial.print(alt);
  Serial.print("], ");
  Serial.print("cosA [");
  Serial.print(cosA);
  Serial.print("], ");
  Serial.print("a [");
  Serial.print(a);
  Serial.print("], ");
  Serial.print("sinHa [");
  Serial.print(sinHa);
  Serial.print("]");
  
  float maxHorizontalPercentage = mapDouble(abs(horizontalMotor->getCurrentSpeedInUs()), 0, horizontalMotor->getMaxSpeedInUs(), 0, 100.0);

  Serial.println();
  Serial.print("HORIZONTAL: ");
  Serial.print("newHorizontalPos: ");
  Serial.print(newHorizontalPos);
  Serial.print(", motor. speed ticks(): ");
  Serial.print(horizontalMotor->getCurrentSpeedInUs());
  Serial.print("/ ");
  Serial.print(horizontalMotor->getMaxSpeedInUs());
  Serial.print(", pos: ");
  Serial.print(horizontalMotor->getCurrentPosition());
  Serial.print(", %: ");
  Serial.print(maxHorizontalPercentage);
  Serial.print(", wantedPos: ");
  Serial.print(newHorizontalPos);
  Serial.print(", ");

  float maxVerticalPercentage = mapDouble(abs(verticalMotor->getCurrentSpeedInUs()), 0, verticalMotor->getMaxSpeedInUs(), 0, 100.0);

  Serial.println();
  Serial.print("VERTICAL: ");
  Serial.print("newVerticalPos: ");
  Serial.print(newVerticalPos);
  Serial.print(", motor. speed ticks(): ");
  Serial.print(verticalMotor->getCurrentSpeedInUs());
  Serial.print("/ ");
  Serial.print(verticalMotor->getMaxSpeedInUs());
  Serial.print(", pos: ");
  Serial.print(verticalMotor->getCurrentPosition());
  Serial.print(", %: ");
  Serial.print(maxVerticalPercentage);
  Serial.print(", wantedPos: ");
  Serial.print(newVerticalPos);
  Serial.print("");

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