#ifndef MotorWithEncoder_H
#define MotorWithEncoder_H
#include <Arduino.h>
#include <FastAccelStepper.h>


class MotorWithEncoder {
  public:
    MotorWithEncoder(HardwareSerial* encoderSerialPort, FastAccelStepper* stepper, int motorStepPin, int motorDirectionPin);
    void updateEncoderFromSerial();
  private:
    HardwareSerial* _encoderSerialPort;
    FastAccelStepper* _stepper;
    int _motorStepPin;
    int _motorDirectionPin;
    byte encoderBuffer[128];
    int encoderBufferPointer = 0;
};

#endif