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
        activeMode = MODE_MOVING;
        break;
      case 2: 
        activeMode = MODE_CALIBRATING;
        break;
      case 3: 
        activeMode = MODE_FIND;
        break;
    }
  } else if(activeMode == MODE_MOVING) {
    resetSpeeds();
    activeMode = MODE_MENU;
  } else if(activeMode == MODE_AZ_ALT) {
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
  } else if (activeMode == MODE_MOVING) {
    drawMoving();
  } else if (activeMode == MODE_AZ_ALT) {
    drawAzimuthAltitude();
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
  renderMenuOptions(" AZ/ALT  CALI   ", " MOVE    FIND   ");
}

void drawMoving() {
  maxChoice = 0;
  renderMenuOptions(" < H:    V:     ","                ");

  printNumber(5, 0, horizontalSpeed, 0);
  printNumber(11, 0, verticalSpeed, 0);
  printNumber(0, 1, horizontalMotor.currentPosition(), 0);
  printNumber(8, 1, verticalMotor.currentPosition(), 0);
}

void drawAzimuthAltitude() {
  maxChoice = 2;
  
  switch(selectedChoice) {
    // Times
    case 0:
      printLcdPadding(0, 0, currentYear, 4, 0);
      printAt(4, 0, "/");
      printLcdPadding(5, 0, currentMonth, 2, 0);
      printAt(7, 0, "/");
      printLcdPadding(8, 0, currentDay, 2, 0);
      printAt(10, 0, " ");
      printLcdPadding(11, 0, currentHour, 2, 0);
      printAt(13, 0, ":");
      printLcdPadding(14, 0, currentMinute, 2, 0);
//       lcd.print(":");
//       printPadding(currentSecond, 2);
//       lcd.print(".");
//       printPadding(currentMs, 3);
      
      printLcdPadding(0, 1, currentSecOfDay, 5, 0);
      printLcdPadding(6, 1, timeOfDay * 10000.0, 8, 0);
      break;
    case 1:
      printAt(0, 0, "                ");
      printLcdPadding(0, 0, ra * 100L, 7, 0);
      printLcdPadding(8, 0, dec * 100L, 7, 0);
      printAt(0, 1, "                ");
      printLcdPadding(0, 1, ha * 1000L, 7, 0);
      printLcdPadding(8, 1, azm * 1000L, 7, 0);
      break;
    case 2:
      printAt(0, 0, "JLN:            ");
      printLcdPadding(4, 0, julianDate, 10, 0);
      printAt(0, 1, "                ");
      printLcdPadding(0, 1, lst * 100L, 5, 0);
      printLcdPadding(6, 1, gstTime * 100L, 5, 0);
      break;
  }
  
  lcd.setCursor(15, 1);
  lcd.print(selectedChoice);
}

void printAt(int horizontalPosition, int verticalPosition, String text) {
  lcd.setCursor(horizontalPosition, verticalPosition);
  lcd.print(text);
}

void printLcdPadding(int horizontalPosition, int verticalPosition, long value, int padding, int decimalPoints) {
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

void printNumber(int horizontalPosition, int verticalPosition, double number, int totalDigits) {
  int len;
  if (number == 0) {
    len = 1;
  } else {
    len = floor(log10(abs(number))) + 1;
  }
  if (number < 0) {
    len++;
  }

  lcd.setCursor(horizontalPosition, verticalPosition); 
  // Not much we can do about this case
  if (len >= totalDigits -1)  {
    lcd.print(number, 0);
  } else {
    lcd.print(number, totalDigits - len);
  }
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
    selectedChoice = maxChoice - 1;
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
