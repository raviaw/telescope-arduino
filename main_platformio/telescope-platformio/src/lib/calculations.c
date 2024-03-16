#include "include/types.h"
#include "include/objects.h"
#include "../library/calculations.h"
/////////////////////////////////////////////////////////////////////////////
//
// Date conversion functions
// Azimuth and shit
//
//

// We need to use the Hour Angle in place of the Right Ascension based on
// what time it is. We can use our LST_hours for this.
void hourAngle(target t) {
  t.HA_decimal = LST_degrees - t.RA_decimal;
}

// converts Right Ascension from HH:MM:SS to degrees
double hourMinSecToDegree(int h, int m, double s) {
  return 15 * (h + ((double)m/60.0) + ((double)s/3600.0));
}

// converts Declination from degrees, arcminutes, and arcseconds to decimal degrees
double degreesToDecimalDegrees(int d, int am, double as) {
  return (d + ((double)am/60.0) + ((double)as/3600.0));
}

// converts Declination from decimal degrees to degrees, arcminutes, and arcseconds
void decimalDegreesToDegrees(target t) {
  t.DEC_degrees = floor(t.DEC_decimal); // DEC in hours
  t.DEC_arcmin = (t.DEC_decimal - t.DEC_degrees) * 60;
  t.DEC_arcsec = (t.DEC_arcmin - floor(t.DEC_arcmin)) * 60;
  
  t.DEC_arcmin = floor(t.DEC_arcmin);

  // this isn't quite exact, but it's very, very close
}

// converts Right Ascension from decimal degrees to HH:MM:SS
void rightAscensionFromDecimalToHourMinSec(target t) {
  t.RA_hour = floor(t.RA_decimal/15.0); // RA in hours
  t.RA_min = ((t.RA_decimal/15.0) - t.RA_hour) * 60;
  t.RA_sec = (t.RA_min - floor(t.RA_min)) * 60;
  
  t.RA_min = floor(t.RA_min);

  // this isn't quite exact, but it's very, very close
}

//
// Local sidereal time is just based on the date/ time for other calculations
//
void calculateLocalSiderealTime() {
  // Calculates local sidereal time based on this calculation,
  // http://www.stargazing.net/kepler/altaz.html
  
  // get dates from RTC, which has been initialized by the GPS module
  double M  = currentMonth;
  double Y  = currentYear;
  double D  = currentDay;
  double MN = currentMinute;
  double H  = currentHour;
  double S  = currentSecond;
  double A  = (double)(Y - 2000) * 365.242199;  // days since Jan 1 2000 to beginning of the year
  double B  = (double)(M - 1) * 30.4368499;     // days since the beginning of the year

  int Amo[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  double B_2020 = 7303.5; // days since J2000 to beginning of 2020
  double JDN2000;
  
  if(((int)Y / 4) && (M > 2)) { // if it's a leap year and past the end of February (leap day)
    JDN2000 = Amo[currentMonth - 1] + 1 + B_2020 + D + (H/24.0);
    //JDN2000 = Amo[rtc.getMonth() - 1] + 1 + B_2020 + D-1 + (H/24.0);
  } else {  // if it's not a leap year, don't add the extra day
    JDN2000 = Amo[currentMonth - 1] + B_2020 + D + (H/24.0);
    //JDN2000 = Amo[rtc.getMonth() - 1] + 1 + B_2020 + D-1 + (H/24.0);
  }
      
  //double JDN2000 = A + B + (D - 1) + H/24.0;
  double decimal_time = H + (MN/60) + (S/3600); // this is our UT (Universal Time) value
  double LST = 100.46 + 0.985647 * JDN2000 + gpslon + 15 * decimal_time; // partial step to get LST in degrees
  LST_degrees = (LST - (floor(LST/360) * 360)); // LST in degrees
  LST_hours = LST_degrees/15; // LST in hours
  LST_minutes = (LST_hours - floor(LST_hours)) * 60;
  LST_seconds = (LST_minutes - floor(LST_minutes)) * 60;
}

// Converts the target's Equatorial Coordinates to Altazimuth Coordinates given
// the current Local Sidereal Time and update the variables in the target object.
void setAltazimuthCoords(target t) {
  float DEG2RAD = 71.0/4068.0; // value used to convert degrees to radians
  float RAD2DEG = 4068.0/71.0; // value used to convert radians to degrees

  double sin_DEC = sin(t.DEC_decimal * DEG2RAD);
  double cos_DEC = cos(t.DEC_decimal * DEG2RAD);
  
  double sin_LAT = sin(gpslat * DEG2RAD);
  double cos_LAT = cos(gpslat * DEG2RAD);

  // LST_time();   // update LST for the most accurate calculation of Horizontal coordinates
  hourAngle(t); // this needs updated constantly, so we update it before deciding on an Altazimuth coord at that second
  double sin_HA = sin(t.HA_decimal * DEG2RAD);
  double cos_HA = cos(t.HA_decimal * DEG2RAD);

  // formulas from http://www.stargazing.net/kepler/altaz.html#twig04a
  t.ALT_decimal = asin( sin_DEC*sin_LAT + cos_DEC*cos_LAT*cos_HA );

  double sin_ALT = sin((float)t.ALT_decimal);
  double cos_ALT = cos((float)t.ALT_decimal);
  
  t.ALT_decimal *= RAD2DEG; // convert ALT back to degrees
  
  t.AZM_decimal = acos( (sin_DEC - sin_ALT*sin_LAT) / (cos_ALT*cos_LAT) ) * RAD2DEG;

  if(sin_HA > 0) {                          // if sin(HourAngle) is positive,
    t.AZM_decimal = 360.0 - t.AZM_decimal;  // then the Azimuth is 360 - (AZM)
  }                                         // otherwise, the Azimuth is AZM
  
  //Use to debug Altitude and Azimuth calculations
  //SerialMonitorInterface.print("ALT: ");
  //SerialMonitorInterface.println(t.ALT_decimal);
  //SerialMonitorInterface.print("AZM: ");
  //SerialMonitorInterface.println(t.AZM_decimal);
}
