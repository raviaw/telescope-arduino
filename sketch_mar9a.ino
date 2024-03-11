#include <AccelStepper.h>
#include <elapsedMillis.h>

AccelStepper verticalMotor(AccelStepper::DRIVER, 4, 3); 
AccelStepper horizontalMotor(AccelStepper::DRIVER, 6, 5); 

elapsedMillis printTime;

// Reduction X Axis = 1/ 198
// Reduction Z Axis = 1/ 33

void setup() {
    Serial.begin(115200);
  // put your setup code here, to run once:
  verticalMotor.setMaxSpeed(2000.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  verticalMotor.setAcceleration(100.0);   // try other acceleration rates.
  verticalMotor.moveTo(79200); 
  horizontalMotor.setMaxSpeed(100.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizontalMotor.setAcceleration(50.0);   // try other acceleration rates.
  horizontalMotor.moveTo(-5000); 
}

float stepsPerDegree = 1000;

int count = 0;    // tracks seconds to trigger an action if desired.
void loop() {
  float verticalSpeed;
  float horizontalSpeed;
  if (printTime >= 1000) {    // reports speed and position each second
    printTime = 0;
    verticalSpeed = verticalMotor.speed();
    Serial.print("V: ");
    Serial.print(verticalSpeed);
    Serial.print("  ");
    Serial.print(verticalMotor.currentPosition());
    Serial.print("  ");
    Serial.println(count);

    horizontalSpeed = horizontalMotor.speed();
    Serial.print("H: ");
    Serial.print(horizontalSpeed);
    Serial.print("  ");
    Serial.print(horizontalMotor.currentPosition());
    Serial.print("  ");
    Serial.println(count);

    if (count++ == 75){      // uncomment an action to see the response in the serial monitor
      verticalMotor.moveTo(0);
      horizontalMotor.moveTo(0);
      //myStepper.move(100);
      //myStepper.stop();
    }
  }
  verticalMotor.run();
  horizontalMotor.run();
}
