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

#include <SPI.h>
  
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

AccelStepper verticalMotor(AccelStepper::DRIVER, 10, 11); 
AccelStepper horizontalMotor(AccelStepper::DRIVER, 12, 13); 

elapsedMillis printTime;
elapsedMillis lcdTime;
elapsedMillis calcTime;

#define MODE_MENU 0 
#define MODE_REPORT 1
#define MODE_MOVE_MENU 2
#define MODE_CALIBRATING 3
#define MODE_FIND 4
#define MODE_AZ_ALT 5
#define MODE_MOVE_COORDINATES 7
#define MODE_MOVE_MOTOR 8
#define MODE_CALIBRATE_PICKED_STAR 9
#define MODE_CALIBRATE_MOVING 10
#define MODE_CALIBRATE_SELECTED 11
#define MODE_CALIBRATION_COMPLETE 12

const int LEDMATRIX_SEGMENTS = 1;
const int SEGMENT_WIDTH = 8;
const int LEDMATRIX_WIDTH = LEDMATRIX_SEGMENTS * SEGMENT_WIDTH;

const uint8_t LEDMATRIX_CS_PIN = 31;

// Reduction X Axis = 1/ 198
// Reduction Z Axis = 1/ 33

typedef struct {
  String name;
  double ra;
  double dec;
} target;

typedef struct {
  double ra;
  double dec;
  double lst;
  double ha;
  double azm;
  long horizontalPosition;
  long verticalPosition;
} calibrationPoint;

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
target mimosa = {"Mimosa", hourMinArcSecToDouble(12, 47, 44), hourMinArcSecToDouble(-59, 41, 19)};
target acrux = {"Acrux", hourMinArcSecToDouble(12, 26, 35.89), hourMinArcSecToDouble(-63, 5, 56.7343)};
// target arcturus = {"Arcturus", hourMinArcSecToDouble(14,15, 39.7), hourMinArcSecToDouble(19, 10, 57)};

target calibratingTarget;

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

int calibratingStarIndex = 0;
calibrationPoint calibrationPoint0;
calibrationPoint calibrationPoint1;
calibrationPoint calibrationPoint2;

int horizontalSpeed = 0;
int verticalSpeed = 0;
float verticalMotorSpeed = 0.0;
float horizontalMotorSpeed = 0.0;

int loopsPerSec = 0;

#define MAX_VERTICAL_SPEED 500
#define MAX_HORIZONTAL_SPEED 100

int calibrated = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("INITIALIZING...."); 
  lcd.setCursor(0, 1);
  lcd.print("    ...TELESCOPE"); 

  Serial.begin(115200);

  parseReceivedTimeString("2024-03-16 23:40:50.123");

  // Limits for the motors
  verticalMotor.setMaxSpeed(MAX_VERTICAL_SPEED);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  verticalMotor.setAcceleration(50.0);   // try other acceleration rates.
  horizontalMotor.setMaxSpeed(MAX_HORIZONTAL_SPEED);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizontalMotor.setAcceleration(50.0);   // try other acceleration rates.

  // getOffset(Polaris);
  horizontalMotor.moveTo(0);
  verticalMotor.moveTo(0);
}

void loop() {
  calculateTime();

  if (calcTime > 100) {
    //
    // Standard loop cals
    julianDate = julianDateCalc();
    gstTime = utcToGstCalc();
    lst = gstToLstCalc();
    azimuthAltitudeCalculation();

    potHorizontal = analogRead(1);
    potVertical = analogRead(2);
    
    calcTime = 0;
  
     moveMotors();
  }

  registerButton();
  //calculateLocalSiderealTime();
  //updateCoords(defaultTarget);

  loopsPerSec++;
  if (printTime >= 1000) {    // reports speed and position each second
    printTime = 0;
    reportStatus();
    loopsPerSec = 0;
  }
  if (lcdTime >= 250) {    // reports speed and position each second
    lcdTime = 0;
    reportLcd();
  }

//  verticalMotor.run();
  
  if (calibrated) {
    horizontalMotor.run();
    verticalMotor.run();
  } else {
    horizontalMotor.runSpeed();
    verticalMotor.runSpeed();
  }
}

void storeCalibrateCoordinates() {
  calibrationPoint usePoint;
  if (calibratingStarIndex == 0) {
    usePoint = calibrationPoint0;
  } else if (calibratingStarIndex == 1) {
    usePoint = calibrationPoint1;
  } else {
    usePoint = calibrationPoint2;
  }  
  
  usePoint.ra = ra;
  usePoint.dec = dec;
  usePoint.lst = lst;
  usePoint.ha = ha;
  usePoint.azm = azm;
  usePoint.horizontalPosition = horizontalMotor.currentPosition(); 
  usePoint.verticalPosition = verticalMotor.currentPosition(); 
}

void storeCalibrationData() {
  calibrated = 1;
}

void moveMotors() {
  if(activeMode == MODE_MOVE_MOTOR || activeMode == MODE_CALIBRATE_MOVING) {
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
  } else {
    horizontalSpeed = 0;
    verticalSpeed = 0;
    horizontalMotorSpeed = 0;
    verticalMotorSpeed = 0;
  }

  horizontalMotor.setSpeed(horizontalMotorSpeed);
  verticalMotor.setSpeed(verticalMotorSpeed);
}

float stepsPerDegree = 1000;

