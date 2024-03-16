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
  lcd.print(obj.DEC_degrees, 0);
  lcd.setCursor(7, 0); 
  lcd.print("."); 
  lcd.print(obj.DEC_arcmin, 0);
  lcd.setCursor(10, 0); 
  lcd.print("/"); 
  lcd.print(obj.RA_hour, 0);
  lcd.setCursor(13, 0); 
  lcd.print("."); 
  lcd.print(obj.RA_min, 0);

  lcd.setCursor(0, 1); 
  lcd.print(obj.HA_decimal, 1);
  lcd.setCursor(5, 1); 
  lcd.print(obj.ALT_decimal, 1);
  lcd.setCursor(10, 1); 
  lcd.print(obj.AZM_decimal, 2);
}
