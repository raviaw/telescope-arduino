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
#include <ArduinoJson.h>
#include <StreamUtils.h>
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
#include <EEPROM.h>

#define MAX_VERTICAL_SPEED 7000
#define MAX_VERTICAL_ACCELERATION 28000
#define MAX_HORIZONTAL_SPEED 7000
#define MAX_HORIZONTAL_ACCELERATION 28000

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_TOP_AREA_PIXELS 16
#define SCREEN_BOTTOM_AREA_PIXELS 48

const int MPU1 = 0x69; // Endereço do sensor 1

float AccX,AccY,AccZ,Tmp1,GyroX,GyroY,GyroZ;
float accAngleX;
float accAngleY;

// region Engine declarations
//  
LiquidCrystal lcd(11, 12, 4, 5, 9, 10); 
RTC_DS3231 rtc;
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *verticalMotor = NULL;
FastAccelStepper *horizontalMotor = NULL;           
Encoder knob(2, 3);
Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define VERTICAL_STEPPER_STEP_PIN 7
#define VERTICAL_STEPPER_DIR_PIN 24
#define HORIZONTAL_STEPPER_STEP_PIN 6
#define HORIZONTAL_STEPPER_DIR_PIN 22

#define HORIZONTAL_JOYSTICK_LEFT 8
#define VERTICAL_JOYSTICK_RIGHT 10

#define ACTION_INPUT_BUTTON 52
#define ENCODER_INPUT_BUTTON 37
#define LEFT_JOYSTICK_BUTTON 39
#define RIGHT_JOYSTICK_BUTTON 41
#define ENABLE_POT_BUTTON 35

#define LCD_INPUT_BUTTON 0
#define LCD_LIGHT_CONTROL 8
#define SPEED_POT 1
#define HORIZONTAL_POT 3
#define VERTICAL_POT 2
#define REFERENCE_INPUT_BUTTON 15


#define POT 1

#define HORIZONTAL_LED 26
#define VERTICAL_LED 28

#define MASTER_LED 31
#define SLAVE_LED 33

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

elapsedMillis monitorEncoderTimer;
elapsedMillis accelerometerTimer;

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
#define MODE_MEASURING_BACKSLASH 13
//
// endregion

// region Menu variablesþ
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
int androidHorizontalSpeed = 0;
int androidVerticalSpeed = 0;
int ledPower = 0;
int ledIncrement = 3;
int leftJoystickDirection = -1;
int rightJoystickDirection = -1;
int MAX_LEFT_JOYSTICK_SPEED = 10;
int leftJoystickSpeed = 5;
int MAX_RIGHT_JOYSTICK_SPEED = 10;
int rightJoystickSpeed = 5;

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
  long horizontalMotorPosition;
  long verticalMotorPosition;
  long horizontalEncoderPosition;
  long verticalEncoderPosition;
  int currentYear;
  int currentMonth;
  int currentDay;
  int currentHour;
  int currentMinute;
  int currentSecond;
} calibrationPoint;

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
double currentEncoderAlt;
double currentEncoderAzm;

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
double altEncoder1;
double altEncoder2;
double azm1;
double azm2;
double azmMotor1;
double azmMotor2;
double azmEncoder1;
double azmEncoder2;
long newMotorVerticalPos;
long newMotorHorizontalPos;
long newEncoderVerticalPos;
long newEncoderHorizontalPos;
//
// endregion

double calcLst;
double calcRa;
double sinAlt;
double cosA;
double a;
double sinHa;

int special = -1;

int slaveMode = 0;

JsonDocument bluetoothDoc;
DeserializationError lastDeserializationStatus;

SolarSystemObject solarSystemObject;

char serialBuffer[128];
int serialBufferPointer = 0;
byte horizontalEncoderBuffer[128];
int horizontalEncoderBufferPointer = 0;
byte verticalEncoderBuffer[128];
int verticalEncoderBufferPointer = 0;

long horizontalEncoderPosition = 0;
long verticalEncoderPosition = 0;

long horizontalEncoderPositionDelta = 0;
long verticalEncoderPositionDelta = 0;
long horizontalMotorPositionDelta = 0;
long verticalMotorPositionDelta = 0;
long lastHorizontalMotorPosition = 0;
long lastVerticalMotorPosition = 0;

long horizontalBackslash = 0;
long verticalBackslash = 0;
int horizontalBackslashDirection = 0;
int verticalBackslashDirection = 0;
int horizontalBackslashMoves = 0;
int verticalBackslashMoves = 0;
long lastHorizontalEncoderBackslashPosition = 0;
long lastVerticalEncoderBackslashPosition = 0;
long lastHorizontalEncoderPosition = 0;
long lastVerticalEncoderPosition = 0;
long lastHorizontalBackslashEncoderPosition = 0;
long lastVerticalBackslashEncoderPosition = 0;
int lastHorizontalEncoderMoveDirection = 0;
int lastVerticalEncoderMoveDirection = 0;
int horizontalBackslashFinished = 0;
int verticalBackslashFinished = 0;

int horizontalBackSlashing = 0;
int verticalBackSlashing = 0;
long horizontalEncoderBackslashLeft = 0;
long verticalEncoderBackslashLeft = 0;
long horizontalStandardBackslash = 300000;
long verticalStandardBackslash = 10000;
long horizontalMotorStartBackslash = 0;  
long verticalMotorStartBackslash = 0;  

void setup() {
  Wire.begin(); // Inicia a comunicação I2C
  Wire.beginTransmission(MPU1); //Começa a transmissao de dados para o sensor 1
  Wire.write(0x6B); // registrador PWR_MGMT_1
  Wire.write(0); // Manda 0 e "acorda" o sensor 1
  Wire.endTransmission(true);
  
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("INITIALIZING...."); 
  lcd.setCursor(0, 1);
  lcd.print("    ...TELESCOPE"); 
  // lcd.noDisplay();
  
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

  // Encoder #2
  Serial2.begin(57600);
  // Encoder #1
  Serial3.begin(57600);
  
  pinMode(ACTION_INPUT_BUTTON, INPUT);
  pinMode(ENCODER_INPUT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(ENABLE_POT_BUTTON, INPUT_PULLUP);
  pinMode(HORIZONTAL_LED, OUTPUT);
  pinMode(VERTICAL_LED, OUTPUT);
  pinMode(MASTER_LED, OUTPUT);
  pinMode(SLAVE_LED, OUTPUT);
  pinMode(LCD_LIGHT_CONTROL, OUTPUT);

  digitalWrite(MASTER_LED, HIGH);
  digitalWrite(SLAVE_LED, LOW);

  digitalWrite(LCD_LIGHT_CONTROL, HIGH);
  //SolarSystemObject solarSystemObject = Ephemeris::solarSystemObjectAtDateAndTime((SolarSystemObjectIndex)num, day, month, year, hour, minute, second);
  
  readEEPROMState();
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

  ///////////////////////////////////////////////////////////////////////////
  // Accelerometer
  //
  if (accelerometerTimer > 100) {
    Wire.beginTransmission(MPU1); //Começa a transmissao de dados do sensor 1
    Wire.write(0x3B); // Registrador dos dados medidos (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU1,14,true); // Faz um "pedido" para ler 14 registradores, que serão os registrados com os dados medidos
    AccX = (Wire.read()<<8|Wire.read()) / 16384.0;
    AccY = (Wire.read()<<8|Wire.read()) / 16384.0;
    AccZ = (Wire.read()<<8|Wire.read()) / 16384.0;
    accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
    accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + 1.58; // AccErrorY ~(-1.58)    Tmp1 = Wire.read()<<8|Wire.read();
    GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
    GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroX = GyroX + 0.56; // GyroErrorX ~(-0.56)
    GyroY = GyroY - 2; // GyroErrorY ~(2)
    GyroZ = GyroZ + 0.79; // GyroErrorZ ~ (-0.8)
    Wire.endTransmission(true); // Se der erro tente tirar esta linha

    accelerometerTimer = 0;
  }

  if (monitorEncoderTimer > 50) {
    while(Serial2.available())
    { 
      int nextChar = Serial2.read();
      if (nextChar != -1) { 
        // Serial.print("Available encoder: ");
        // Serial.println(nextChar);
        if ((nextChar & 0x10) == 0x10) {
          if (verticalEncoderBufferPointer >= 7) {
            // encoderBufferPointer++; // To do the correct math
            long n8 = nextChar;
            long n7 = verticalEncoderBuffer[verticalEncoderBufferPointer - 1];
            long n6 = verticalEncoderBuffer[verticalEncoderBufferPointer - 2];
            long n5 = verticalEncoderBuffer[verticalEncoderBufferPointer - 3];
            long n4 = verticalEncoderBuffer[verticalEncoderBufferPointer - 4];
            long n3 = verticalEncoderBuffer[verticalEncoderBufferPointer - 5];
            long n2 = verticalEncoderBuffer[verticalEncoderBufferPointer - 6];
            long n1 = verticalEncoderBuffer[verticalEncoderBufferPointer - 7];
            int c1 = (n1 & 0x80) == 0x80;
            int c2 = (n2 & 0x40) == 0x40;
            int c3 = (n3 & 0x40) == 0x40;
            int c4 = (n4 & 0x40) == 0x40;
            int c5 = (n5 & 0x40) == 0x40;
            int c6 = (n6 & 0x40) == 0x40;
            int c7 = (n7 & 0x20) == 0x20;
            int c8 = (n8 & 0x10) == 0x10;
            long sum = 
              (((n1 & 0x0F) << 28) & 0xF0000000L) + 
              (((n2 & 0x0F) << 24) & 0x0F000000L) + 
              (((n3 & 0x0F) << 20) & 0x00F00000L) + 
              (((n4 & 0x0F) << 16) & 0x000F0000L) + 
              (((n5 & 0x0F) << 12) & 0x0000F000L) + 
              (((n6 & 0x0F) << 8) & 0x00000F00L) + 
              (((n7 & 0x0F) << 4) & 0x000000F0L) + 
              ((n8 & 0x0F) & 0x0000000FL);

            // Ensures noise is filtered out
            if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8) {
              setVerticalEncoderPosition(sum);
            }
            // Serial.print("Encoder bytesc: ");
            // Serial.print(n1);
            // Serial.print(".");
            // Serial.print(c1);
            // Serial.print(", ");
            // Serial.print(n2);
            // Serial.print(".");
            // Serial.print(c2);
            // Serial.print(", ");
            // Serial.print(n3);
            // Serial.print(".");
            // Serial.print(c3);
            // Serial.print(", ");
            // Serial.print(n4);
            // Serial.print(".");
            // Serial.print(c4);
            // Serial.print(", ");
            // Serial.print(n5);
            // Serial.print(".");
            // Serial.print(c5);
            // Serial.print(", ");
            // Serial.print(n6);
            // Serial.print(".");
            // Serial.print(c6);
            // Serial.print(", ");
            // Serial.print(n7);
            // Serial.print(".");
            // Serial.print(c7);
            // Serial.print(", ");
            // Serial.print(n8);
            // Serial.print(".");
            // Serial.print(c8);
            // Serial.print(", ");
            // Serial.print(sum);
            // Serial.print(", ");
            // Serial.print(encoderPosition);
            // Serial.println();
          }
          verticalEncoderBufferPointer = 0;
        } else if(verticalEncoderBufferPointer < sizeof(verticalEncoderBuffer) -1) {
          verticalEncoderBuffer[verticalEncoderBufferPointer] = nextChar;
          verticalEncoderBufferPointer++;
        } else {
          verticalEncoderBufferPointer = 0;
        }
      }
    }

    while(Serial3.available())
    { 
      int nextChar = Serial3.read();
      if (nextChar != -1) { 
        // Serial.print("Available encoder: ");
        // Serial.println(nextChar);
        if ((nextChar & 0x10) == 0x10) {
          if (horizontalEncoderBufferPointer >= 7) {
            // encoderBufferPointer++; // To do the correct math
            long n8 = nextChar;
            long n7 = horizontalEncoderBuffer[horizontalEncoderBufferPointer - 1];
            long n6 = horizontalEncoderBuffer[horizontalEncoderBufferPointer - 2];
            long n5 = horizontalEncoderBuffer[horizontalEncoderBufferPointer - 3];
            long n4 = horizontalEncoderBuffer[horizontalEncoderBufferPointer - 4];
            long n3 = horizontalEncoderBuffer[horizontalEncoderBufferPointer - 5];
            long n2 = horizontalEncoderBuffer[horizontalEncoderBufferPointer - 6];
            long n1 = horizontalEncoderBuffer[horizontalEncoderBufferPointer - 7];
            int c1 = (n1 & 0x80) == 0x80;
            int c2 = (n2 & 0x40) == 0x40;
            int c3 = (n3 & 0x40) == 0x40;
            int c4 = (n4 & 0x40) == 0x40;
            int c5 = (n5 & 0x40) == 0x40;
            int c6 = (n6 & 0x40) == 0x40;
            int c7 = (n7 & 0x20) == 0x20;
            int c8 = (n8 & 0x10) == 0x10;
            long sum = 
              (((n1 & 0x0F) << 28) & 0xF0000000L) + 
              (((n2 & 0x0F) << 24) & 0x0F000000L) + 
              (((n3 & 0x0F) << 20) & 0x00F00000L) + 
              (((n4 & 0x0F) << 16) & 0x000F0000L) + 
              (((n5 & 0x0F) << 12) & 0x0000F000L) + 
              (((n6 & 0x0F) << 8) & 0x00000F00L) + 
              (((n7 & 0x0F) << 4) & 0x000000F0L) + 
              ((n8 & 0x0F) & 0x0000000FL);

            // Ensures noise is filtered out
            if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8) {
              setHorizontalEncoderPosition(sum);
            }
            // Serial.print("Encoder bytesc: ");
            // Serial.print(n1);
            // Serial.print(".");
            // Serial.print(c1);
            // Serial.print(", ");
            // Serial.print(n2);
            // Serial.print(".");
            // Serial.print(c2);
            // Serial.print(", ");
            // Serial.print(n3);
            // Serial.print(".");
            // Serial.print(c3);
            // Serial.print(", ");
            // Serial.print(n4);
            // Serial.print(".");
            // Serial.print(c4);
            // Serial.print(", ");
            // Serial.print(n5);
            // Serial.print(".");
            // Serial.print(c5);
            // Serial.print(", ");
            // Serial.print(n6);
            // Serial.print(".");
            // Serial.print(c6);
            // Serial.print(", ");
            // Serial.print(n7);
            // Serial.print(".");
            // Serial.print(c7);
            // Serial.print(", ");
            // Serial.print(n8);
            // Serial.print(".");
            // Serial.print(c8);
            // Serial.print(", ");
            // Serial.print(sum);
            // Serial.print(", ");
            // Serial.print(encoderPosition);
            // Serial.println();
          }
          horizontalEncoderBufferPointer = 0;
        } else if(horizontalEncoderBufferPointer < sizeof(horizontalEncoderBuffer) -1) {
          horizontalEncoderBuffer[horizontalEncoderBufferPointer] = nextChar;
          horizontalEncoderBufferPointer++;
        } else {
          horizontalEncoderBufferPointer = 0;
        }
      }
    }

    monitorEncoderTimer = 0;
  }
 
   if(Serial1.available()) {
     bluetoothSerialAvailable();
   }
  //   char nextChar = Serial1.read();
  //   if (nextChar != -1) { 
  //     if (nextChar == '\n') {
  //       if (serialBufferPointer > 0) {
  //         serialBuffer[serialBufferPointer] = 0;
  //         Serial.print("Received serial coordinates: ");
  //         Serial.write(serialBuffer);
  //         Serial.println();
        
  //         processInput(serialBuffer);
  //       }
  //       serialBufferPointer = 0;
  //     } else if(serialBufferPointer < sizeof(serialBuffer) -1) {
  //       serialBuffer[serialBufferPointer] = nextChar;
  //       serialBufferPointer++;
  //     }    
  //   }
  // }
    
  loopsPerSec++;

  registerButton();

  if (calcTime > 25) {
    if (!slaveMode) {
      if(digitalRead(ENABLE_POT_BUTTON) == 1) {
        potHorizontal = analogRead(HORIZONTAL_POT);
        potVertical = analogRead(VERTICAL_POT);
      } else {
        potHorizontal = 512;
        potVertical = 512;
      }
      potHorizontalJoystickLeft = analogRead(HORIZONTAL_JOYSTICK_LEFT);
      potVerticalJoystickRight = analogRead(VERTICAL_JOYSTICK_RIGHT);
    } else {
      potHorizontal = 512;
      potVertical = 512;
      potHorizontalJoystickLeft = 512;
      potVerticalJoystickRight = 512;
      newHorizontalValue = 0;
      newVerticalValue = 0;
      lastJoystickNavigationValue = 0;
    }

    //
    calculateEverything();
   
    calcTime = 0;
  }
  
  if (moveMotors > 100) {
    moveMotors();

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

//   if (logMotorsTime > 100) {
//     float maxHorizontalPercentage = mapDouble(abs(horizontalMotor->getCurrentSpeedInUs()), 0, horizontalMotor->getMaxSpeedInUs(), 0, 100.0);
//     float maxVerticalPercentage = mapDouble(abs(verticalMotor->getCurrentSpeedInUs()), 0, verticalMotor->getMaxSpeedInUs(), 0, 100.0);
// 
//     logMotorsTime = 0;
//   }
}

void setHorizontalEncoderPosition(long sum) {
  horizontalEncoderPosition = sum;
  if (horizontalEncoderPosition != lastHorizontalEncoderPosition) {
    if (horizontalEncoderPosition < lastHorizontalEncoderPosition) {
      lastHorizontalEncoderMoveDirection = -1; 
    } else {
      lastHorizontalEncoderMoveDirection = +1; 
    }
    lastHorizontalEncoderPosition = horizontalEncoderPosition;
  }
}

void setVerticalEncoderPosition(long sum) {
  verticalEncoderPosition = sum;
  if (verticalEncoderPosition != lastVerticalEncoderPosition) {
    if (verticalEncoderPosition < lastVerticalEncoderPosition) {
      lastVerticalEncoderMoveDirection = -1; 
    } else {
      lastVerticalEncoderMoveDirection = +1; 
    }
    lastVerticalEncoderPosition = verticalEncoderPosition;
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
  usePoint->horizontalMotorPosition = readHorizontalMotorPosition(); 
  usePoint->verticalMotorPosition = readVerticalMotorPosition(); 
  usePoint->horizontalEncoderPosition = readHorizontalEncoderPosition(); 
  usePoint->verticalEncoderPosition = readVerticalEncoderPosition(); 
}

void prepareStarCoordinates() {
  ra = calibratingTarget->ra;
  dec = calibratingTarget->dec;
  calculateEverything();
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

  altMotor1 = calibrationPoint0.verticalMotorPosition;
  altMotor2 = calibrationPoint1.verticalMotorPosition;
  azmMotor1 = calibrationPoint0.horizontalMotorPosition;
  azmMotor2 = calibrationPoint1.horizontalMotorPosition;
  altEncoder1 = calibrationPoint0.verticalEncoderPosition;
  altEncoder2 = calibrationPoint1.verticalEncoderPosition;
  azmEncoder1 = calibrationPoint0.horizontalEncoderPosition;
  azmEncoder2 = calibrationPoint1.horizontalEncoderPosition;

  // So that it doesn't move when the last point moves
  ra = calibrationPoint1.ra;
  dec = calibrationPoint1.dec;
  
  storeEEPROMData();

  calculateEverything();
}

int readHorizontalControl() {
  if (slaveMode) {
    return withinBounds(androidHorizontalSpeed, -100, +100); // 100 stops;
  } else {
    int horizontal = translatePotValueToSpeed(potHorizontalJoystickLeft, leftJoystickDirection);
    if (horizontal != 0) {
      return horizontal / max(MAX_LEFT_JOYSTICK_SPEED - leftJoystickSpeed, 1); 
   } else {
      return translatePotValueToSpeed(potHorizontal, -1);
    }
  }
}

int readVerticalControl() {
  if (slaveMode) {
    return withinBounds(androidVerticalSpeed, -100, +100); // 100 stops;
  } else {
  int vertical = translatePotValueToSpeed(potVerticalJoystickRight, rightJoystickDirection);
    if (vertical != 0) {
      return vertical / max(MAX_RIGHT_JOYSTICK_SPEED - rightJoystickSpeed, 1); 
    } else {
      return translatePotValueToSpeed(potVertical, -1);
    }
  }
}

