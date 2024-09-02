#ifndef MotorWithEncoder_H
#define MotorWithEncoder_H
#include <Arduino.h>
#include <FastAccelStepper.h>


class MotorWithEncoder {
  public:
    MotorWithEncoder(HardwareSerial* encoderSerialPort, FastAccelStepper* motor, int motorStepPin, int motorDirectionPin);
    void updateEncoderFromSerial();
    long readMotorPosition();
    long readEncoderPosition();
    void setEncoderPosition(long newPosition);
    void setCalibrationPoints(double trackPoint1, double trackPoint2, double trackEncoder1, double trackEncoder2, double trackMotor1, double trackMotor2);
    void moveMotors(double trackPoint, double speed);
    void moveMotorsTrackingWithEncoder(double trackPoint);
    void prepareToMoveWithCalibration();
  private:
    HardwareSerial* _encoderSerialPort;
    FastAccelStepper* _motor;
    int _motorStepPin;
    int _motorDirectionPin;
    byte _encoderBuffer[128];
    int _encoderBufferPointer = 0;
    long _encoderPosition = 0;
    long _previousEncoderPosition = 0;
    int _encoderDirection = 0;
    double _trackPoint1;
    double _trackPoint2;
    long _trackEncoder1;
    long _trackEncoder2;
    long _trackMotor1;
    long _trackMotor2;
    double _coordinateSpeed;
    long _motorSpeed;
};

#endif