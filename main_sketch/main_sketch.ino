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
elapsedMillis lcdTime;

#define MODE_MENU 0 
#define MODE_REPORT 1
#define MODE_MOVING 2
#define MODE_CALIBRATING 3
#define MODE_FIND 4
#define MODE_AZ_ALT 5

// Reduction X Axis = 1/ 198
// Reduction Z Axis = 1/ 33

typedef struct {
  String name;
  double ra;
  double dec;
} target;

double hourMinArcSecToDouble(float hour, float minute, float second) {
  if (hour < 0) {
    return hour - (minute / 60.0) - (second / 3600.0);
  } else {
    return hour + (minute / 60.0) + (second / 3600.0);
  }
}

target sirius = {"Sirius", hourMinArcSecToDouble(6, 46, 13), hourMinArcSecToDouble(-16, 45, 7.3)};
target canopus = {"Canopus", hourMinArcSecToDouble(6, 24, 29.6), hourMinArcSecToDouble(-52, 42, 45.3)};
target alphaCentauri = {"Alpha Centauri", hourMinArcSecToDouble(14, 41, 16.7), hourMinArcSecToDouble(-60, 55, 59.1)};

float gpsLatitude = -22.6599734;
float gpsLongitude = -46.9420532;
//unsigned long age;
//double timenow;_

// Current ra and current dec
double ra = sirius.ra;
double dec = sirius.dec;
double julianDate;
double timeOfDay;
double gstTime;
double lst;
double ha;
double azm;

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
int potHorizontal;
int potVertical;

int activeMode = MODE_MENU;

#define MAX_VERTICAL_SPEED 2000
#define MAX_HORIZONTAL_SPEED 100

void setup() {
  lcd.begin(16,2); //SETA A QUANTIDADE DE COLUNAS(16) E O NÚMERO DE LINHAS(2) DO DISPLAY_. EM SUMA: UMA MATRIZ DE 16 COLUNAS E 2 LINHAS
  lcd.setCursor(0,0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1) 
  lcd.print("TELESCOPE"); 

  Serial.begin(115200);

  parseReceivedTimeString("2024-03-16 23:40:50.123");

  // Limits for the motors
  verticalMotor.setMaxSpeed(MAX_VERTICAL_SPEED);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  verticalMotor.setAcceleration(100.0);   // try other acceleration rates.
  horizontalMotor.setMaxSpeed(MAX_HORIZONTAL_SPEED);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizontalMotor.setAcceleration(50.0);   // try other acceleration rates.

  // getOffset(Polaris);
}

void loop() {
  calculateTime();

  //
  // Standard loop cals
  julianDate = julianDateCalc();
  gstTime = utcToGstCalc();
  lst = gstToLstCalc();
  azimuthAltitudeCalculation();

  potHorizontal = analogRead(1);
  potVertical = analogRead(2);

  registerButton();
  //calculateLocalSiderealTime();
  //updateCoords(defaultTarget);

  if (printTime >= 1000) {    // reports speed and position each second
    printTime = 0;
    reportStatus();
  }
  if (lcdTime >= 250) {    // reports speed and position each second
    lcdTime = 0;
    reportLcd();
  }
  
  moveMotors();
  
  verticalMotor.run();
  horizontalMotor.run();
}

int horizontalSpeed = 0;
int verticalSpeed = 0;
float verticalMotorSpeed = 0.0;
float horizontalMotorSpeed = 0.0;

void moveMotors() {
  if(activeMode == MODE_MOVING) {
    horizontalSpeed = translatePotValueToSpeed(potHorizontal);
    verticalSpeed = translatePotValueToSpeed(potVertical);
    horizontalMotorSpeed = map(horizontalSpeed, -120, +120, -1 * MAX_HORIZONTAL_SPEED, MAX_HORIZONTAL_SPEED);
    verticalMotorSpeed = map(verticalSpeed, -120, +120, -1 * MAX_VERTICAL_SPEED, MAX_VERTICAL_SPEED);
    
    // Crawl speed
    if (horizontalSpeed == -1) {
      horizontalMotorSpeed = -1.0; 
    } else if(horizontalSpeed == 1) {
      horizontalMotorSpeed = 1.0;
    }
    if (verticalSpeed == -1) {
      verticalMotorSpeed = -1.0; 
    } else if(verticalSpeed == 1) {
      verticalMotorSpeed = 1.0;
    }
    
    horizontalMotor.setSpeed(horizontalMotorSpeed);
    verticalMotor.setSpeed(verticalMotorSpeed);

    horizontalMotor.runSpeed();
    verticalMotor.runSpeed();
  } else {
    horizontalSpeed = 0;
    verticalSpeed = 0;
    horizontalMotorSpeed = 0;
    verticalMotorSpeed = 0;
  }
}

float stepsPerDegree = 1000;

