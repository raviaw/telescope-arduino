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
#include <Encoder.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

#define MAX_VERTICAL_SPEED 15000
#define MAX_HORIZONTAL_SPEED 500

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_TOP_AREA_PIXELS 16
#define SCREEN_BOTTOM_AREA_PIXELS 48

// region Engine declarations
//  
LiquidCrystal lcd(11, 12, 4, 5, 9, 10); 
RTC_DS3231 rtc;
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *verticalMotor = NULL;
FastAccelStepper *horizontalMotor = NULL;
Encoder knob(2, 3);
Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define VERTICAL_STEPPER_STEP_PIN 6
#define VERTICAL_STEPPER_DIR_PIN 22
#define HORIZONTAL_STEPPER_STEP_PIN 7
#define HORIZONTAL_STEPPER_DIR_PIN 24

#define HORIZONTAL_JOYSTICK_COARSE 3
#define HORIZONTAL_JOYSTICK_FINE 2
#define VERTICAL_JOYSTICK_COARSE 0
#define VERTICAL_JOYSTICK_FINE 6
#define LCD_INPUT_BUTTON 1
#define HORIZONTAL_POT 4
#define VERTICAL_POT 5
#define REFERENCE_INPUT_BUTTON 7
#define ACTION_INPUT_BUTTON 52
#define ENCODER_INPUT_BUTTON 48
#define BACK_INPUT_BUTTON 46
#define COARSE_JOYSTICK_BUTTON 32
#define ENABLE_POT_BUTTON 34
#define FINE_JOYSTICK_BUTTON 30

#define POT 1
#define REFERENCE_INPUT_BUTTON 7

#define HORIZONTAL_LED 26
#define VERTICAL_LED 28

//
// endregion

elapsedMillis printTime;
elapsedMillis lcdTime;
elapsedMillis calcTime;
elapsedMillis ledTime;
elapsedMillis buttonTime;
elapsedMillis moveMotorsTime;
elapsedMillis logMotorsTime;
elapsedMillis oledRefreshTime;
elapsedMillis androidRefreshTime;
elapsedMillis horizontalBlinkTimer;
elapsedMillis verticalBlinkTimer;

int horizontalBlinkOnTime = 10;
int horizontalBlinkOffTime = 1000;
int horizontalBlinkState = 0; // 0 OFF, 1 ON
int verticalBlinkOnTime = 10;
int verticalBlinkOffTime = 1000;
int verticalBlinkState = 0; // 0 OFF, 1 ON

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

int selectButtonAction1 = -2;
int selectButtonAction2 = -2;
int selectActionIndex = 0;
int lastSelectButtonAction = -1;

int selectedChoice = 0;
int maxChoice = 0;

int newHorizontalValue;
int newVerticalValue;

int activeMode = MODE_MENU;

//int lcdStarIndex = 0;
int lcdSelectedStar = -1;

int potHorizontal = 0;
int potVertical = 0;
int potHorizontalJoystickCoarse = 0;
int potVerticalJoystickCoarse = 0;
int potHorizontalJoystickFine = 0;
int potVerticalJoystickFine = 0;
int ledPower = 0;
int ledIncrement = 3;

long lastKnobValue = 0;

int lastJoystickNavigationValue = 0;
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
  double alt;
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

double hourMinArcSecToDoubleRa(float hour, float minute, float second) {
  double value = hourMinArcSecToDouble(hour, minute, second);
  return mapDouble(value, 0, 24.0, 0, 360);
}

target sirius = {"Sirius", hourMinArcSecToDouble(6, 46, 13), hourMinArcSecToDouble(-16, 45, 7.3)};
target canopus = {"Canopus", hourMinArcSecToDouble(6, 24, 29.6), hourMinArcSecToDouble(-52, 42, 45.3)};
target alphaCentauri = {"Alpha Centauri", hourMinArcSecToDouble(14, 41, 16.7), hourMinArcSecToDouble(-60, 55, 59.1)};
target mimosa = {"Mimosa", hourMinArcSecToDouble(12, 47, 44), hourMinArcSecToDouble(-59, 41, 19)};
target acrux = {"Acrux", hourMinArcSecToDouble(12, 26, 35.89), hourMinArcSecToDouble(-63, 5, 56.7343)};
target rigel = {"Rigel", hourMinArcSecToDouble(5, 14, 32.27210), hourMinArcSecToDouble(-8, 12, 5.8981)};
target hadar = {"Hadar", hourMinArcSecToDouble(14, 3, 49.40535), hourMinArcSecToDouble(-60, 22, 22.9266)};
target altair = {"Altair", hourMinArcSecToDouble(19, 50, 46.99855), hourMinArcSecToDouble(8, 52, 5.9563)};
target polaris = {"Polaris", hourMinArcSecToDouble(2, 31, 49.09), hourMinArcSecToDouble(89, 15, 50.8)};
target sigmaOctantis = {"Sigma Octantis", hourMinArcSecToDouble(21, 8, 46.86357), hourMinArcSecToDouble(-88, 57, 23.3983)};
// target arcturus = {"Arcturus", hourMinArcSecToDouble(14,15, 39.7), hourMinArcSecToDouble(19, 10, 57)};

target targets[] = { sirius, canopus, alphaCentauri, mimosa, acrux, rigel, hadar, altair, polaris, sigmaOctantis };
target* calibratingTarget;

int calibratingStarIndex = 0;
calibrationPoint calibrationPoint0;
calibrationPoint calibrationPoint1;
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
double alt;
double azm;

double currentMotorAlt;
double currentMotorAzm;

double lastStarRa = -999;
double lastStarDec = -999;

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
double alt1;
double alt2;
double altMotor1;
double altMotor2;
double azm1;
double azm2;
double azmMotor1;
double azmMotor2;
long newVerticalPos;
long newHorizontalPos;
//
// endregion

double calcLst;
double calcRa;
double sinAlt;
double cosA;
double a;
double sinHa;

char serialBuffer[128];
int serialBufferPointer = 0;

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

  oledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oledDisplay.display();

  engine.init();
  // engine.setDebugLed(26);
  verticalMotor = engine.stepperConnectToPin(VERTICAL_STEPPER_STEP_PIN);
  if (verticalMotor) {
    verticalMotor->setDirectionPin(VERTICAL_STEPPER_DIR_PIN);
    verticalMotor->setAcceleration(1500);
    verticalMotor->setAutoEnable(true);
  }
  horizontalMotor = engine.stepperConnectToPin(HORIZONTAL_STEPPER_STEP_PIN);
  if (horizontalMotor) {
    horizontalMotor->setDirectionPin(HORIZONTAL_STEPPER_DIR_PIN);
    horizontalMotor->setAcceleration(100);
    horizontalMotor->setAutoEnable(true);
  }

  // Bluetooth  
  Serial1.begin(9600);
  
  pinMode(ACTION_INPUT_BUTTON, INPUT);
  pinMode(ENCODER_INPUT_BUTTON, INPUT_PULLUP);
  pinMode(COARSE_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(FINE_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(ENABLE_POT_BUTTON, INPUT_PULLUP);
  pinMode(HORIZONTAL_LED, OUTPUT);
  pinMode(VERTICAL_LED, OUTPUT);

  analogWrite(8, 240);
}

void loop() {
  ///////////////////////////////////////////////////////////////////////////
  // Error situation
  //
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
  //
  ///////////////////////////////////////////////////////////////////////////
  
  if(Serial1.available())
  { 
    char nextChar = Serial1.read();
    if (nextChar != -1) { 
      if (nextChar == '\n') {
        if (serialBufferPointer > 0) {
          serialBuffer[serialBufferPointer] = 0;
          Serial.print("Received serial coordinates: ");
          Serial.write(serialBuffer);
          Serial.println();
        
          processInput(serialBuffer);
        }
        serialBufferPointer = 0;
      } else if(serialBufferPointer < sizeof(serialBuffer) -1) {
        serialBuffer[serialBufferPointer] = nextChar;
        serialBufferPointer++;
      }    
    }
  }
    
  loopsPerSec++;

  registerButton();

  if (calcTime > 25) {
    if(digitalRead(ENABLE_POT_BUTTON) == 1) {
      potHorizontal = analogRead(HORIZONTAL_POT);
      potVertical = analogRead(VERTICAL_POT);
    } else {
      potHorizontal = 512;
      potVertical = 512;
    }
    potHorizontalJoystickCoarse = analogRead(HORIZONTAL_JOYSTICK_COARSE);
    potVerticalJoystickCoarse = analogRead(VERTICAL_JOYSTICK_COARSE);
    potHorizontalJoystickFine = analogRead(HORIZONTAL_JOYSTICK_FINE);
    potVerticalJoystickFine = analogRead(VERTICAL_JOYSTICK_FINE);

    //
    calculateEverything();
   
    calcTime = 0;
  }
  
  if (moveMotors > 100) {
    moveMotors();
    determineLedIntervals();

    moveMotorsTime = 0;
  }

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
  
  if (androidRefreshTime > 1000) {
    reportBluetooth();
    androidRefreshTime = 0;
  }
  
  if (oledRefreshTime > 1000) {
    refreshOled();
    
    oledRefreshTime = 0;
  }

  if (logMotorsTime > 100) {
    float maxHorizontalPercentage = mapDouble(abs(horizontalMotor->getCurrentSpeedInUs()), 0, horizontalMotor->getMaxSpeedInUs(), 0, 100.0);
    float maxVerticalPercentage = mapDouble(abs(verticalMotor->getCurrentSpeedInUs()), 0, verticalMotor->getMaxSpeedInUs(), 0, 100.0);

    logMotorsTime = 0;
  }
  
  if (horizontalBlinkState == 0 && horizontalBlinkTimer > horizontalBlinkOffTime) {
    horizontalBlinkTimer = 0;
    horizontalBlinkState = 1;
    digitalWrite(HORIZONTAL_LED, HIGH);
  } else if (horizontalBlinkState == 1 && horizontalBlinkTimer > horizontalBlinkOnTime) { 
    horizontalBlinkTimer = 0;
    horizontalBlinkState = 0;
    digitalWrite(HORIZONTAL_LED, LOW);
  }

  if (verticalBlinkState == 0 && verticalBlinkTimer > verticalBlinkOffTime) {
    verticalBlinkTimer = 0;
    verticalBlinkState = 1;
    digitalWrite(VERTICAL_LED, HIGH);
  } else if (verticalBlinkState == 1 && verticalBlinkTimer > verticalBlinkOnTime) { 
    verticalBlinkTimer = 0;
    verticalBlinkState = 0;
    digitalWrite(VERTICAL_LED, LOW);
  }
}

void calculateEverything() {
  //
  calculateTime();
  
  //
  // Standard loop cals
  julianDate = julianDateCalc();
  gstTime = utcToGstCalc();
  lst = gstToLstCalc();
  azimuthAltitudeCalculation();
}
 
void storeCalibrateCoordinates() {
  calibrationPoint* usePoint;
  if (calibratingStarIndex == 0) {
    usePoint = &calibrationPoint0;
  } else if (calibratingStarIndex == 1) {
    usePoint = &calibrationPoint1;
  } 
  
  usePoint->ra = calibratingTarget->ra;
  usePoint->dec = calibratingTarget->dec;
  usePoint->lst = lst;
  usePoint->alt = alt;
  usePoint->azm = azm;
  usePoint->horizontalPosition = horizontalMotor->getCurrentPosition(); 
  usePoint->verticalPosition = verticalMotor->getCurrentPosition(); 
}

void prepareStarCoordinates() {
  ra = calibratingTarget->ra;
  dec = calibratingTarget->dec;
  calculateEverything();
}

void prepareToMoveWithCalibration() {
    horizontalMotor->setAcceleration(100);
    horizontalMotor->setAutoEnable(true);
    verticalMotor->setAcceleration(1500);
    verticalMotor->setAutoEnable(true);
    horizontalMotor->setSpeedInHz(MAX_HORIZONTAL_SPEED);
    verticalMotor->setSpeedInHz(MAX_VERTICAL_SPEED);
    horizontalMotor->applySpeedAcceleration();
    verticalMotor->applySpeedAcceleration();
}

void storeCalibrationData() {
  calibrated = 1;
  ledIncrement = 1;

  alt1 = calibrationPoint0.alt;
  alt2 = calibrationPoint1.alt;
  altMotor1 = calibrationPoint0.verticalPosition;
  altMotor2 = calibrationPoint1.verticalPosition;
  azm1 = calibrationPoint0.azm;
  azm2 = calibrationPoint1.azm;
  azmMotor1 = calibrationPoint0.horizontalPosition;
  azmMotor2 = calibrationPoint1.horizontalPosition;

  // So that it doesn't move when the last point moves
  ra = calibrationPoint1.ra;
  dec = calibrationPoint1.dec;

  calculateEverything();
}

void moveMotorsTracking() {
  newVerticalPos = mapDouble(alt, alt1, alt2, altMotor1, altMotor2);
  newHorizontalPos = mapDouble(azm, azm1, azm2, azmMotor1, azmMotor2);
  verticalMotor->moveTo(newVerticalPos);
  horizontalMotor->moveTo(newHorizontalPos);
}

int readHorizontalPots() {
  int coarse = translatePotValueToSpeed(potHorizontalJoystickCoarse, -1);
  if (coarse != 0) {
    return coarse; 
  }
  int fine = translatePotValueToSpeed(potHorizontalJoystickFine, -1) / 20.0;
  if (fine != 0) {
    return fine; 
  }
  return translatePotValueToSpeed(potHorizontal, -1);
}

int readVerticalPots() {
  int coarse = translatePotValueToSpeed(potVerticalJoystickCoarse, -1);
  if (coarse != 0) {
    return coarse; 
  }
  int fine = translatePotValueToSpeed(potVerticalJoystickFine, -1) / 20.0;
  if (fine != 0) {
    return fine; 
  }
  return translatePotValueToSpeed(potVertical, -1);
}

void moveMotors() {
  if (calibrated) {
    if(activeMode == MODE_MOVE_COORDINATES) {
      horizontalSpeed = readHorizontalPots();
      verticalSpeed = readVerticalPots();
      horizontalCoordinateSpeed = mapDouble(horizontalSpeed, -120, +120, -1, +1) / 500.0;
      verticalCoordinateSpeed = mapDouble(verticalSpeed, -120, +120, -1, +1) / 500.0;
      long horizontalMotorDiff = abs(horizontalMotor->getCurrentPosition() - newHorizontalPos);
      if (horizontalMotorDiff < MAX_HORIZONTAL_SPEED / 2) {
        ra += horizontalCoordinateSpeed;
      }
      long verticalMotorDiff = abs(verticalMotor->getCurrentPosition() - newVerticalPos);
      if (verticalMotorDiff < MAX_VERTICAL_SPEED / 2) {
        dec += verticalCoordinateSpeed;
      }
    }
    
    currentMotorAlt = mapDouble(verticalMotor->getCurrentPosition(), altMotor1, altMotor2, alt1, alt2);
    currentMotorAzm = mapDouble(horizontalMotor->getCurrentPosition(), azmMotor1, azmMotor2, azm1, azm2);

    moveMotorsTracking();
  } else {
    currentMotorAlt = alt;
    currentMotorAzm = azm;

    if(activeMode == MODE_MOVE_MOTOR || activeMode == MODE_CALIBRATE_MOVING) {
      horizontalSpeed = readHorizontalPots();
      verticalSpeed = readVerticalPots();
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

void determineLedIntervals() {
  double azmDifference = abs(azm - currentMotorAzm);
  double altDifference = abs(alt - currentMotorAlt);
  horizontalBlinkOnTime = azmDifference * 100;  
  horizontalBlinkOffTime = 1000 - horizontalBlinkOnTime; 
  verticalBlinkOnTime = altDifference * 100;  
  verticalBlinkOffTime = 1000 - verticalBlinkOnTime; 
}