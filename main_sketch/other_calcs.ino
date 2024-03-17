/////////////////////////////////////////////////////////////////////////////
//
// Date conversion functions
// Azimuth and shit
//
//

double julianDateCalc() { //For more info see 'Practical Astronomy With Your Calculator'.
  double thisDay = (currentDay - 1.0) + (currentSecOfDay / (3600L * 24L));
  double gpsMonth = currentMonth;
  double gpsYear = currentYear;
  if (gpsMonth == 1 || gpsMonth == 2) {
    gpsMonth = gpsMonth + 12;
    gpsYear = gpsYear - 1;
  }
  // I will just trust it 
  int a = floor ((double)gpsYear / 100.0);
  int b = 2 - a + floor (a / 4.0);
  long c = (365.25 * (double)gpsYear);
  float d = floor (30.6001 * ((double)gpsMonth + 1));
  float jd = b + c + d + (double)thisDay + 1720994.5;

  return jd; //'jd' being the Julian Date.
}

double utcToGstCalc() { //Converts UTC (Univeral Time) to GST (Greenwich Sidereal Time). 
  double s = julianDate - 2451545.0;
  double t = s / 36525.0;
  double step1 = (2400.051336 * t);
  double step2 = (t * t);
  double step3 = (0.000025862 * step2);
  double to = (6.697374558 + step1 + step3);
  double n1 = floor (to / 24);
  to = to - (n1 * 24);
  double h1 = (timeOfDay * 1.002737909);
  double n2 = floor ((to + h1) / 24.0);
  double gst = (to + h1) - (n2 * 24.0);
  return gst;
}

float gstToLstCalc() { //Converts GST (Greenwich Sidereal Time) to LST (Local Sidereal Time).
  double timeVariable = gstTime; 
  double diff = abs(gpsLongitude);
  diff = (diff / 15.0);
  double lst;
  if ((gpsLongitude * -1) > 0) {
    timeVariable = timeVariable - diff;
  } else {
    timeVariable = timeVariable + diff;
  }
  if (timeVariable > 24) {
    lst = timeVariable - 24;
  } else if ((timeVariable * -1) > 0) {
    lst = timeVariable + 24;
  } else {
    lst = timeVariable;
  }
  
  return lst;
}

void azimuthAltitudeCalculation() { //This section calculates the Azimuth and the Altitude of the target object.
  double targetRa = ra;
  double targetDec = dec;
  
  targetRa = (targetRa / 15.0);
  double h = 15.0 * (lst - targetRa);
  h = (h / 360.0) * (2 * PI);
  targetDec = ((targetDec / 360.0) * (2 * PI));
  double sinDec = sin(targetDec);
  double sinLat = sin(gpsLatitude);
  double cosDec = cos(targetDec);
  double cosLat = cos(gpsLatitude);
  double jeremy = cos(h);
  double sinAltitude = (sinDec * sinLat) + (cosDec * cosLat * jeremy);
  double alt = asin(sinAltitude);
  double cosAltitude = cos(alt);
  alt = ((alt / (2 * PI)) * 360);
  double cosAz = (sinDec - (sinLat * sinAltitude)) / (cosLat * cosAltitude);
  double az = ((acos(cosAz)) * 4068) / 71;
  double sinHh = sin(h);
  if ((sinHh * -1) > 0) {
    az = az;
  } else {
    az = 360.0 - az;
  }
  
  ha = alt;
  azm = az;

//   if (n == 666) { //This variable will only equal 666 when the Arduino is first powered on, so it triggers the calibration sequence.
//     centralstate = digitalRead(central);
//     while (centralstate == LOW) { //Keep reading the potentiometer values until the central button is pressed.
//       altpot = ads1015.readADC_SingleEnded(1);
//       azpot = ads1015.readADC_SingleEnded(0);
//       centralstate = digitalRead(central);
//     }
//     calibalt = altpot; 
//     calibaz = azpot;
//     polarisalt = alt;
//     polarisaz = az;
//     lcd.setCursor(0, 0);
//     lcd.print("    COMPLETE    "); //Calibration sequence is complete.
//     lcd.setCursor(0, 1);
//     lcd.print("                ");
//     delay(2000);
//   }
// 
//   else {
//     if ((alt * -1) > 0) { //If altitude is below 0 degrees, then the object is below the observer's horizon.
//       lcd.setCursor(0, 1);
//       lcd.print("IS BELOW HORIZON"); 
//       digitalWrite(ledup, LOW);
//       digitalWrite(leddown, LOW);
//       digitalWrite(ledright, LOW);
//       digitalWrite(ledleft, LOW);
//     }
//     else {
//       lcd.setCursor(0, 1); //This starts the next text to be printed at a specific character on the LCD screen.
//       lcd.print(az, 3);
//       lcd.setCursor(7, 1);
//       lcd.print(" / ");
//       lcd.setCursor(10, 1);
//       lcd.print(alt, 3);
//       altpot = ads1015.readADC_SingleEnded(1);
//       azpot = ads1015.readADC_SingleEnded(0);
//       if (alt < polarisalt) {
//         altpotgood = (calibalt + ((polarisalt - alt) * 6.47777));
//       }
//       else if (alt > polarisalt) {
//         altpotgood = (calibalt - ((alt - polarisalt) * 6.47777));
//       }
//       if (az < polarisaz) {
//         if (az > 180) {
//           azpotgood = (calibaz + ((polarisaz - az) * 4.88529));
//         }
//         else if (az < 180) {
//           azpotgood = (calibaz - (((az + 360) - polarisaz) * 4.88529));
//         }
//       }
//       else if (az > polarisaz) {
//         if (az > 180) {
//           azpotgood = (calibaz + (((polarisaz + 360.0) - az) * 4.88529));
//         }
//         else if (az < 180) {
//           azpotgood = (calibaz - ((az - polarisaz) * 4.88529));
//         }
//       }
//       if (altpot == altpotgood) { //This section controls which of the four LEDs are lit, to indicate which direction the telescope needs to be moved in.
//         digitalWrite(ledup, HIGH);
//         digitalWrite(leddown, HIGH);
//       }
//       else {
//         if (altpot > altpotgood) {
//           digitalWrite(ledup, HIGH);
//           digitalWrite(leddown, LOW);
//         }
//         else if (altpot < altpotgood) {
//           digitalWrite(ledup, LOW);
//           digitalWrite(leddown, HIGH);
//         }
//       }
//       if (azpot == azpotgood) {
//         digitalWrite(ledright, HIGH);
//         digitalWrite(ledleft, HIGH);
//       }
//       else {
//         if (azpot > azpotgood) {
//           digitalWrite(ledright, HIGH);
//           digitalWrite(ledleft, LOW);
//         }
//         else if (azpot < azpotgood) {
//           digitalWrite(ledright, LOW);
//           digitalWrite(ledleft, HIGH);
//         }
//         if (azpot == azpotgood) {
//           digitalWrite(ledright, HIGH);
//           digitalWrite(ledleft, HIGH); 
//         }
//       }
//     }
//   }
//   delay(250); //Wait 250ms and the repeat.
}
