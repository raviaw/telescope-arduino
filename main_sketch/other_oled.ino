void refreshOled() {
  oledDisplay.clearDisplay(); // Clear display buffer
  
  int azimuthLineLocation;
  int altLineLocation;
  
  // right ascension 0 to 24
  // declination -90 to +90
  int raLineLocation = mapDouble(ra, 0, 24, 0, SCREEN_WIDTH);
  oledDisplay.drawFastVLine(raLineLocation, 16, SCREEN_BOTTOM_AREA_PIXELS, WHITE);

  int decLineLocation = mapDouble(dec, 90, -90, 16, SCREEN_HEIGHT);
  oledDisplay.drawFastHLine(0, decLineLocation, SCREEN_WIDTH, WHITE);
  //oledDisplay.drawFastVLine(0, decLineLocation, 1, WHITE);

  // right ascension 0 to 24
  // declination -90 to +90
  azimuthLineLocation = mapDouble(azm, 0, 360, 0, SCREEN_WIDTH);
  for (int pixelLocation = 16; pixelLocation < SCREEN_HEIGHT; pixelLocation += 2) {
    oledDisplay.drawPixel(azimuthLineLocation, pixelLocation, WHITE);
  }
  altLineLocation = mapDouble(alt, 90, -90, 16, SCREEN_HEIGHT);
  for (int pixelLocation = 0; pixelLocation < SCREEN_WIDTH; pixelLocation += 2) {
    oledDisplay.drawPixel(pixelLocation, altLineLocation, WHITE);
  }
  
  if (activeMode == MODE_CALIBRATING || activeMode == MODE_FIND) {
    // right ascension 0 to 24
    // declination -90 to +90
    int raFindLineLocation = mapDouble(lastStarRa, 0, 24, 0, SCREEN_WIDTH);
    for (int pixelLocation = 16; pixelLocation < SCREEN_HEIGHT; pixelLocation += 4) {
      oledDisplay.drawPixel(raFindLineLocation, pixelLocation, WHITE);
    }

    int decFindLineLocation = mapDouble(lastStarDec, 90, -90, 16, SCREEN_HEIGHT);
    for (int pixelLocation = 0; pixelLocation < SCREEN_WIDTH; pixelLocation += 4) {
      oledDisplay.drawPixel(pixelLocation, decFindLineLocation, WHITE);
    }
  }
  if (calibrated) {
   double currentMotorAlt = mapDouble(verticalMotor->getCurrentPosition(), altMotor1, altMotor2, alt1, alt2);
   double currentMotorAzm = mapDouble(horizontalMotor->getCurrentPosition(), azmMotor1, azmMotor2, azm1, azm2);

    if (abs(currentMotorAzm - azm) > 0.01) {
      azimuthLineLocation = mapDouble(currentMotorAzm, 0, 360, 0, SCREEN_WIDTH);
      for (int pixelLocation = 16 + (currentSecOfDay % 5); pixelLocation < SCREEN_HEIGHT; pixelLocation += 5) {
        oledDisplay.drawPixel(azimuthLineLocation, pixelLocation, WHITE);
      }
    }
    if (abs(currentMotorAlt - alt) > 0.01) {
      altLineLocation = mapDouble(currentMotorAlt, 90, -90, 16, SCREEN_HEIGHT);
      for (int pixelLocation = (currentSecOfDay % 5); pixelLocation < SCREEN_WIDTH; pixelLocation += 5) {
        oledDisplay.drawPixel(pixelLocation, altLineLocation, WHITE);
      }
    }
  }
  //oledDisplay.drawLine(raLineLocation, 16, azimuthLineLocation, SCREEN_HEIGHT, WHITE);

  //int decLineLocation = mapDouble(dec, -90, 90, 16, SCREEN_HEIGHT);
  //oledDisplay.drawLine(0, decLineLocation, SCREEN_WIDTH, decLineLocation, WHITE);

  oledDisplay.setTextSize(1);             // Normal 1:1 pixel scale
  oledDisplay.setTextColor(WHITE);        // Draw white text
  oledDisplay.setCursor(0,0);             // Start at top-left corner
  oledDisplay.print("DEC:");
  oledDisplay.print(dec, 2);
  oledDisplay.print(",RA:");
  oledDisplay.print(ra, 2);

  oledDisplay.setCursor(0,8);             // Start at top-left corner
  oledDisplay.print("AT:");
  oledDisplay.print(alt, 3);
  oledDisplay.print(",AZ:");
  oledDisplay.print(azm, 3);

  oledDisplay.display();
}