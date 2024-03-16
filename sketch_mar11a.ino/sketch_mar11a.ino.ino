/*
  Uses code from https://github.com/TinyCircuits/Wiki-Tutorials-Supporting-Files/blob/master/Projects/Star-Tracker/software/RTC_GPS_IMU/RTC_GPS_IMU.ino

  Useful Resources:
    Converting RA and DEC to ALT and AZ                   http://www.stargazing.net/kepler/altaz.html
    RA & DEC Conversion from HH:MM:SS to Degrees          https://www.swift.psu.edu/secure/toop/convert.htm
    Altitude & Azimuth: The Horizontal Coordinate System  https://www.timeanddate.com/astronomy/horizontal-coordinate-system.html
  References Used:
    Star Track - Arduino Powered Star Pointer and Tracker https://www.instructables.com/id/Star-Track-Arduino-Powered-Star-Pointer-and-Tracke/
    Arduino Star-Finder for Telescopes                    https://www.instructables.com/id/Arduino-Star-Finder-for-Telescopes/
*/
#include <AccelStepper.h>
#include <elapsedMillis.h>

AccelStepper verticalMotor(AccelStepper::DRIVER, 4, 3); 
AccelStepper horizontalMotor(AccelStepper::DRIVER, 6, 5); 

elapsedMillis printTime;

// Reduction X Axis = 1/ 198
// Reduction Z Axis = 1/ 33

typedef struct {
  String name; // name of the target
  
  double DEC_degrees, DEC_arcmin, DEC_arcsec;
  double RA_hour, RA_min, RA_sec;
  
  double DEC_decimal, RA_decimal;   // declination and right ascension
  double HA_decimal;
  double ALT_decimal, AZM_decimal;  // altitude and azimuth
}target;

target Polaris = {"Polaris", 89, 15, 50.78, 2, 31, 48.704};           // (Ursa Minor)
target Antares = {"Antares", -26, 25, 55.20, 16, 29, 24};             // (Scorpio)
target Arcturus = {"Arcturus", 19, 10, 56.67, 14, 15, 39};            // (Bootes)
target Betelgeuse = {"Betelgeuse", 7, 24, 25.43, 5, 55, 10};          // (Orion)
target Regulus = {"Regulus", 11, 58, 1.95, 10, 8, 22};                // (Leo)
target Spica = {"Spica", -11, 9, 40.76, 13, 25, 11};                  // (Virgo)
target Altair = {"Altair", 8, 52, 5.96, 19, 50, 46};                  // (Aquilla)
target Deneb = {"Deneb", 45, 16, 49.22, 20, 41, 25};                  // (Cygnus)
target Vega = {"Vega", 38, 47, 1.29, 18, 36, 56};                     // (Lyra)
target Andromeda_G = {"Andromeda Galaxy", 41, 16, 8.76, 0, 42, 44};   // (near the Andromeda constellation)

// array of predefined targets
target predefined[10] = {Polaris, Antares, Arcturus, Betelgeuse, Regulus, Spica, Altair, Deneb, Vega, Andromeda_G};

float gpslat = 41.08;
float gpslon = -81.517849;
int gpsyear;
byte gpsmonth, gpsday, gpshour, gpsminute, gpssecond, hundredths;
unsigned long age;
double timenow;

// Local Sidereal Time variables
double LST_degrees; // variable to store local side real time(LST) in degrees.
double LST_hours;   // variable to store local side real time(LST) in decimal hours.
double LST_minutes;
double LST_seconds

void setup() {
    Serial.begin(115200);
  // put your setup code here, to run once:
  verticalMotor.setMaxSpeed(2000.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  verticalMotor.setAcceleration(100.0);   // try other acceleration rates.
  verticalMotor.moveTo(79200); 
  horizontalMotor.setMaxSpeed(100.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizontalMotor.setAcceleration(50.0);   // try other acceleration rates.
  horizontalMotor.moveTo(-5000); 
}

float stepsPerDegree = 1000;

int count = 0;    // tracks seconds to trigger an action if desired.
void loop() {
  float verticalSpeed;
  float horizontalSpeed;
  if (printTime >= 1000) {    // reports speed and position each second
    printTime = 0;
    verticalSpeed = verticalMotor.speed();
    Serial.print("V: ");
    Serial.print(verticalSpeed);
    Serial.print("  ");
    Serial.print(verticalMotor.currentPosition());
    Serial.print("  ");
    Serial.println(count);

    horizontalSpeed = horizontalMotor.speed();
    Serial.print("H: ");
    Serial.print(horizontalSpeed);
    Serial.print("  ");
    Serial.print(horizontalMotor.currentPosition());
    Serial.print("  ");
    Serial.println(count);

    if (count++ == 75){      // uncomment an action to see the response in the serial monitor
      verticalMotor.moveTo(0);
      horizontalMotor.moveTo(0);
      //myStepper.move(100);
      //myStepper.stop();
    }
  }
  verticalMotor.run();
  horizontalMotor.run();
}

// We need to use the Hour Angle in place of the Right Ascension based on
// what time it is. We can use our LST_hours for this.
void hourAngle(target &t) {
  t.HA_decimal = LST_degrees - t.RA_decimal;
}

// converts Right Ascension from HH:MM:SS to degrees
double HrMinSec_to_Deg(int h, int m, double s) {
  return 15 * (h + ((double)m/60.0) + ((double)s/3600.0));
}

// converts Declination from degrees, arcminutes, and arcseconds to decimal degrees
double DegArcminArcsec_to_Deg(int d, int am, double as) {
  return (d + ((double)am/60.0) + ((double)as/3600.0));
}

// converts Declination from decimal degrees to degrees, arcminutes, and arcseconds
void deg_to_DegArcminArcsec(target &t) {
  t.DEC_degrees = floor(t.DEC_decimal); // DEC in hours
  t.DEC_arcmin = (t.DEC_decimal - t.DEC_degrees) * 60;
  t.DEC_arcsec = (t.DEC_arcmin - floor(t.DEC_arcmin)) * 60;
  
  t.DEC_arcmin = floor(t.DEC_arcmin);

  // this isn't quite exact, but it's very, very close
}

// converts Right Ascension from HH:MM:SS to decimal degrees
void setDecimalCoords(target &t) {
  t.DEC_decimal = t.DEC_degrees + (t.DEC_arcmin/60.0) + (t.DEC_arcsec/3600.0);
  t.RA_decimal =  15 * (t.RA_hour + (t.RA_min/60.0) + (t.RA_sec/3600.0));
}

// converts Right Ascension from decimal degrees to HH:MM:SS
void deg_to_HrMinSec(target &t) {
  t.RA_hour = floor(t.RA_decimal/15.0); // RA in hours
  t.RA_min = ((t.RA_decimal/15.0) - t.RA_hour) * 60;
  t.RA_sec = (t.RA_min - floor(t.RA_min)) * 60;
  
  t.RA_min = floor(t.RA_min);

  // this isn't quite exact, but it's very, very close
}
// Converts the target's Equatorial Coordinates to Altazimuth Coordinates given
// the current Local Sidereal Time and update the variables in the target object.
void setAltazimuthCoords(target &t) {
  float DEG2RAD = 71.0/4068.0; // value used to convert degrees to radians
  float RAD2DEG = 4068.0/71.0; // value used to convert radians to degrees

  double sin_DEC = sin(t.DEC_decimal * DEG2RAD);
  double cos_DEC = cos(t.DEC_decimal * DEG2RAD);
  
  double sin_LAT = sin(gpslat * DEG2RAD);
  double cos_LAT = cos(gpslat * DEG2RAD);

  LST_time();   // update LST for the most accurate calculation of Horizontal coordinates
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
