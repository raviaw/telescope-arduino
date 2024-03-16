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
#include <Arduino.h>
#include <AccelStepper.h>
#include <elapsedMillis.h>
#include <LiquidCrystal.h> // Inclui biblioteca "LiquidCristal.h"
#include "include/types.h"
#include "include/objects.h"
#include "include/time.h"
#include "include/lcd.h"
#include "include/calculations.h"

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

