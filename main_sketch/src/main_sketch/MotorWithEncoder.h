#ifndef MotorWithEncoder_H
#define MotorWithEncoder_H
#include <Arduino.h>
#include <FastAccelStepper.h>

#define MOTOR_STATUS_FREE 0
#define MOTOR_STATUS_BACKSLASH 1

class MotorWithEncoder {
  public:
    MotorWithEncoder(HardwareSerial* encoderSerialPort, FastAccelStepper* motor, int motorStepPin, int motorDirectionPin);
    void updateEncoderFromSerial();
    long readMotorPosition();
    long readEncoderPosition();
    void setCalibrationPoints(double trackPoint1, double trackPoint2, double trackEncoder1, double trackEncoder2, double trackMotor1, double trackMotor2);
    void moveMotors(double trackPoint, double speed);
    void prepareToMoveWithCalibration();
    void calculateBackslash();
  private:
    void keepMovingBackslash();
    void moveMotorsTrackingWithEncoder(double trackPoint);
    void setEncoderPosition(long newPosition);

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
    
    // Backslash
    int _motorStatus = MOTOR_STATUS_FREE;
    int _backslashDirection = 0;
    int _backslashStep = 0;
    long _backslashEncoderPosition0;
    long _backslashEncoderPosition1;
    long _backslashEncoderPosition2;
    long _backslashEncoderPosition3;
    long _backslashMotorPosition0;
    long _backslashMotorPosition1;
    long _backslashMotorPosition2;
    long _backslashMotorPosition3;
    long _negativeBackslash = 0;
    long _positiveBackslash = 0;
};

#endif