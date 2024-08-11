#include "FastAccelStepper.h"

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *verticalMotor = NULL;

void setup() {
  engine.init();
  // engine.setDebugLed(26);
  verticalMotor = engine.stepperConnectToPin(9);

  verticalMotor->setDirectionPin(2);

  verticalMotor->setSpeedInHz(500);       // 500 steps/s
  verticalMotor->setAcceleration(100);    // 100 steps/s²
  verticalMotor->move(100000);
}

void loop() {
  // put your main code here, to run repeatedly:
}
