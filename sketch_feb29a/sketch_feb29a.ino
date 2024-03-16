#include <AFMotor.h>

// Number of steps per output rotation
// Change this as per your motor's specification
const int stepsPerRevolution = 200;

// connect motor to port #2 (M3 and M4)
AF_Stepper motor(stepsPerRevolution, 2);

void setup() {
  Serial.begin(9600);
  Serial.println("Stepper test!");

  motor.setSpeed(60);  // 10 rpm   
}

void loop() {
  Serial.println("Single coil steps");
  motor.step(100, FORWARD, SINGLE); 
  motor.step(100, BACKWARD, SINGLE); 

  // Serial.println("Double coil steps");
  // motor.step(100, FORWARD, DOUBLE); 
  // motor.step(100, BACKWARD, DOUBLE);

  // Serial.println("Interleave coil steps");
  // motor.step(100, FORWARD, INTERLEAVE); 
  // motor.step(100, BACKWARD, INTERLEAVE); 

}