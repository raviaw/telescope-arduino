// /////////////////////////////////////////////////////////////////////////////
// //
// // Date conversion functions
// // Azimuth and shit
// //
// //

// double julianDateCalc() { //For more info see 'Practical Astronomy With Your Calculator'.
//   double thisDay = (currentDay - 1.0) + (currentSecOfDay / (3600L * 24L));
//   double gpsMonth = currentMonth;
//   double gpsYear = currentYear;
//   if (gpsMonth == 1 || gpsMonth == 2) {
//     gpsMonth = gpsMonth + 12;
//     gpsYear = gpsYear - 1;
//   }
//   // I will just trust it 
//   int a = floor ((double)gpsYear / 100.0);
//   int b = 2 - a + floor (a / 4.0);
//   long c = (365.25 * (double)gpsYear);
//   float d = floor (30.6001 * ((double)gpsMonth + 1));
//   float jd = b + c + d + (double)thisDay + 1720994.5;

//   return jd; //'jd' being the Julian Date.
// }

// double utcToGstCalc() { //Converts UTC (Univeral Time) to GST (Greenwich Sidereal Time). 
//   double s = julianDate - 2451545.0;
//   double t = s / 36525.0;
//   double step1 = (2400.051336 * t);
//   double step2 = (t * t);
//   double step3 = (0.000025862 * step2);
//   double to = (6.697374558 + step1 + step3);
//   double n1 = floor (to / 24);
//   to = to - (n1 * 24);
//   double h1 = (timeOfDay * 1.002737909);
//   double n2 = floor ((to + h1) / 24.0);
//   double gst = (to + h1) - (n2 * 24.0);
//   return gst;
// }

// float gstToLstCalc() { //Converts GST (Greenwich Sidereal Time) to LST (Local Sidereal Time).
//   double timeVariable = gstTime; 
//   double diff = abs(gpsLongitude);
//   diff = (diff / 15.0);
//   double lst;
//   if ((gpsLongitude * -1) > 0) {
//     timeVariable = timeVariable - diff;
//   } else {
//     timeVariable = timeVariable + diff;
//   }
//   if (timeVariable > 24) {
//     lst = timeVariable - 24;
//   } else if ((timeVariable * -1) > 0) {
//     lst = timeVariable + 24;
//   } else {
//     lst = timeVariable;
//   }
  
//   return lst;
// }
// void azimuthAltitudeCalculation() { //This section calculates the Azimuth and the Altitude of the target object.
//   double piConversion = PI / 180.0;
//   double radiansConversion = 180.0 / PI;
//   calcLst = lst * 15.0 * piConversion;
//   calcRa = ra * 15.0 * piConversion;
//   double haDiff = calcLst - calcRa;
//   if (haDiff < 0) {
//     haDiff += 2 * PI;
//   }
  
//   sinAlt = sin(dec * piConversion) * sin(gpsLatitude * piConversion) + (cos(dec * piConversion) * cos(gpsLatitude * piConversion) * cos(haDiff));
//   alt = asin(sinAlt);
//   cosA = (sin(dec* piConversion) - (sin(alt) * sin(gpsLatitude * piConversion))) / (cos(alt) * cos(gpsLatitude* piConversion));
//   a = acos(cosA);
//   sinHa = sin(haDiff);
//   if (sinHa < 0) {
//     azm = a;
//   } else {
//     azm = 2 * PI - a;
//   }

//   alt = alt * radiansConversion;
//   azm = azm * radiansConversion;
//   // alt = alt * radiansConversion;
// }

// void azimuthAltitudeCalculationOld() { //This section calculates the Azimuth and the Altitude of the target object.
//   double targetDec = dec;
//   double targetRa = ra;
  
//   targetRa = (targetRa / 15.0);
//   double h = 15.0 * (lst - targetRa);
//   h = (h / 360.0) * (2 * PI);
//   targetDec = ((targetDec / 360.0) * (2 * PI));
//   double sinDec = sin(targetDec);
//   double sinLat = sin(gpsLatitude);
//   double cosDec = cos(targetDec);
//   double cosLat = cos(gpsLatitude);
//   double jeremy = cos(h);
//   double sinAltitude = (sinDec * sinLat) + (cosDec * cosLat * jeremy);
//   double alt = asin(sinAltitude);
//   double cosAltitude = cos(alt);
//   alt = ((alt / (2 * PI)) * 360);
//   double cosAz = (sinDec - (sinLat * sinAltitude)) / (cosLat * cosAltitude);
//   double az = ((acos(cosAz)) * 4068) / 71;
//   double sinHh = sin(h);
//   if ((sinHh * -1) > 0) {
//     az = az;
//   } else {
//     az = 360.0 - az;
//   }
  
//   alt = alt;
//   azm = az;
// }

