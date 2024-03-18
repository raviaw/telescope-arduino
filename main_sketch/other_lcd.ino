int lastButtonAction = -1;
#define RIGHT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define SELECT 5

int selectedChoice = 0;
int maxChoice = 0;

void registerButton() {
  int newAction = -1;

  int currentButtonValue = analogRead(0);
  if (currentButtonValue > 1000) {
    // Button UP
    currentButtonValue = 1000;
    newAction = -1;
  } else if (currentButtonValue < 80) {
    newAction = RIGHT;
  } else if (currentButtonValue < 200) {
    newAction = UP;
  } else if (currentButtonValue < 400) {
    newAction = DOWN;
  } else if (currentButtonValue < 600) {
    newAction = LEFT;
  } else if (currentButtonValue < 800) {
    newAction = SELECT;
  }

  if (newAction == -1 && lastButtonAction != -1) {
    buttonPressed(lastButtonAction);
    lastButtonAction = -1;
  }
  lastButtonAction = newAction;
}

void buttonSelect() {
  if (activeMode == MODE_MENU) {
    switch(selectedChoice) {
      // 0 = AZ/ALT
      // 1 - CALI
      // 2 - MOVE
      // 3 - FIND
      case 0: 
        activeMode = MODE_AZ_ALT;
        break;
      case 1: 
        activeMode = MODE_MOVE_MENU;
        break;
      case 2: 
        activeMode = MODE_CALIBRATING;
        calibratingStarIndex = 0;
        break;
      case 3: 
        activeMode = MODE_FIND;
        break;
    }
  } else if( activeMode == MODE_MOVE_MENU) {
    switch(selectedChoice) {
      case 0: 
        activeMode = MODE_MOVE_COORDINATES;
        break;
      case 1: 
        activeMode = MODE_MOVE_MOTOR;
        break;
    }
  } else if(activeMode == MODE_CALIBRATING) {
    switch(selectedChoice) {
      case 0:
        calibratingTarget = sirius;
        break;
      case 1:
        calibratingTarget = alphaCentauri;
        break;
      case 2:
        calibratingTarget = canopus;
        break;
      case 3:
        calibratingTarget = acrux;
        break;
    }
    activeMode = MODE_CALIBRATE_PICKED_STAR;
  } else if(activeMode == MODE_CALIBRATE_PICKED_STAR) {
    activeMode = MODE_CALIBRATE_MOVING;
  } else if(activeMode == MODE_CALIBRATE_MOVING) {
    if (calibratingStarIndex < 2) {
      storeCalibrateCoordinates();
      calibratingStarIndex++;
    }
    activeMode = MODE_CALIBRATE_SELECTED;
  } else if(activeMode == MODE_CALIBRATE_SELECTED) {
    if (calibratingStarIndex == 2) {
      storeCalibrationData();
      activeMode = MODE_CALIBRATION_COMPLETE;
    } else {
      activeMode = MODE_CALIBRATING;
      calibratingStarIndex++;
    }  
  } else {
    resetSpeeds();
    activeMode = MODE_MENU;
  }
    
  selectedChoice = 0;
}

void reportLcd() {
  // ################
  // ################
  // 0123456789012345

  // VEGA XX.XX/YY.YY
  // XX.X XX.X XXX.XX
  if (activeMode == MODE_MENU) {
    drawMainMenu();
  } else if (activeMode == MODE_FIND) {
    drawFindMenu();
  } else if (activeMode == MODE_MOVE_MENU) {
    drawMoveMenu();
  } else if (activeMode == MODE_MOVE_COORDINATES) {
    drawMovingCoordinates();
  } else if (activeMode == MODE_MOVE_MOTOR) {
    drawMovingMotor();
  } else if (activeMode == MODE_AZ_ALT) {
    drawAzimuthAltitude();
  } else if (activeMode == MODE_CALIBRATING) {
    drawCalibrateMenu();
  } else if (activeMode == MODE_CALIBRATE_PICKED_STAR) {
    drawCalibratePickedStar();
  } else if (activeMode == MODE_CALIBRATE_MOVING) {
    drawCalibrateMoving();
  } else if (activeMode == MODE_CALIBRATE_SELECTED) {
    drawCalibrateSelected();
  } else if (activeMode == MODE_CALIBRATION_COMPLETE) {
    drawCalibrationComplete();
  } else {
    // Don't know what to do 
  }

  // printNumber(5, 0, obj.DEC_degrees, 2);
  // lcd.print("."); 
  // printNumber(8, 0, obj.DEC_arcmin, 2);

  // printNumber(11, 0, obj.RA_hour, 2);
  // lcd.print("."); 
  // printNumber(14, 0, obj.RA_min, 2);

  // printNumber(0, 1, obj.HA_decimal, 4);
  // printNumber(5, 1, obj.AZM_decimal, 4);
  // printNumber(10, 1, obj.ALT_decimal, 5);
}

void drawMainMenu() {
  // [0] MAIN#[2]#CALB
  // [3] MOVE#########
  // 0123456789012345
  maxChoice = 3;
  if (calibrated) {
    renderMenuOptions(" AZ/ALT  CAL[Y] ", " MOVE    FIND   ");
  } else {
    renderMenuOptions(" AZ/ALT  CAL[N] ", " MOVE    FIND   ");
  }
}

void drawMoveMenu() {
  maxChoice = 1;
  renderMenuOptions(" MOVE COORDINATE", " MOVE MOTOR    ");
}

void drawFindMenu() {
  maxChoice = 0;
  renderMenuOptions(" FIND NOT YET   ", " IMPLEMENTED    ");
}

void drawMovingCoordinates() {
  maxChoice = 0;
  renderMenuOptions(" < H:    V:     ","                ");

  printNumber(5, 0, horizontalSpeed, 0);
  printNumber(11, 0, verticalSpeed, 0);
  printNumber(0, 1, horizontalMotor.currentPosition(), 0);
  printNumber(8, 1, verticalMotor.currentPosition(), 0);
}

void drawMovingMotor() {
  maxChoice = 0;
  renderMenuOptions(" < H:    V:     ","                ");

  printNumber(5, 0, horizontalSpeed, 0);
  printNumber(11, 0, verticalSpeed, 0);
  printNumber(0, 1, horizontalMotor.currentPosition(), 0);
  printNumber(8, 1, verticalMotor.currentPosition(), 0);
}

void drawCalibrateMenu() {
  // [0] MAIN#[2]#CALB
  // [3] MOVE#########
  // 0123456789012345
  maxChoice = 4;
  renderMenuOptions(" SIRIUS  CAN.   ", " A. CEN  ACRUX  ");
  printNumber(15, 1, calibratingStarIndex, 0);
}

void drawCalibratePickedStar() {
  maxChoice = 0;
  lcd.setCursor(0, 0); 
  lcd.print("                ");
  lcd.setCursor(0, 1); 
  lcd.print("                "); 
  
  printAt(0, 0, "Find");
  printAt(5, 0, calibratingTarget.name);
  lcd.print(":");
  printFloatingPointNumber(0, 1, calibratingTarget.ra, 7, 4);
  printFloatingPointNumber(8, 1, calibratingTarget.dec, 7, 4);
}

void drawCalibrateMoving() {
  maxChoice = 0;
  renderMenuOptions(" < H:    V:     ","                ");

  printNumber(5, 0, horizontalSpeed, 0);
  printNumber(11, 0, verticalSpeed, 0);
  printNumber(0, 1, horizontalMotor.currentPosition(), 0);
  printNumber(8, 1, verticalMotor.currentPosition(), 0);
}

void drawCalibrateSelected() {
  maxChoice = 0;
  lcd.setCursor(0, 0); 
  lcd.print("Star coordinates");
  lcd.setCursor(0, 1); 
  lcd.print("... stored      "); 
}

void drawCalibrationComplete() {
  maxChoice = 0;
  lcd.setCursor(0, 0); 
  lcd.print("Calibration is  ");
  lcd.setCursor(0, 1); 
  lcd.print("... completed   "); 
}

void drawAzimuthAltitude() {
  maxChoice = 7;
  
  switch(selectedChoice) {
    // Times
    case 0:
      printAt(0, 0, "> DATE/ TIME:  ");
      printLcdPadding(0, 1, currentYear, 4, 0);
      printAt(4, 1, "/");
      printLcdPadding(5, 1, currentMonth, 2, 0);
      printAt(7, 1, "/");
      printLcdPadding(8, 1, currentDay, 2, 0);
      printAt(10, 1, " ");
      printLcdPadding(11, 1, currentHour, 2, 0);
      printAt(13, 1, ":");
      printLcdPadding(14, 1, currentMinute, 2, 0);
//       lcd.print(":");
//       printPadding(currentSecond, 2);
//       lcd.print(".");
//       printPadding(currentMs, 3);
      break;
    case 1:
      printAt(0, 0, "> SEC/D T/DAY:  ");
      printAt(0, 1, "                ");
      printNumber(0, 1, currentSecOfDay, 5);
      printFloatingPointNumber(6, 1, timeOfDay, 8, 6);
      break;
    case 2:
      printAt(0, 0, "> JULIAN DATE:  ");
      printAt(0, 1, "                ");
      printFloatingPointNumber(1, 1, julianDate, 10, 2);
      break;
    case 3:
      printAt(0, 0, "> LST/ GMT:     ");
      printAt(0, 1, "                ");
      printFloatingPointNumber(0, 1, lst, 7, 5);
      printFloatingPointNumber(8, 1, gstTime, 7, 5);
      break;
    case 4:
      printAt(0, 0, "> RA/ DEC:       ");
      printAt(0, 1, "                ");
      printFloatingPointNumber(0, 1, ra, 7, 5);
      printFloatingPointNumber(8, 1, dec, 7, 5);
      break;
    case 5:
      printAt(0, 0, "> HA/ AZM:      ");
      printAt(0, 1, "                ");
      printFloatingPointNumber(0, 1, ha, 7, 4);
      printFloatingPointNumber(8, 1, azm, 7, 4);
      break;
    case 6:
      printAt(0, 0, "> HRZ M POS:    ");
      printAt(0, 1, "                ");
      printFloatingPointNumber(1, 1, horizontalMotor.currentPosition(), 14, 0);
      break;
    case 7:
      printAt(0, 0, "> VER M POS:    ");
      printAt(0, 1, "                ");
      printFloatingPointNumber(1, 1, verticalMotor.currentPosition(), 14, 0);
      break;
  }
  
  lcd.setCursor(15, 0);
  lcd.print(selectedChoice);
}

void printAt(int horizontalPosition, int verticalPosition, String text) {
  lcd.setCursor(horizontalPosition, verticalPosition);
  lcd.print(text);
}

void resetSpeeds() {
}

void renderMenuOptions(String line1, String line2) {
  // *MAIN   *CALI
  // *MOVE   *FIND
  // 0123456789012345
  lcd.setCursor(0, 0); 
  lcd.print(line1); 
  lcd.setCursor(0, 1); 
  lcd.print(line2);
  switch(selectedChoice) {
    case 0:
      lcd.setCursor(0, 0);
      break; 
    case 1:
      lcd.setCursor(0, 1);
      break; 
    case 2:
      lcd.setCursor(8, 0);
      break; 
    case 3:
      lcd.setCursor(8, 1);
      break; 
  } 
  lcd.print("*"); 
  lcd.setCursor(15, 0);
  lcd.print(selectedChoice);
}

void printLcdPadding(int horizontalPosition, int verticalPosition, double value, int padding, int decimalPoints) {
  lcd.setCursor(horizontalPosition, verticalPosition);

  int len;
  if (value == 0) {
    len = 1;
  } else {
    len = floor(log10(value)) + 1;
  }
  if (decimalPoints > 0) {
    len += decimalPoints + 1;
  }
  int negative = 0;
  if(value < 0) {
    padding--;
    lcd.print("-");
  }
  for (int i = 0; i < padding - len; i++) {
    lcd.print("0");
  }
  lcd.print(abs(value));
}

void printNumber(int horizontalPosition, int verticalPosition, double number, int totalDigits) {
  lcd.setCursor(horizontalPosition, verticalPosition); 
  lcd.print(number, 0);
}

void printFloatingPointNumber(int horizontalPosition, int verticalPosition, double number, int totalDigits, int floatingPointDigits) {
  lcd.setCursor(horizontalPosition, verticalPosition); 
  lcd.print(number, floatingPointDigits);
}

void buttonPressed(int buttonNumber) {
  switch(buttonNumber) {
    case RIGHT:
      buttonRight();
      break;
    case UP:
      buttonUp();
      break;
    case DOWN:
      buttonDown();
      break;
    case LEFT:
      buttonLeft();
      break;
    case SELECT:
      buttonSelect();
      break;
  }
}

void buttonRight() {
  selectedChoice += 2;
  if (selectedChoice > maxChoice) {
    selectedChoice = 0;
  }
}
void buttonUp() {
  selectedChoice --;
  if (selectedChoice < 0) {
    selectedChoice = maxChoice;
  }
}
void buttonDown() {
  selectedChoice ++;
  if (selectedChoice > maxChoice) {
    selectedChoice = 0;
  }
}
void buttonLeft() {
  selectedChoice -= 2;
  if (selectedChoice < 0) {
    selectedChoice = maxChoice - 1;
  }
}

int translatePotValueToSpeed(int value) {
  int testValue; 
  if (value < 512) {
    testValue = 512 - value;
  } else {
    testValue = value - 512;
  }
  
  int multiply = -1;
  if (value > 512) {
    multiply = 1;
  }
  
  if (testValue < 30) {
    return 0; 
  } else {
    return multiply * ((testValue -30) / (480 / 100)); // 100 stops 
  }
}
