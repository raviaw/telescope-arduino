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
  moveMotorTrackingWithEncoder(horizontalMotor, readHorizontalEncoderPosition(), newEncoderHorizontalPos, readHorizontalMotorPosition(), lastHorizontalMotorPosition);
  lastHorizontalMotorPosition = currentHorizontalMotorPosition;

  long currentVerticalMotorPosition = readVerticalMotorPosition();
  moveMotorTrackingWithEncoder(verticalMotor, readVerticalEncoderPosition(), newEncoderVerticalPos, readVerticalMotorPosition(), lastVerticalMotorPosition);
  lastVerticalMotorPosition = currentVerticalMotorPosition;
}

void moveMotorTrackingWithEncoder(FastAccelStepper *motor, long currentEncoderPos, long targetEncoderPos, long currentMotorPos, long lastMotorPos) {
  if (abs(currentEncoderPos - targetEncoderPos) < 20) {
    // Nothing to do
    return;
  }
  int encoderDirection = currentEncoderPos < targetEncoderPos ? +1 : -1;
  int motorDirection = currentMotorPos < lastMotorPos ? +1 : -1;
  motor->move(encoderDirection * 20);
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
//   horizontalMotor->move(horizontalMotorSpeed * horizontalMultiplier);
//   verticalMotor->move(verticalMotorSpeed * verticalMultiplier);
}