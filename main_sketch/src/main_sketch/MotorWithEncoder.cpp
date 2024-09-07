#include <Arduino.h>
#include "Globals.hpp"
#include "MotorWithEncoder.h"

MotorWithEncoder::MotorWithEncoder(HardwareSerial* encoderSerialPort, FastAccelStepper* motor, int motorNumber, int motorStepPin, int motorDirectionPin) {
  _encoderSerialPort = encoderSerialPort;
  _motor = motor;
  _motorNumber = motorNumber;
  _motorStepPin = motorStepPin;
  _motorDirectionPin = motorDirectionPin;
}

void MotorWithEncoder::updateEncoderFromSerial() {
  //
  // Reads the buffer from the serial port
  while(_encoderSerialPort->available())
  { 
    int nextChar = _encoderSerialPort->read();
    if (nextChar != -1) { 
      if ((nextChar & 0x10) == 0x10) {
        if (_encoderBufferPointer >= 7) {
          long n8 = nextChar;
          long n7 = _encoderBuffer[_encoderBufferPointer - 1];
          long n6 = _encoderBuffer[_encoderBufferPointer - 2];
          long n5 = _encoderBuffer[_encoderBufferPointer - 3];
          long n4 = _encoderBuffer[_encoderBufferPointer - 4];
          long n3 = _encoderBuffer[_encoderBufferPointer - 5];
          long n2 = _encoderBuffer[_encoderBufferPointer - 6];
          long n1 = _encoderBuffer[_encoderBufferPointer - 7];
          
          int c1 = (n1 & 0x80) == 0x80;
          int c2 = (n2 & 0x40) == 0x40;
          int c3 = (n3 & 0x40) == 0x40;
          int c4 = (n4 & 0x40) == 0x40;
          int c5 = (n5 & 0x40) == 0x40;
          int c6 = (n6 & 0x40) == 0x40;
          int c7 = (n7 & 0x20) == 0x20;
          int c8 = (n8 & 0x10) == 0x10;
          
          long sum = 
            (((n1 & 0x0F) << 28) & 0xF0000000L) + 
            (((n2 & 0x0F) << 24) & 0x0F000000L) + 
            (((n3 & 0x0F) << 20) & 0x00F00000L) + 
            (((n4 & 0x0F) << 16) & 0x000F0000L) + 
            (((n5 & 0x0F) << 12) & 0x0000F000L) + 
            (((n6 & 0x0F) << 8) & 0x00000F00L) + 
            (((n7 & 0x0F) << 4) & 0x000000F0L) + 
            ((n8 & 0x0F) & 0x0000000FL);
  
          // Ensures noise is filtered out
          if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8) {
            setEncoderPosition(sum);
          }
        }
        _encoderBufferPointer = 0;
      } else if(_encoderBufferPointer < sizeof(_encoderBuffer) -1) {
        _encoderBuffer[_encoderBufferPointer] = nextChar;
        _encoderBufferPointer++;
      } else {
        _encoderBufferPointer = 0;
      }
    }
  }
}

long MotorWithEncoder::readMotorPosition() {
  return _motor->getCurrentPosition();
}

long MotorWithEncoder::readEncoderPosition() {
  return _encoderPosition;
}

void MotorWithEncoder::setCalibrationPoints(double trackPoint1, double trackPoint2, double trackEncoder1, double trackEncoder2, double trackMotor1, double trackMotor2) {
  _trackPoint1 = trackPoint1;
  _trackPoint2 = trackPoint2;
  _trackEncoder1 = trackEncoder1;
  _trackEncoder2 = trackEncoder2;
  _trackMotor1 = trackMotor1;
  _trackMotor2 = trackMotor2;
}

void MotorWithEncoder::moveMotors(double trackPoint, double speed) {
  if (_motorStatus == MOTOR_STATUS_CALC_BACKSLASH) { 
    keepMovingBackslash();
    return;
  }
  
  if (calibrated) {
    if(activeMode == MODE_MOVE_COORDINATES) {
    }
    
    _reverseMotorPosition = mapDouble(_motor->getCurrentPosition(), _trackMotor1, _trackMotor2, _trackPoint1, _trackPoint2);
    _reverseEncoderPosition = mapDouble(_encoderPosition, _trackEncoder1, _trackEncoder2, _trackPoint1, _trackPoint2);

    moveMotorsTrackingWithEncoder(trackPoint);
  } else {
    if(activeMode == MODE_MOVE_MOTOR || activeMode == MODE_CALIBRATE_MOVING) {
      _motorSpeed = map(speed, -100, +100, -1 * MAX_SPEED, MAX_SPEED);
    } else {
      _motorSpeed = 0;
    }

    int multiplier = 1;
    if (speed < 0) {
      multiplier = -1; 
    }

    if(_motorSpeed == 0) {
      _motor->stopMove();
    } else {
      _motor->setSpeedInHz(abs(_motorSpeed));
      _motor->applySpeedAcceleration();
      if (multiplier > 0) {
        _motor->runForward();
      } else {
        _motor->runBackward();
      }
    }
  }
}

void MotorWithEncoder::moveMotorsTrackingWithEncoder(double trackPoint) {
  long targetEncoderPosition = mapDouble(trackPoint, _trackPoint1, _trackPoint2, _trackEncoder1, _trackEncoder2);

  Serial.print("moveMotorsTrackingWithEncoder, "); 
  Serial.print(_motorNumber);
  Serial.print(", track point: ");
  Serial.print(trackPoint);
  Serial.print(", ");

  long diff = targetEncoderPosition - _encoderPosition;
  long absDiff = abs(diff); 
  Serial.print(", diff: ");
  Serial.print(diff);
  Serial.print(", abs diff: ");
  Serial.print(absDiff);
  if (absDiff < 5) {
    // Nothing to do
    Serial.println(", near enough, stopping");
    _motorTrackStatus = MOTOR_TRACK_STATUS_FORWARD;
    _motor->stopMove();
    return;
  }

  if (_encoderDirection >= 0 == diff >= 0) {
    Serial.print(", same direction...");
    // Same direction
      if (absDiff > 500) {
        _motor->setSpeedInHz(7000);
      } else if(absDiff > 200) {
        _motor->setSpeedInHz(6000);
      } else if(absDiff > 100) {
        _motor->setSpeedInHz(6000);
      } else if(absDiff > 30) {
        _motor->setSpeedInHz(500);
      } else {
        _motor->setSpeedInHz(50);
      }

//    if (absDiff > 20000) {
//      _motor->setSpeedInHz(7000);
//    } else if(absDiff > 15000) {
//      _motor->setSpeedInHz(6000);
//    } else if(absDiff > 15000) {
//      _motor->setSpeedInHz(6000);
//    } else if(absDiff > 10000) {
//      _motor->setSpeedInHz(3000);
//    } else if(absDiff > 5000) {
//      _motor->setSpeedInHz(1000);
//    } else {
//      _motor->setSpeedInHz(500);
//    }

    _motor->applySpeedAcceleration();
    if (_encoderDirection > 0) {
      _motor->runForward();
    } else {
      _motor->runBackward();
    }
    Serial.println(", running");
  } else {
    Serial.print(", different direction...");
    Serial.print(", motor track status: ");
    Serial.print(_motorTrackStatus);
    Serial.print(", max backslash: ");
    Serial.print(_maxBackslash);
    // Reverse direction
    if (_motorTrackStatus == MOTOR_TRACK_STATUS_FORWARD) {
      Serial.print(", MOTOR_TRACK_STATUS_FORWARD");
      if (diff < 0) {
        _backslashMotorPos = _motor->getCurrentPosition() - _maxBackslash; 
      } else {
        _backslashMotorPos = _motor->getCurrentPosition() + _maxBackslash; 
      } 
      Serial.print(", _backslashMotorPos: ");
      Serial.print(_backslashMotorPos);
      
      _motorTrackStatus = MOTOR_TRACK_STATUS_BACKSLASH_RUNNING;
      
      //
      Serial.println(", backslash running now");
    } else if (_motorTrackStatus == MOTOR_TRACK_STATUS_BACKSLASH_RUNNING) {
      Serial.print(", MOTOR_TRACK_STATUS_BACKSLASH_RUNNING");
      long backslashLeft = abs(_backslashMotorPos - _motor->getCurrentPosition());
      Serial.print(", backslashLeft: ");
      Serial.print(backslashLeft);
      
      if (backslashLeft > 500) {
        _motor->setSpeedInHz(7000);
      } else if(backslashLeft > 200) {
        _motor->setSpeedInHz(6000);
      } else if(backslashLeft > 100) {
        _motor->setSpeedInHz(6000);
      } else if(backslashLeft > 30) {
        _motor->setSpeedInHz(500);
      } else {
        _motor->setSpeedInHz(50);
      }

//      if (backslashLeft > 20000) {
//        _motor->setSpeedInHz(7000);
//      } else if(backslashLeft > 15000) {
//        _motor->setSpeedInHz(6000);
//      } else if(backslashLeft > 15000) {
//        _motor->setSpeedInHz(6000);
//      } else if(backslashLeft > 10000) {
//        _motor->setSpeedInHz(3000);
//      } else if(backslashLeft > 200) {
//        _motor->setSpeedInHz(5000);
//      } else {
//        _motor->setSpeedInHz(500);
//      }
      
      _motor->applySpeedAcceleration();
      
      if (_encoderDirection > 0) {
        _motor->runBackward();
      } else {
        _motor->runForward();
      }
  
      Serial.println(", running");
    } else {
      Serial.print(", UNKNOWN STATUS ");
      Serial.println(_motorTrackStatus);
    }
  }
}

void MotorWithEncoder::prepareToMoveWithCalibration() {
  _motor->setAcceleration(MAX_ACCELERATION);
  _motor->setSpeedInHz(MAX_SPEED);
  _motor->applySpeedAcceleration();
}

void MotorWithEncoder::setEncoderPosition(long newEncoderPosition) {
  _encoderPosition = newEncoderPosition;
  
  if (_encoderPosition != _previousEncoderPosition) {
    if (_encoderPosition < _previousEncoderPosition) {
      _encoderDirection = -1; 
    } else {
      _encoderDirection = +1; 
    }
    _previousEncoderPosition = _encoderPosition;
  }
}

void MotorWithEncoder::calculateBackslash() {
  Serial.println("Wants to calculate backslash");
  
  _motorStatus = MOTOR_STATUS_CALC_BACKSLASH;
  _backslashEncoderPosition0 = readEncoderPosition(); 
  _backslashMotorPosition0 = readMotorPosition();
  _backslashStep = 0;
  if (_encoderDirection <= 0) {
    _backslashDirection = 1;
  } else {
    _backslashDirection = -1;
  }
}

void MotorWithEncoder::keepMovingBackslash() {
  if (_motorStatus == MOTOR_STATUS_FREE) {
    return;
  }
  // This is very important
  updateEncoderFromSerial();
  
  long compareEncoderValue;
  long compareMotorValue;
  if (_backslashStep == 0) {
    compareEncoderValue = _backslashEncoderPosition0;
    compareMotorValue = _backslashMotorPosition0;
  } else if (_backslashStep == 1) {
    compareEncoderValue = _backslashEncoderPosition1;
    compareMotorValue = _backslashMotorPosition1;
  } else if (_backslashStep == 2) {
    compareEncoderValue = _backslashEncoderPosition2;
    compareMotorValue = _backslashMotorPosition2;
  } else if (_backslashStep == 3) {
    compareEncoderValue = _backslashEncoderPosition3;
    compareMotorValue = _backslashMotorPosition3;
  } else {
    _motorStatus = MOTOR_STATUS_FREE;
  }
  long currentEncoderPosition = readEncoderPosition();
  long currentMotorPosition = readMotorPosition();
  long diff = abs(currentEncoderPosition - compareEncoderValue);
  
  if (diff > 20) {
    Serial.print("Diff is ");
    Serial.print(diff);
    Serial.print(", current encoder position: ");
    Serial.print(currentEncoderPosition);
    Serial.print(", current motor position: ");
    Serial.print(currentMotorPosition);
    Serial.print(", compare encoder value: ");
    Serial.print(compareEncoderValue);
    Serial.print(", compare motor value: ");
    Serial.print(compareMotorValue);
    Serial.print(", backslash step: ");
    Serial.print(_backslashStep);
    Serial.println(", and motor stops");
    
    _motor->stopMove();
    delay(2000); // Let the motor stop
    
    // This is very important
    updateEncoderFromSerial();
    
    // Increase step - was 0 is now 1
    _backslashStep++;

    //
    // Read it again with everything stopped
    //
    currentEncoderPosition = readEncoderPosition();
    currentMotorPosition = readMotorPosition();
    
    Serial.print("After stopping");
    Serial.print(", current encoder position: ");
    Serial.print(currentEncoderPosition);
    Serial.print(", current motor position: ");
    Serial.print(currentMotorPosition);
    Serial.print(", step: ");
    Serial.print(_backslashStep);
    Serial.println();

    long encoderDiff = 0;
    long motorDiff = 0;

    if (_backslashStep == 1) {
      _backslashEncoderPosition1 = currentEncoderPosition;
      _backslashMotorPosition1 = currentMotorPosition;
      encoderDiff = abs(currentEncoderPosition - _backslashEncoderPosition0);
      motorDiff = abs(currentMotorPosition - _backslashMotorPosition0);
    } else if (_backslashStep == 2) {
      _backslashEncoderPosition2 = currentEncoderPosition;
      _backslashMotorPosition2 = currentMotorPosition;
      encoderDiff = abs(currentEncoderPosition - _backslashEncoderPosition1);
      motorDiff = abs(currentMotorPosition - _backslashMotorPosition1);
    } else if (_backslashStep == 3) {
      _backslashEncoderPosition3 = currentEncoderPosition;
      _backslashMotorPosition3 = currentMotorPosition;
      encoderDiff = abs(currentEncoderPosition - _backslashEncoderPosition2);
      motorDiff = abs(currentMotorPosition - _backslashMotorPosition2);
    } else if (_backslashStep == 4) {
      encoderDiff = abs(currentEncoderPosition - _backslashEncoderPosition3);
      motorDiff = abs(currentMotorPosition - _backslashMotorPosition3);
    }
    
    Serial.print("Backslash direction ");
    Serial.print(_backslashDirection);
    Serial.print(", step: ");
    Serial.print(_backslashStep);
    Serial.print(", encoderDiff: ");
    Serial.print(encoderDiff);
    Serial.print(", currentEncoderPosition: ");
    Serial.print(currentEncoderPosition);
    Serial.print(", currentMotorPosition: ");
    Serial.print(currentMotorPosition);
    Serial.print(", motorDiff: ");
    Serial.print(motorDiff);
    Serial.println();
    
    if (_backslashDirection > 0) {
      Serial.print("Calculating, encoder diff: ");
      Serial.print(encoderDiff);
      Serial.print(", positive backslash: ");
      Serial.println(_positiveBackslash);
      if (motorDiff > _positiveBackslash) {
        _positiveBackslash = motorDiff;
      }
    } else { 
      Serial.print("Calculating, encoder diff: ");
      Serial.print(encoderDiff);
      Serial.print(", negative backslash: ");
      Serial.println(_negativeBackslash);
      if (motorDiff > _negativeBackslash) {
        _negativeBackslash = motorDiff;
      }
    }
    
    if (_positiveBackslash > _maxBackslash) {
      _maxBackslash = _positiveBackslash;
    }
    if (_negativeBackslash > _maxBackslash) {
      _maxBackslash = _negativeBackslash;
    }
    
    _backslashDirection = _backslashDirection * -1;
  } else {
    // Serial.println("Still moving");
    _motor->setSpeedInHz(4000);
    _motor->applySpeedAcceleration();
    if (_backslashDirection > 0) {
      _motor->runForward();
    } else {
      _motor->runBackward();
    }
  }
}

void MotorWithEncoder::preloadBackslash(long backslash) {
  _maxBackslash = backslash;
}