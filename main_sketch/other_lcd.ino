#define MODE_REPORT 1
#define MODE_MOVING 2
#define MODE_CALIBRATING 3


void reportLcd(target& obj) {
  // ################
  // ################
  // 0123456789012345

  // VEGA XX.XX/YY.YY
  // XX.X XX.X XXX.XX

  lcd.setCursor(0, 0); 
  lcd.print("                "); 
  lcd.setCursor(0, 1); 
  lcd.print("                "); 

  lcd.setCursor(0, 0); 
  lcd.print(obj.name); 
  lcd.setCursor(4, 0); 
  lcd.print(" "); 

  printNumber(5, 0, obj.DEC_degrees, 2);
  lcd.print("."); 
  printNumber(8, 0, obj.DEC_arcmin, 2);

  printNumber(11, 0, obj.RA_hour, 2);
  lcd.print("."); 
  printNumber(14, 0, obj.RA_min, 2);

  printNumber(0, 1, obj.HA_decimal, 4);
  printNumber(5, 1, obj.AZM_decimal, 4);
  printNumber(10, 1, obj.ALT_decimal, 5);
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