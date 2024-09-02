long readHorizontalMotorPosition() {
  return horizontalMotor->getCurrentPosition();
}
long readVerticalMotorPosition() {
  return verticalMotor->getCurrentPosition();
}
long readHorizontalEncoderPosition() {
  return horizontalEncoderPosition;
}
long readVerticalEncoderPosition() {
  return verticalEncoderPosition;
}

void moveMotorsTracking() {
  newMotorVerticalPos = mapDouble(alt, alt1, alt2, altMotor1, altMotor2);
  newMotorHorizontalPos = mapDouble(azm, azm1, azm2, azmMotor1, azmMotor2);
  moveHorizontalMotorTo(newMotorHorizontalPos);
  moveVerticalMotorTo(newMotorVerticalPos);
}

void moveMotorsTrackingWithEncoder() {
  newEncoderVerticalPos = mapDouble(alt, alt1, alt2, altEncoder1, altEncoder2);
  newEncoderHorizontalPos = mapDouble(azm, azm1, azm2, azmEncoder1, azmEncoder2);

  long currentHorizontalMotorPosition = readHorizontalMotorPosition();
  
  moveMotorTrackingWithEncoder(0, horizontalMotor, readHorizontalEncoderPosition(), newEncoderHorizontalPos, readHorizontalMotorPosition(), lastHorizontalMotorPosition, horizontalBackSlashing, horizontalEncoderBackslashLeft, horizontalMotorStartBackslash, horizontalStandardBackslash);
  lastHorizontalMotorPosition = currentHorizontalMotorPosition;

  long currentVerticalMotorPosition = readVerticalMotorPosition();
  // moveMotorTrackingWithEncoder(1, verticalMotor, readVerticalEncoderPosition(), newEncoderVerticalPos, readVerticalMotorPosition(), lastVerticalMotorPosition, verticalBackSlashing, verticalEncoderBackslashLeft, verticalMotorStartBackslash, verticalStandardBackslash);
  lastVerticalMotorPosition = currentVerticalMotorPosition;
}

void moveMotorTrackingWithEncoder(int motorNo, FastAccelStepper *motor, long currentEncoderPos, long targetEncoderPos, long currentMotorPos, long lastMotorPos, int& backSlashing, long& backslashLeft, long& motorStartBackslash, long standardBackslash) {
  long diff = abs(currentEncoderPos - targetEncoderPos); 
  if (diff < 5) {
    // Nothing to do
    motor->stopMove();
    return;
  }
  int encoderDirection = currentEncoderPos < targetEncoderPos ? +1 : -1;
  int motorDirection = currentMotorPos < lastMotorPos ? +1 : -1;
  long currentSpeed = abs(motor->getCurrentSpeedInMilliHz(true));
  Serial.print("Motor: ");
  Serial.print(motorNo);
  Serial.print(", current: ");
  Serial.print(currentEncoderPos);
  Serial.print(", target: ");
  Serial.print(targetEncoderPos);
  Serial.print(", diff: ");
  Serial.print(diff);
  Serial.print(", encoderDirection: ");
  Serial.print(encoderDirection);
  Serial.print(", motorDirection: ");
  Serial.print(motorDirection);
  Serial.print(", currentSpeed: ");
  Serial.print(currentSpeed);
  Serial.print(", backSlashing: ");
  Serial.print(backSlashing);
  Serial.print(", backslashLeft: ");
  Serial.print(backslashLeft);
  Serial.print(", motorStartBackslash: ");
  Serial.print(motorStartBackslash);
  Serial.print(", standardBackslash: ");
  Serial.print(standardBackslash);
  Serial.println();
  
  if (encoderDirection != motorDirection) {
    if (!backSlashing) {
      backslashLeft = standardBackslash;
      motorStartBackslash = motor->getCurrentPosition();
      backSlashing = 1;
      motor->setSpeedInHz(7000);
    } else {
      long motorDiff = abs(motor->getCurrentPosition() - motorStartBackslash);
      backslashLeft -= motorDiff;
      if (backslashLeft > 20000) {
        motor->setSpeedInHz(7000);
      } else if(backslashLeft > 15000) {
        motor->setSpeedInHz(6000);
      } else if(backslashLeft > 15000) {
        motor->setSpeedInHz(6000);
      } else if(backslashLeft > 10000) {
        motor->setSpeedInHz(3000);
      } else if(backslashLeft > 5000) {
        motor->setSpeedInHz(1000);
      } else {
        motor->setSpeedInHz(500);
      }
    }
  } else {
    backSlashing = 0;
    if (diff < 10) {
      motor->setSpeedInHz(500);
    } else if (diff > 10 && diff < 20) {
      motor->setSpeedInHz(1000);
    } else {
      motor->setSpeedInHz(2000);
    }
  } 
  motor->applySpeedAcceleration();
  if (encoderDirection > 0) {
    motor->runForward();
  } else {
    motor->runBackward();
  }
}

void prepareToMoveWithCalibration() {
  horizontalMotor->setAcceleration(MAX_HORIZONTAL_ACCELERATION);
  horizontalMotor->setAutoEnable(true);
  verticalMotor->setAcceleration(MAX_VERTICAL_ACCELERATION);
  verticalMotor->setAutoEnable(true);
  horizontalMotor->setSpeedInHz(MAX_HORIZONTAL_SPEED);
  verticalMotor->setSpeedInHz(MAX_VERTICAL_SPEED);
  horizontalMotor->applySpeedAcceleration();
  verticalMotor->applySpeedAcceleration();
}

void moveHorizontalMotorTo(long newPosition) {
  horizontalMotor->moveTo(newPosition);
}

void moveVerticalMotorTo(long newPosition) {
  verticalMotor->moveTo(newPosition);
}

void moveMotors() {
  if (activeMode == MODE_MEASURING_BACKSLASH) {
    // void moveMeasuringBackslash(FastAccelStepper *motor, long encoderPosition, int& backslashFinished, int& backslashMoveCount, int& backslashMoveDirection, int& encoderMoveDirection, long& backlashValue) {
    if (!horizontalBackslashFinished) {
      moveMeasuringBackslash(
        horizontalMotor, 
        lastHorizontalEncoderBackslashPosition, 
        horizontalEncoderPosition, 
        horizontalBackslashFinished, 
        horizontalBackslashMoves, 
        horizontalBackslashDirection, 
        lastHorizontalEncoderMoveDirection, 
        horizontalBackslash);
    }
    
    if (horizontalBackslashFinished) {
      activeMode = MODE_MENU;
    }
    return;
  }
  
  if (calibrated) {
    if(activeMode == MODE_MOVE_COORDINATES) {
      horizontalSpeed = readHorizontalControl();
      verticalSpeed = readVerticalControl();
      horizontalCoordinateSpeed = mapDouble(horizontalSpeed, -100, +100, -1, +1) / 500.0; 
      verticalCoordinateSpeed = mapDouble(verticalSpeed, -100, +100, -1, +1) / 500.0;
      long horizontalMotorDiff = abs(readHorizontalMotorPosition() - newMotorHorizontalPos);
      if (horizontalMotorDiff < MAX_HORIZONTAL_SPEED) {
        ra += horizontalCoordinateSpeed;
      }
      long verticalMotorDiff = abs(readVerticalMotorPosition() - newMotorVerticalPos);
      if (verticalMotorDiff < MAX_VERTICAL_SPEED) {
        dec += verticalCoordinateSpeed;
      }
    }
    
    currentMotorAlt = mapDouble(readVerticalMotorPosition(), altMotor1, altMotor2, alt1, alt2);
    currentMotorAzm = mapDouble(readHorizontalMotorPosition(), azmMotor1, azmMotor2, azm1, azm2);
    currentEncoderAlt = mapDouble(readVerticalEncoderPosition(), altEncoder1, altEncoder2, alt1, alt2);
    currentEncoderAzm = mapDouble(readHorizontalEncoderPosition(), azmEncoder1, azmEncoder2, azm1, azm2);

    // moveMotorsTracking();
    moveMotorsTrackingWithEncoder();
  } else {
    currentMotorAlt = alt;
    currentMotorAzm = azm;

    if(activeMode == MODE_MOVE_MOTOR || activeMode == MODE_CALIBRATE_MOVING) {
      horizontalSpeed = readHorizontalControl();
      verticalSpeed = readVerticalControl();
      horizontalMotorSpeed = map(horizontalSpeed, -100, +100, -1 * MAX_HORIZONTAL_SPEED, MAX_HORIZONTAL_SPEED);
      verticalMotorSpeed = map(verticalSpeed, -100, +100, -1 * MAX_VERTICAL_SPEED, MAX_VERTICAL_SPEED);
    } else {
      horizontalSpeed = 0;
      verticalSpeed = 0;
      horizontalMotorSpeed = 0;
      verticalMotorSpeed = 0;
    }

    int horizontalMultiplier = 1;
    int verticalMultiplier = 1;
    if (horizontalMotorSpeed < 0) {
      horizontalMultiplier = -1; 
    }
    if (verticalMotorSpeed < 0) {
      verticalMultiplier = -1; 
    }

    if(horizontalMotorSpeed == 0 || horizontalSpeed == 0) {
      horizontalMotor->stopMove();
    } else {
      horizontalMotor->setSpeedInHz(abs(horizontalMotorSpeed));
      horizontalMotor->applySpeedAcceleration();
      if (horizontalMultiplier > 0) {
        horizontalMotor->runForward();
      } else {
        horizontalMotor->runBackward();
      }
    }

    if(verticalMotorSpeed == 0 || verticalSpeed == 0) {
      verticalMotor->stopMove();
    } else {
      verticalMotor->setSpeedInHz(abs(verticalMotorSpeed));
      verticalMotor->applySpeedAcceleration();
      if (verticalMultiplier > 0) {
        verticalMotor->runForward();
      } else {
        verticalMotor->runBackward();
      }
    }
  }
}

void measureBackslash() {
  activeMode = MODE_MEASURING_BACKSLASH;
  horizontalBackslashMoves = 0;
  verticalBackslashMoves = 0;
  lastHorizontalEncoderBackslashPosition = horizontalEncoderPosition;
  lastVerticalEncoderBackslashPosition = verticalEncoderPosition;
  lastHorizontalEncoderBackslashPosition = horizontalEncoderPosition;
  lastVerticalEncoderBackslashPosition = verticalEncoderPosition;
  if (lastHorizontalEncoderMoveDirection == 0) {
    horizontalBackslashDirection = 1;
  } else {
    lastHorizontalEncoderMoveDirection = -1;
  }
}

void moveMeasuringBackslash(
  FastAccelStepper *motor, 
  long& lastEncoderBackslashPosition, 
  long encoderPosition, 
  int& backslashFinished, 
  int& backslashMoveCount, 
  int& backslashMoveDirection, 
  int& encoderMoveDirection, 
  long& backlashValue) {
  //
  
  Serial.print("Backslash parameters (");
  Serial.print("encoderPosition: ");
  Serial.print(encoderPosition);
  Serial.print(", lastEncoderBackslashPosition: ");
  Serial.print(lastEncoderBackslashPosition);
  Serial.print(", backslashFinished: ");
  Serial.print(backslashFinished);
  Serial.print(", backslashMoveCount: ");
  Serial.print(backslashMoveCount);
  Serial.print(", backslashMoveDirection: ");
  Serial.print(backslashMoveDirection);
  Serial.print(", encoderMoveDirection: ");
  Serial.print(encoderMoveDirection);
  Serial.print(", backlashValue: ");
  Serial.print(backlashValue);
  Serial.print(")");
  Serial.println();
  
  // Still moving in the same direction
  if (backslashMoveDirection != encoderMoveDirection) {
    Serial.print("Backslash different direction... ");
    if (abs(lastHorizontalEncoderBackslashPosition - horizontalEncoderPosition) > 10) {
      motor->stopMove();
      backslashMoveDirection = encoderMoveDirection * -1;
    } else {
      lastHorizontalEncoderBackslashPosition = horizontalEncoderPosition;
      motor->setSpeedInHz(4000);
      motor->applySpeedAcceleration();
      if (backslashMoveDirection > 0) {
        motor->runForward();
      } else {
        motor->runBackward();
      }
      long delta = abs(encoderPosition - lastHorizontalEncoderBackslashPosition);
      if (delta > backlashValue) {
        Serial.print("New delta is higher than backslash value");
        Serial.print(", delta: ");
        Serial.print(delta);
        Serial.print(", backlashValue: ");
        Serial.print(backlashValue);
        Serial.println();
        backlashValue = delta;
      }
    }
  } else {
    Serial.println("Backslash same direction..");
    if (abs(lastHorizontalEncoderBackslashPosition - horizontalEncoderPosition) > 10) {
      motor->stopMove();
      backslashMoveCount++;
      if (backslashMoveCount > 2) {
        Serial.print("Backslash measure finished");
        Serial.print(", backlashValue: ");
        Serial.print(backlashValue);
        Serial.println();
        backslashFinished = 1;
      } else {
        Serial.println("Backslash measure inverting");
        backslashMoveDirection = encoderMoveDirection * -1;
      }
    } else {
      Serial.print("Same direction, no difference, keep moving");
      motor->setSpeedInHz(4000);
      motor->applySpeedAcceleration();
      if (backslashMoveDirection > 0) {
        motor->runForward();
      } else {
        motor->runBackward();
      }
    }
  }
}