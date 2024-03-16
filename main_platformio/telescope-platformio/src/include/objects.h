#include <AccelStepper.h>
#include <elapsedMillis.h>
#include <LiquidCrystal.h> // Inclui biblioteca "LiquidCristal.h"
#include "types.h"

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

AccelStepper verticalMotor(AccelStepper::DRIVER, 4, 3); 
AccelStepper horizontalMotor(AccelStepper::DRIVER, 6, 5); 

elapsedMillis printTime;

target Polaris = {"Polaris", 89, 15, 50.78, 2, 31, 48.704};           // (Ursa Minor)
target Antares = {"Antares", -26, 25, 55.20, 16, 29, 24};             // (Scorpio)
target Arcturus = {"Arcturus", 19, 10, 56.67, 14, 15, 39};            // (Bootes)
target Betelgeuse = {"Betelgeuse", 7, 24, 25.43, 5, 55, 10};          // (Orion)
target Regulus = {"Regulus", 11, 58, 1.95, 10, 8, 22};                // (Leo)
target Spica = {"Spica", -11, 9, 40.76, 13, 25, 11};                  // (Virgo)
target Altair = {"Altair", 8, 52, 5.96, 19, 50, 46};                  // (Aquilla)
target Deneb = {"Deneb", 45, 16, 49.22, 20, 41, 25};                  // (Cygnus)
target Vega = {"Vega", 38, 47, 1.29, 18, 36, 56};                     // (Lyra)
target Andromeda_G = {"Andromeda Galaxy", 41, 16, 8.76, 0, 42, 44};   // (near the Andromeda constellation)

// array of predefined targets
target predefined[10] = {Polaris, Antares, Arcturus, Betelgeuse, Regulus, Spica, Altair, Deneb, Vega, Andromeda_G};

float gpslat = -22.6599734;
float gpslon = -46.9420532;
//unsigned long age;
//double timenow;


// Local Sidereal Time variables
double LST_degrees; // variable to store local side real time(LST) in degrees.
double LST_hours;   // variable to store local side real time(LST) in decimal hours.
double LST_minutes;
double LST_seconds;

// Buttons
int lastButtonAction = -1;
#define RIGHT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define SELECT 5

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
