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
#include <elapsedMillis.h>
#include <LiquidCrystal.h> // Inclui biblioteca "LiquidCristal.h"
#include "FastAccelStepper.h"
#include <Wire.h>
#include <RTClib.h>

#include <SPI.h>

#define MAX_VERTICAL_SPEED 15000
#define MAX_HORIZONTAL_SPEED 500

// region Engine declarations
//  
LiquidCrystal lcd(11, 12, 4, 5, 9, 10); 
RTC_DS3231 rtc;
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *verticalMotor = NULL;
FastAccelStepper *horizontalMotor = NULL;
//
// endregion

elapsedMillis printTime;
elapsedMillis lcdTime;
elapsedMillis calcTime;
elapsedMillis ledTime;
elapsedMillis buttonTime;
elapsedMillis moveMotorsTime;

// region Menu modes
//
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
#define MODE_CALIBRATE_STAR_COMPLETE 11
#define MODE_CALIBRATION_COMPLETE 12
//
// endregion

// region Menu variables
//
#define RIGHT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define SELECT 5

int buttonAction1 = -2;
int buttonAction2 = -2;
int actionIndex = 0;
int lastButtonAction = -1;

int selectedChoice = 0;
int maxChoice = 0;

int activeMode = MODE_MENU;

int potHorizontal;
int potVertical;
int ledPower = 0;
int ledIncrement = 3;
//
// endregion

// region Target/ calibrating data declaration
//
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
target rigel = {"Rigel", hourMinArcSecToDouble(5, 14, 32.27210), hourMinArcSecToDouble(-8, 12, 5.8981)};
// target arcturus = {"Arcturus", hourMinArcSecToDouble(14,15, 39.7), hourMinArcSecToDouble(19, 10, 57)};

target targets[] = { sirius, canopus, alphaCentauri, mimosa, acrux, rigel };
target* calibratingTarget;

int calibratingStarIndex = 0;
calibrationPoint calibrationPoint0;
calibrationPoint calibrationPoint1;
calibrationPoint calibrationPoint2;
int calibrated = 0;
//
// endregion

// region Hardcoded GPS lat/ lon
// 
float gpsLatitude = -22.6599734;
float gpsLongitude = -46.9420532;
//
// endregion

// region Where we are pointing at, where we are
//
double ra = 10.0;
double dec = -20.0;
double julianDate;
double timeOfDay;
double gstTime;
double lst;
double ha;
double azm;

int horizontalSpeed = 0;
int verticalSpeed = 0;
float verticalMotorSpeed = 0.0;
float horizontalMotorSpeed = 0.0;
float horizontalCoordinateSpeed = 0;
float verticalCoordinateSpeed = 0;

int loopsPerSec = 0;
int rtcInitialized = 0;
//
// endregion

// region Date/ time variables
//
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
//
// endregion

// region Calibration info
// 
long ha1;
long ha2;
long haMotor1;
long haMotor2;
long azm1;
long azm2;
long azmMotor1;
long azmMotor2;
//
// endregion

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("INITIALIZING...."); 
  lcd.setCursor(0, 1);
  lcd.print("    ...TELESCOPE"); 
  
  Serial.begin(115200);

  // Wire.begin();
  rtcInitialized = rtc.begin();
  if (rtcInitialized) {
    if (rtc.lostPower()) {
      // This time needs to come from the serial
      rtc.adjust(DateTime(2024, 3, 19, 20, 31, 0));
    }    
  }

  // rtc.setMinute(12);
  // rtc.setHour(20);
  // rtc.setDate(19);
  // rtc.setMonth(3);
  // rtc.setYear(2024);


  parseReceivedTimeString("2024-03-16 23:40:50.123");

  engine.init();
  verticalMotor = engine.stepperConnectToPin(6);
  if (verticalMotor) {
    verticalMotor->setDirectionPin(22);
    verticalMotor->setAcceleration(1500);
    verticalMotor->setAutoEnable(true);
  }
  horizontalMotor = engine.stepperConnectToPin(7);
  if (horizontalMotor) {
    horizontalMotor->setDirectionPin(24);
    horizontalMotor->setAcceleration(100);
    horizontalMotor->setAutoEnable(true);
  }
  
  pinMode(13, OUTPUT);
}

void loop() {
  //
  // Error situation
  if(verticalMotor == NULL || horizontalMotor == NULL) {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("                ");

    if (verticalMotor == NULL) {
      lcd.setCursor(0, 0);
      lcd.print("VERT");
    }
    if (horizontalMotor == NULL) {
      lcd.setCursor(8, 0);
      lcd.print("HORZ");
    }
    lcd.setCursor(0, 1);
    lcd.print("STEPPER FAILURE");

    delay(500);
    return;
  }
  if (!rtcInitialized) {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("RTC FAILURE     ");

    delay(500);
  }
  
  if (calcTime > 25) {
    // Time calculation
    calculateTime();
    
    //
    // Standard loop cals
    julianDate = julianDateCalc();
    gstTime = utcToGstCalc();
    lst = gstToLstCalc();
    azimuthAltitudeCalculation();

    potHorizontal = analogRead(4);
    potVertical = analogRead(5);
    
    moveMotors();

    calcTime = 0;
  }
  
  if (moveMotors > 100) {
    moveMotors();

    moveMotorsTime = 0;
  }

  if (buttonTime > 100) {
    registerButton();

    buttonTime = 0;
  }
  //calculateLocalSiderealTime();
  //updateCoords(defaultTarget);

  loopsPerSec++;
  if (printTime >= 1000) {    // reports speed and position each second
    reportStatus();
    loopsPerSec = 0;

    printTime = 0;
  }
  if (lcdTime >= 250) {    // reports speed and position each second
    reportLcd();

    lcdTime = 0;
  }

  if (ledTime > 10) {
    ledPower += ledIncrement;
    if (ledIncrement > 0 && ledPower > 255) {
      ledIncrement *= -1;
    } else if(ledIncrement < 0 && ledPower < 0) {
      ledIncrement *= -1;
    }
    analogWrite(13, ledPower);

    ledTime = 0;
  }
}

void storeCalibrateCoordinates() {
  calibrationPoint* usePoint;
  if (calibratingStarIndex == 0) {
    usePoint = &calibrationPoint0;
  } else if (calibratingStarIndex == 1) {
    usePoint = &calibrationPoint1;
  } else {
    usePoint = &calibrationPoint2;
  }
  
  usePoint->ra = calibratingTarget->ra;
  usePoint->dec = calibratingTarget->dec;
  usePoint->lst = lst;
  usePoint->ha = ha;
  usePoint->azm = azm;
  usePoint->horizontalPosition = horizontalMotor->getCurrentPosition(); 
  usePoint->verticalPosition = verticalMotor->getCurrentPosition(); 
}

void storeCalibrationData() {
  calibrated = 1;
  ledIncrement = 1;

  ha1 = calibrationPoint0.ha * 10000.0;
  ha2 = calibrationPoint1.ha * 10000.0;
  haMotor1 = calibrationPoint0.verticalPosition;
  haMotor2 = calibrationPoint1.verticalPosition;
  azm1 = calibrationPoint0.azm * 10000.0;
  azm2 = calibrationPoint1.azm * 10000.0;
  azmMotor1 = calibrationPoint0.horizontalPosition;
  azmMotor2 = calibrationPoint1.horizontalPosition;

  // So that it doesn't move when the last point moves
  ra = calibratingTarget->ra;
  dec = calibratingTarget->dec;
}

void moveMotors() {
  if (calibrated) {
    if(activeMode == MODE_MOVE_COORDINATES) {
      int horizontalPotValue = translatePotValueToSpeed(potHorizontal);
      int verticalPotValue = translatePotValueToSpeed(potVertical);
      horizontalCoordinateSpeed = map(horizontalSpeed, -120, +120, -1000, +1000) / 100000.0;
      verticalCoordinateSpeed = map(verticalSpeed, -120, +120, -1000, +1000) / 100000.0;
      ra += horizontalCoordinateSpeed;
      dec += verticalCoordinateSpeed;
    } else if (activeMode == MODE_MOVE_MENU) {
      // tracking
    }
    
    long newVerticalPos = map(ha * 1000, ha1, ha2, haMotor1, haMotor2);
    long newHorizontalPos = map(ha * 1000, azm1, azm2, azmMotor1, azmMotor2);
    
    verticalMotor->moveTo(newVerticalPos);
    horizontalMotor->moveTo(newHorizontalPos);
  } else {
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

    int horizontalMultiplier = 1;
    int verticalMultiplier = 1;
    if (horizontalMotorSpeed < 0) {
      horizontalMultiplier = -1; 
    }
    if (verticalMotorSpeed < 0) {
      verticalMultiplier = -1; 
    }

    if(horizontalMotorSpeed == 0 || horizontalSpeed == 0) {
      horizontalMotor->stopMove();
    } else {
      horizontalMotor->setSpeedInHz(abs(horizontalMotorSpeed));
      horizontalMotor->applySpeedAcceleration();
      if (horizontalMultiplier > 0) {
        horizontalMotor->runForward();
      } else {
        horizontalMotor->runBackward();
      }
    }

    if(verticalMotorSpeed == 0 || verticalSpeed == 0) {
      verticalMotor->stopMove();
    } else {
      verticalMotor->setSpeedInHz(abs(verticalMotorSpeed));
      verticalMotor->applySpeedAcceleration();
      if (verticalMultiplier > 0) {
        verticalMotor->runForward();
      } else {
        verticalMotor->runBackward();
      }
    }
  }
//   horizontalMotor->move(horizontalMotorSpeed * horizontalMultiplier);
//   verticalMotor->move(verticalMotorSpeed * verticalMultiplier);
}
