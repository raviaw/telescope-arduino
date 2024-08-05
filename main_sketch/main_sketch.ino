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
/*
00 
01
10
11

00000000 - 256
Each Arduino Uno has 14 digital pins - I need 2 of these for the encoder
I will use the other 12 digital pins for the communication - it will 2 bits for the "clock", and one bit for the "delta" - 0 to 3 for clock, 0 to 1023 for the delta
You read the encoder input and store it in a number
You keep an internal clock that you progress every 0.5 seconds on the source arduino
Each time you progress the clock, you calculate the delta between the current value from the encoder and last value, and that is your delta. You put your delta on the 10 digital pins.
When reading it, you read the clock - every time the clock changes, you apply the delta to your local number and you have the same value as the source arduino, provided you can read the data frequently enough
I was thinking how to make them communicate without losing data and this is one way - it should work
Clock and delta
12 PINS
00
01
10
11
00000000000 0 to 1024
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
#include <Ephemeris.h>

#define MAX_VERTICAL_SPEED 15000
#define MAX_VERTICAL_ACCELERATION 1000
#define MAX_HORIZONTAL_SPEED 1500
#define MAX_HORIZONTAL_ACCELERATION 100

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

#define HORIZONTAL_JOYSTICK_LEFT 3
#define HORIZONTAL_JOYSTICK_RIGHT 2
#define VERTICAL_JOYSTICK_LEFT 0
#define VERTICAL_JOYSTICK_RIGHT 6
#define LCD_INPUT_BUTTON 1
#define HORIZONTAL_POT 4
#define VERTICAL_POT 5
#define REFERENCE_INPUT_BUTTON 7
#define ACTION_INPUT_BUTTON 52
#define ENCODER_INPUT_BUTTON 48
#define ENCODER_INPUT_BUTTON 25
#define RIGHT_JOYSTICK_BUTTON 21
#define LEFT_JOYSTICK_BUTTON 23
#define ENABLE_POT_BUTTON 34

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
elapsedMillis monitorEncoder1Timer;

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
#define FLIP_LEFT 6
#define FLIP_RIGHT 7

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
// horizontal right and vertical left are ignored
int potHorizontalJoystickLeft = 0;
int potVerticalJoystickRight = 0;
int ledPower = 0;
int ledIncrement = 3;
int leftJoystickDirection = -1;
int rightJoystickDirection = -1;
int MAX_LEFT_JOYSTICK_SPEED = 10;
int leftJoystickSpeed = 0;
int MAX_RIGHT_JOYSTICK_SPEED = 10;
int rightJoystickSpeed = 0;

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
  int special;
} target;

typedef struct {
  double ra;
  double dec;
  long horizontalPosition;
  long verticalPosition;
  int currentYear;
  int currentMonth;
  int currentDay;
  int currentHour;
  int currentMinute;
  int currentSecond;
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

target sirius = {"Sirius", Ephemeris::hoursMinutesSecondsToFloatingHours(6, 46, 13), Ephemeris::hoursMinutesSecondsToFloatingHours(-16, -45, -7.3), -1};
target canopus = {"Canopus", Ephemeris::hoursMinutesSecondsToFloatingHours(6, 24, 29.6), Ephemeris::hoursMinutesSecondsToFloatingHours(-52, -42, -45.3), -1};
target alphaCentauri = {"Alpha Centauri", Ephemeris::hoursMinutesSecondsToFloatingHours(14, 41, 16.7), Ephemeris::hoursMinutesSecondsToFloatingHours(-60, -55, -59.1), -1};
target becrux = {"Becrux", Ephemeris::hoursMinutesSecondsToFloatingHours(12, 47, 44), Ephemeris::hoursMinutesSecondsToFloatingHours(-59, -41, -19), -1};
target acrux = {"Acrux", Ephemeris::hoursMinutesSecondsToFloatingHours(12, 26, 35.89), Ephemeris::hoursMinutesSecondsToFloatingHours(-63, -5, -56.7343), -1};
target rigel = {"Rigel", Ephemeris::hoursMinutesSecondsToFloatingHours(5, 14, 32.27210), Ephemeris::hoursMinutesSecondsToFloatingHours(-8, -12, -5.8981), -1};
target hadar = {"Hadar", Ephemeris::hoursMinutesSecondsToFloatingHours(14, 3, 49.40535), Ephemeris::hoursMinutesSecondsToFloatingHours(-60, -22, -22.9266), -1};
target betelgeuse = {"Betelgeuse", Ephemeris::hoursMinutesSecondsToFloatingHours(5, 56, 28.5), Ephemeris::hoursMinutesSecondsToFloatingHours(7, 24, 38), -1};
target procyon = {"Procyon", Ephemeris::hoursMinutesSecondsToFloatingHours(7, 40, 34.2), Ephemeris::hoursMinutesSecondsToFloatingHours(5, 9, 42.4), -1};
target pollux = {"Pollux", Ephemeris::hoursMinutesSecondsToFloatingHours(7, 46, 47.9), Ephemeris::hoursMinutesSecondsToFloatingHours(27, 58, 6.7), -1};
target spica = {"Spica", Ephemeris::hoursMinutesSecondsToFloatingHours(13, 26, 29.2), Ephemeris::hoursMinutesSecondsToFloatingHours(-11, -17, -22.4), -1};
target capella = {"Capella", Ephemeris::hoursMinutesSecondsToFloatingHours(5, 18, 27.8), Ephemeris::hoursMinutesSecondsToFloatingHours(46, 1, 27.4), -1};
target achernar = {"Achernar", Ephemeris::hoursMinutesSecondsToFloatingHours(1, 38, 35.2), Ephemeris::hoursMinutesSecondsToFloatingHours(-57, -6, -57.3), -1};
target antares = {"Antares", Ephemeris::hoursMinutesSecondsToFloatingHours(16, 30, 54.1), Ephemeris::hoursMinutesSecondsToFloatingHours(-26, -29, -9.3), -1};
// target altair = {"Altair", Ephemeris::hoursMinutesSecondsToFloatingHours(19, 50, 46.99855), Ephemeris::hoursMinutesSecondsToFloatingHours(8, 52, 5.9563), -1};
// target polaris = {"Polaris", Ephemeris::hoursMinutesSecondsToFloatingHours(2, 31, 49.09), Ephemeris::hoursMinutesSecondsToFloatingHours(89, 15, 50.8), -1};
target sigmaOctantis = {"Sigma Octantis", Ephemeris::hoursMinutesSecondsToFloatingHours(21, 8, 46.86357), Ephemeris::hoursMinutesSecondsToFloatingHours(-88, -57, -23.3983), -1};
target specialMoon = { "Moon", 0, 0, EarthsMoon};
target specialSaturn = { "Saturn", 0, 0, Saturn};
target specialJupiter = { "Jupiter", 0, 0, Jupiter};
target specialMars = { "Mars", 0, 0, Mars};
target specialVenus = { "Venus", 0, 0, Venus};
target specialSun = { "Sun", 0, 0, Sun};
// target arcturus = {"Arcturus", hourMinArcSecToDouble(14,15, 39.7), hourMinArcSecToDouble(19, 10, 57)};

target targets[] = { sirius, canopus, alphaCentauri, becrux, acrux, rigel, hadar, betelgeuse, procyon, pollux, capella, achernar, antares,
  sigmaOctantis, specialMoon, specialSaturn, specialJupiter, specialMars, specialVenus, specialSun };
target* calibratingTarget;

int calibratingStarIndex = 0;
calibrationPoint calibrationPoint0;
calibrationPoint calibrationPoint1;
int calibrated = 0;
//
// endregion

// region Hardcoded GPS lat/ lon
// 
float gpsLatitude = -22.660029;
float gpsLongitude = -46.939291;

//
// endregion

// region Where we are pointing at, where we are
//
EquatorialCoordinates equatorialCoordinates;
HorizontalCoordinates horizontalCoordinates;

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

int special = -1;

SolarSystemObject solarSystemObject;

char serialBuffer[128];
int serialBufferPointer = 0;

int lastClock = -1;
long encoderPosition = 0;
long clockCounter = 0;
int missedClocks = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("INITIALIZING...."); 
  lcd.setCursor(0, 1);
  lcd.print("    ...TELESCOPE"); 
  
  Serial.begin(115200);

  Ephemeris::setLocationOnEarth(-22.6599734F, -46.9420532F);
  Ephemeris::setAltitude(614);

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
    verticalMotor->setAcceleration(MAX_VERTICAL_ACCELERATION);
    verticalMotor->setAutoEnable(true);
  }
  horizontalMotor = engine.stepperConnectToPin(HORIZONTAL_STEPPER_STEP_PIN);
  if (horizontalMotor) {
    horizontalMotor->setDirectionPin(HORIZONTAL_STEPPER_DIR_PIN);
    horizontalMotor->setAcceleration(MAX_HORIZONTAL_ACCELERATION);
    horizontalMotor->setAutoEnable(true);
  }

  // Bluetooth  
  Serial1.begin(9600);
  
  pinMode(ACTION_INPUT_BUTTON, INPUT);
  pinMode(ENCODER_INPUT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(ENABLE_POT_BUTTON, INPUT_PULLUP);
  pinMode(HORIZONTAL_LED, OUTPUT);
  pinMode(VERTICAL_LED, OUTPUT);
  pinMode(37, INPUT);
  pinMode(39, INPUT);
  pinMode(41, INPUT);
  pinMode(43, INPUT);
  pinMode(45, INPUT);
  pinMode(47, INPUT);
  pinMode(49, INPUT);
  pinMode(51, INPUT);
  pinMode(53, INPUT);
  pinMode(55, INPUT);

  analogWrite(8, 240);

  //SolarSystemObject solarSystemObject = Ephemeris::solarSystemObjectAtDateAndTime((SolarSystemObjectIndex)num, day, month, year, hour, minute, second);
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
  
  if (monitorEncoder1Timer > 40) {
    int clock0 = digitalRead(37) == HIGH;
    int clock1 = digitalRead(39) == HIGH;
    int clockNumber = (clock0 << 0) + (clock1 << 1);
    if (clockNumber != lastClock) {
      clockCounter++;
      if (!(clockNumber == lastClock +1 || clockNumber == 0 && lastClock == 3)) {
        Serial.print("MISSED CLOCK");
        Serial.print(", last clock: ");
        Serial.print(lastClock);
        Serial.print(", clock: ");
        Serial.print(clockNumber);
        Serial.print(", bits: ");
        Serial.print(clock1);
        Serial.print(clock0);
        Serial.println();
        missedClocks++;
      }
      lastClock = clockNumber;
      int n0 = digitalRead(41) == HIGH;
      int n1 = digitalRead(43) == HIGH;
      int n2 = digitalRead(45) == HIGH;
      int n3 = digitalRead(47) == HIGH;
      int n4 = digitalRead(49) == HIGH;
      int n5 = digitalRead(51) == HIGH;
      int n6 = digitalRead(53) == HIGH;
      int delta = n0 + n1 << 1 + n2 << 2 + n3 << 3 + n4 << 4 + n5 << 5 + n6 << 6;
      int sign = digitalRead(55) == HIGH;
      if (sign) {
        delta *= -1;
      }
      encoderPosition += delta;
    }
    monitorEncoder1Timer = 0;
  }
  
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
    potHorizontalJoystickLeft = analogRead(HORIZONTAL_JOYSTICK_LEFT);
    potVerticalJoystickRight = analogRead(VERTICAL_JOYSTICK_RIGHT);

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
  
  if (special != -1) {
    solarSystemObject = Ephemeris::solarSystemObjectAtDateAndTime(special, 
      currentDay, currentMonth, currentYear,
      currentHour, currentMinute, currentSecond);
      
    ra = solarSystemObject.equaCoordinates.ra;
    dec = solarSystemObject.equaCoordinates.dec;
  }
  
  //
  // Standard loop cals
  equatorialCoordinates.ra = ra;
  equatorialCoordinates.dec = dec;
  
  horizontalCoordinates = Ephemeris::equatorialToHorizontalCoordinatesAtDateAndTime(
    equatorialCoordinates,
    currentDay, currentMonth, currentYear,
    currentHour, currentMinute, currentSecond);
                                                                                                      
  azm = horizontalCoordinates.azi;
  alt = horizontalCoordinates.alt;                      
  
  // SolarSystemObjectIndex;                                                                                
  
//   julianDate = julianDateCalc();
//   gstTime = utcToGstCalc();
//   lst = gstToLstCalc();
//   azimuthAltitudeCalculation();
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
  usePoint->currentYear = currentYear;
  usePoint->currentMonth = currentMonth;
  usePoint->currentDay = currentDay;
  usePoint->currentHour = currentHour;
  usePoint->currentMinute = currentMinute;
  usePoint->currentSecond = currentSecond;
  usePoint->horizontalPosition = horizontalMotor->getCurrentPosition(); 
  usePoint->verticalPosition = verticalMotor->getCurrentPosition(); 
}

void prepareStarCoordinates() {
  ra = calibratingTarget->ra;
  dec = calibratingTarget->dec;
  calculateEverything();
}

void prepareToMoveWithCalibration() {
    horizontalMotor->setAcceleration(MAX_HORIZONTAL_ACCELERATION);
    horizontalMotor->setAutoEnable(true);
    verticalMotor->setAcceleration(MAX_VERTICAL_ACCELERATION);
    verticalMotor->setAutoEnable(true);
    horizontalMotor->setSpeedInHz(MAX_HORIZONTAL_SPEED);
    verticalMotor->setSpeedInHz(MAX_VERTICAL_SPEED);
    horizontalMotor->applySpeedAcceleration();
    verticalMotor->applySpeedAcceleration();
}

void storeCalibrationData() {
  calibrated = 1;
  ledIncrement = 1;

  equatorialCoordinates.ra = calibrationPoint0.ra;
  equatorialCoordinates.dec = calibrationPoint0.dec;
  
  horizontalCoordinates = Ephemeris::equatorialToHorizontalCoordinatesAtDateAndTime(
    equatorialCoordinates,
    calibrationPoint0.currentDay, calibrationPoint0.currentMonth, calibrationPoint0.currentYear,
    calibrationPoint0.currentHour, calibrationPoint0.currentMinute, calibrationPoint0.currentSecond);
                                                                                                      
  azm1 = horizontalCoordinates.azi;
  alt1 = horizontalCoordinates.alt;                      

  equatorialCoordinates.ra = calibrationPoint1.ra;
  equatorialCoordinates.dec = calibrationPoint1.dec;
  
  horizontalCoordinates = Ephemeris::equatorialToHorizontalCoordinatesAtDateAndTime(
    equatorialCoordinates,
    calibrationPoint1.currentDay, calibrationPoint1.currentMonth, calibrationPoint1.currentYear,
    calibrationPoint1.currentHour, calibrationPoint1.currentMinute, calibrationPoint1.currentSecond);
                                                                                                      
  azm2 = horizontalCoordinates.azi;
  alt2 = horizontalCoordinates.alt;                      

  altMotor1 = calibrationPoint0.verticalPosition;
  altMotor2 = calibrationPoint1.verticalPosition;
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

int readHorizontalControl() {
  int horizontal = translatePotValueToSpeed(potHorizontalJoystickLeft, leftJoystickDirection);
  if (horizontal != 0) {
    return horizontal / max(MAX_LEFT_JOYSTICK_SPEED - leftJoystickSpeed, 1); 
  } else {
    return translatePotValueToSpeed(potHorizontal, -1);
  }
}

int readVerticalControl() {
  int vertical = translatePotValueToSpeed(potVerticalJoystickRight, rightJoystickDirection);
  if (vertical != 0) {
    return vertical / max(MAX_RIGHT_JOYSTICK_SPEED - rightJoystickSpeed, 1); 
  } else {
    return translatePotValueToSpeed(potVertical, -1);
  }
}

void moveMotors() {
  if (calibrated) {
    if(activeMode == MODE_MOVE_COORDINATES) {
      horizontalSpeed = readHorizontalControl();
      verticalSpeed = readVerticalControl();
      horizontalCoordinateSpeed = mapDouble(horizontalSpeed, -100, +100, -1, +1) / 500.0; 
      verticalCoordinateSpeed = mapDouble(verticalSpeed, -100, +100, -1, +1) / 500.0;
      long horizontalMotorDiff = abs(horizontalMotor->getCurrentPosition() - newHorizontalPos);
      if (horizontalMotorDiff < MAX_HORIZONTAL_SPEED) {
        ra += horizontalCoordinateSpeed;
      }
      long verticalMotorDiff = abs(verticalMotor->getCurrentPosition() - newVerticalPos);
      if (verticalMotorDiff < MAX_VERTICAL_SPEED) {
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
      horizontalSpeed = readHorizontalControl();
      verticalSpeed = readVerticalControl();
      horizontalMotorSpeed = map(horizontalSpeed, -100, +100, -1 * MAX_HORIZONTAL_SPEED, MAX_HORIZONTAL_SPEED);
      verticalMotorSpeed = map(verticalSpeed, -100, +100, -1 * MAX_VERTICAL_SPEED, MAX_VERTICAL_SPEED);
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