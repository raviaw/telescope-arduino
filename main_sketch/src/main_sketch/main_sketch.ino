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
#include <StreamUtils.h>
#include <elapsedMillis.h>
#include <LiquidCrystal.h> // Inclui biblioteca "LiquidCristal.h"
#include <FastAccelStepper.h>
#include <Wire.h>
#include <RTClib.h>
#include <Encoder.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <EEPROM.h>

#include "Globals.hpp"
#include "MotorWithEncoder.h"

// Global objects
//
LiquidCrystal lcd(11, 12, 4, 5, 9, 10); 
RTC_DS3231 rtc;
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *verticalMotor = NULL;
FastAccelStepper *horizontalMotor = NULL;           
Encoder knob(2, 3);
Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MotorWithEncoder *verticalMagic = NULL;
MotorWithEncoder *horizontalMagic = NULL;           

// Timers
//
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
    verticalMotor->setAcceleration(MAX_ACCELERATION);
    verticalMotor->setAutoEnable(true);
  }
  horizontalMotor = engine.stepperConnectToPin(HORIZONTAL_STEPPER_STEP_PIN);
  if (horizontalMotor) {
    horizontalMotor->setDirectionPin(HORIZONTAL_STEPPER_DIR_PIN);
    horizontalMotor->setAcceleration(MAX_ACCELERATION);
    horizontalMotor->setAutoEnable(true);
  }
  
  // Bluetooth  
  Serial1.begin(9600);

  // Encoder #2
  Serial2.begin(57600);
  // Encoder #1
  Serial3.begin(57600);
  
  verticalMagic = new MotorWithEncoder(&Serial2, verticalMotor, VERTICAL_STEPPER_STEP_PIN, VERTICAL_STEPPER_DIR_PIN);
  horizontalMagic = new MotorWithEncoder(&Serial3, horizontalMotor, HORIZONTAL_STEPPER_STEP_PIN, HORIZONTAL_STEPPER_DIR_PIN);

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
    return;
  }
  //
  ///////////////////////////////////////////////////////////////////////////

  loopsPerSec++;

  registerButton();

  if(Serial1.available()) {
    bluetoothSerialAvailable();
  }
    

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
    verticalMagic->updateEncoderFromSerial();
    horizontalMagic->updateEncoderFromSerial();
    monitorEncoderTimer = 0;
  }
 
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
  
  if (moveMotorsTime > 100) {
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
  usePoint->horizontalMotorPosition = horizontalMagic->readMotorPosition(); 
  usePoint->verticalMotorPosition = verticalMagic->readMotorPosition(); 
  usePoint->horizontalEncoderPosition = horizontalMagic->readEncoderPosition(); 
  usePoint->verticalEncoderPosition = verticalMagic->readEncoderPosition(); 
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
  
  horizontalMagic->setCalibrationPoints(azm1, azm2, azmEncoder1, azmEncoder2, azmMotor1, azmMotor2);
  verticalMagic->setCalibrationPoints(alt1, alt2, altEncoder1, altEncoder2, altMotor1, altMotor2);

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

//
// Global functions
//
void prepareToMoveWithCalibration() {
  horizontalMagic->prepareToMoveWithCalibration();
  verticalMagic->prepareToMoveWithCalibration();
}

void moveMotors() {

}

void measureBackslash() {
}
