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
#include <LiquidCrystal.h> // Inclui biblioteca "LiquidCristal.h"
 
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

AccelStepper verticalMotor(AccelStepper::DRIVER, 4, 3); 
AccelStepper horizontalMotor(AccelStepper::DRIVER, 6, 5); 

elapsedMillis printTime;

// Reduction X Axis = 1/ 198
// Reduction Z Axis = 1/ 33

// DEC -> Declination
// RA -> Right Ascension
// ALT -> Altitude
// AZM - Azimuth
typedef struct {
  String name; // name of the target
  
  // Predefined
  double DEC_degrees, DEC_arcmin, DEC_arcsec;
  // Predefined
  double RA_hour, RA_min, RA_sec;
  
  // Calculated
  double DEC_decimal, RA_decimal;   // declination and right ascension
  // Calculated
  double HA_decimal;
  // Calculated
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

float gpslat = -22.6599734;
float gpslon = -46.9420532;
//unsigned long age;
//double timenow;


// Local Sidereal Time variables
double LST_degrees; // variable to store local side real time(LST) in degrees.
double LST_hours;   // variable to store local side real time(LST) in decimal hours.
double LST_minutes;
double LST_seconds;

// Buttons
int lastButtonAction = -1;
#define RIGHT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define SELECT 5

int startYear;
int startMonth;
int startDay;
int startHour;
int startMinute;
int startSecond;
int startMs;
long startSecOfDay;
long startTimeMs;
int currentYear;
int currentMonth;
int currentDay;
int currentHour;
int currentMinute;
int currentSecond;
int currentMs;
long currentSecOfDay;

void setup() {
  lcd.begin(16,2); //SETA A QUANTIDADE DE COLUNAS(16) E O NÚMERO DE LINHAS(2) DO DISPLAY_. EM SUMA: UMA MATRIZ DE 16 COLUNAS E 2 LINHAS
  lcd.setCursor(0,0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1) 
  lcd.print("TELESCOPE"); 

  Serial.begin(115200);

  parseReceivedTimeString("2024-15-03 12:05:00.123");

  // put your setup code here, to run once:
  verticalMotor.setMaxSpeed(2000.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  verticalMotor.setAcceleration(100.0);   // try other acceleration rates.
  verticalMotor.moveTo(79200); 
  horizontalMotor.setMaxSpeed(100.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizontalMotor.setAcceleration(50.0);   // try other acceleration rates.
  horizontalMotor.moveTo(-5000); 

  // getOffset(Polaris);
}

void loop() {
  int currentButtonValue = analogRead(0);
  int newAction = -1;
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

  calculateTime();
  calculateLocalSiderealTime();
  updateCoords(Vega);

  float verticalSpeed;
  float horizontalSpeed;
  if (printTime >= 1000) {    // reports speed and position each second
    reportStatus();
    reportLcd(Vega);
  }
  verticalMotor.run();
  horizontalMotor.run();
}

void updateCoords(target &t) {
  setDecimalCoords(t);
  setAltazimuthCoords(t);
}

void setDecimalCoords(target &t) {
  t.DEC_decimal = t.DEC_degrees + (t.DEC_arcmin/60.0) + (t.DEC_arcsec/3600.0);
  t.RA_decimal =  15 * (t.RA_hour + (t.RA_min/60.0) + (t.RA_sec/3600.0));
}

void buttonPressed(int buttonNumber) {
  Serial.print("Button action");
  Serial.print(buttonNumber);
}

float stepsPerDegree = 1000;

