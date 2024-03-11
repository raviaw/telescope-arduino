#include <AccelStepper.h>
#include <elapsedMillis.h>

AccelStepper verticalMotor(AccelStepper::DRIVER, 7, 6); 
AccelStepper horizontalMotor(AccelStepper::DRIVER, 8, 9); 

elapsedMillis printTime;

void setup() {
    Serial.begin(115200);
  // put your setup code here, to run once:
  verticalMotor.setMaxSpeed(1000.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  verticalMotor.setAcceleration(50.0);   // try other acceleration rates.
  verticalMotor.moveTo(30000); 
}

float stepsPerDegree = 1000;

int count = 0;    // tracks seconds to trigger an action if desired.
void loop() {
  float mSpeed;
  if (printTime >= 1000) {    // reports speed and position each second
    printTime = 0;
    mSpeed = verticalMotor.speed();
    Serial.print(mSpeed);
    Serial.print("  ");
    Serial.print(verticalMotor.currentPosition());
    Serial.print("  ");
    Serial.println(count);
    if (count++ == 30){      // uncomment an action to see the response in the serial monitor
      verticalMotor.moveTo(100);
      //myStepper.move(100);
      //myStepper.stop();
    }
  }
  verticalMotor.run();
}
