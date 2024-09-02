#include "Globals.hpp"

double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double withinBounds(double value, double min, double max) {
  if (value < min) return min;
  else if (value > max) return max;
  else return value;
}

float AccX,AccY,AccZ,Tmp1,GyroX,GyroY,GyroZ;
float accAngleX;
float accAngleY;
int horizontalBlinkOnTime = 10;
int horizontalBlinkOffTime = 1000;
int horizontalBlinkState = 0; // 0 OFF, 1 ON
int verticalBlinkOnTime = 10;
int verticalBlinkOffTime = 1000;
int verticalBlinkState = 0; // 0 OFF, 1 ON

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

