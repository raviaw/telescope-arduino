#ifndef Globals_H
#define Globals_H

//
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ephemeris.h>

//
#define MAX_SPEED 7000
#define MAX_ACCELERATION 28000
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_TOP_AREA_PIXELS 16
#define SCREEN_BOTTOM_AREA_PIXELS 48
#define MPU1 0x69

// region Engine declarations
//  
#define VERTICAL_STEPPER_STEP_PIN 7
#define VERTICAL_STEPPER_DIR_PIN 24
#define HORIZONTAL_STEPPER_STEP_PIN 6
#define HORIZONTAL_STEPPER_DIR_PIN 22

#define LASER_PIN 43

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

extern float AccX,AccY,AccZ,Tmp1,GyroX,GyroY,GyroZ;
extern float accAngleX;
extern float accAngleY;
extern int horizontalBlinkOnTime;
extern int horizontalBlinkOffTime;
extern int horizontalBlinkState;
extern int verticalBlinkOnTime;
extern int verticalBlinkOffTime;
extern int verticalBlinkState;

extern int selectButtonAction1;
extern int selectButtonAction2;
extern int selectActionIndex;
extern int lastSelectButtonAction;

extern int selectedChoice;
extern int maxChoice;

extern int newHorizontalValue;
extern int newVerticalValue;

extern int activeMode;

//int lcdStarIndex = 0;
extern int lcdSelectedStar;

extern int potHorizontal;
extern int potVertical;
extern int potHorizontalJoystickLeft;
extern int potVerticalJoystickRight;
extern int androidHorizontalSpeed;
extern int androidVerticalSpeed;
extern int ledPower;
extern int ledIncrement;
extern int leftJoystickDirection;
extern int rightJoystickDirection;
extern int MAX_LEFT_JOYSTICK_SPEED;
extern int leftJoystickSpeed;
extern int MAX_RIGHT_JOYSTICK_SPEED;
extern int rightJoystickSpeed;

extern long lastKnobValue;

extern int lastJoystickNavigationValue;
//
// endregion
extern target sirius;
extern target canopus;
extern target alphaCentauri;
extern target becrux;
extern target acrux;
extern target rigel;
extern target hadar;
extern target betelgeuse;
extern target procyon;
extern target pollux;
extern target spica;
extern target capella;
extern target achernar;
extern target antares;
// target altair;
// target polaris;
extern target sigmaOctantis;
extern target specialMoon;
extern target specialSaturn;
extern target specialJupiter;
extern target specialMars;
extern target specialVenus;
extern target specialSun;
// target arcturus = {"Arcturus", hourMinArcSecToDouble(14,15, 39.7), hourMinArcSecToDouble(19, 10, 57)};

extern target targets[];
extern target* calibratingTarget;

extern int calibratingStarIndex;
extern calibrationPoint calibrationPoint0;
extern calibrationPoint calibrationPoint1;
extern int calibrated;
//
// endregion

// region Hardcoded GPS lat/ lon
// 
extern float gpsLatitude;
extern float gpsLongitude;

//
// endregion

// region Where we are pointing at, where we are
//
extern EquatorialCoordinates equatorialCoordinates;
extern HorizontalCoordinates horizontalCoordinates;

extern double ra;
extern double dec;
extern double julianDate;
extern double timeOfDay;
extern double gstTime;
extern double lst;
extern double alt;
extern double azm;

extern double currentMotorAlt;
extern double currentMotorAzm;
extern double currentEncoderAlt;
extern double currentEncoderAzm;

extern double lastStarRa;
extern double lastStarDec;

extern int horizontalSpeed;
extern int verticalSpeed;
extern float horizontalCoordinateSpeed;
extern float verticalCoordinateSpeed;

extern int loopsPerSec;
extern int rtcInitialized;

//
// endregion

// region Date/ time variables
//
extern int currentYear;
extern int currentMonth;
extern int currentDay;
extern int currentHour;
extern int currentMinute;
extern int currentSecond;
extern int currentMs;
extern long currentSecOfDay;
//
// endregion

// region Calibration info
// 
extern double alt1;
extern double alt2;
extern double altMotor1;
extern double altMotor2;
extern double altEncoder1;
extern double altEncoder2;
extern double azm1;
extern double azm2;
extern double azmMotor1;
extern double azmMotor2;
extern double azmEncoder1;
extern double azmEncoder2;
extern long newMotorVerticalPos;
extern long newMotorHorizontalPos;
extern long newEncoderVerticalPos;
extern long newEncoderHorizontalPos;
//
// endregion

extern double calcLst;
extern double calcRa;
extern double sinAlt;
extern double cosA;
extern double a;
extern double sinHa;

extern int special;

extern int slaveMode;

extern JsonDocument bluetoothDoc;
extern DeserializationError lastDeserializationStatus;
extern SolarSystemObject solarSystemObject;

extern char serialBuffer[128];
extern int serialBufferPointer;

// long horizontalEncoderPosition = 0;
// long verticalEncoderPosition = 0;
// 
// long horizontalEncoderPositionDelta = 0;
// long verticalEncoderPositionDelta = 0;
// long horizontalMotorPositionDelta = 0;
// long verticalMotorPositionDelta = 0;
// long lastHorizontalMotorPosition = 0;
// long lastVerticalMotorPosition = 0;
// 
// long horizontalBackslash = 0;
// long verticalBackslash = 0;
// int horizontalBackslashDirection = 0;
// int verticalBackslashDirection = 0;
// int horizontalBackslashMoves = 0;
// int verticalBackslashMoves = 0;
// long lastHorizontalEncoderBackslashPosition = 0;
// long lastVerticalEncoderBackslashPosition = 0;
// long lastHorizontalEncoderPosition = 0;
// long lastVerticalEncoderPosition = 0;
// long lastHorizontalBackslashEncoderPosition = 0;
// long lastVerticalBackslashEncoderPosition = 0;
// int lastHorizontalEncoderMoveDirection = 0;
// int lastVerticalEncoderMoveDirection = 0;
// int horizontalBackslashFinished = 0;
// int verticalBackslashFinished = 0;
// 
// int horizontalBackSlashing = 0;
// int verticalBackSlashing = 0;
// long horizontalEncoderBackslashLeft = 0;
// long verticalEncoderBackslashLeft = 0;
// long horizontalStandardBackslash = 300000;
// long verticalStandardBackslash = 10000;
// long horizontalMotorStartBackslash = 0;  
// long verticalMotorStartBackslash = 0;

double mapDouble(double x, double in_min, double in_max, double out_min, double out_max);

double withinBounds(double value, double min, double max);
  
#endif