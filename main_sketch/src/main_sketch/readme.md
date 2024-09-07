# Pre-defined values

Horizontal Encoder Start: -24188 21488

45000 max - round up to 60000 travel limit

Motor: -350000 to 509116

860000 motor

Motor increases 19 times faster

Limite 1: AccX: 1.02

Limite 2: AccX: 0.44/

Old code:

```
//void moveVerticalMotorTo(long newPosition) {
//  verticalMotor->moveTo(newPosition);
//}


//void measureBackslash() {
//  activeMode = MODE_MEASURING_BACKSLASH;
//  horizontalBackslashMoves = 0;
//  verticalBackslashMoves = 0;
//  lastHorizontalEncoderBackslashPosition = horizontalEncoderPosition;
//  lastVerticalEncoderBackslashPosition = verticalEncoderPosition;
//  lastHorizontalEncoderBackslashPosition = horizontalEncoderPosition;
//  lastVerticalEncoderBackslashPosition = verticalEncoderPosition;
//  if (lastHorizontalEncoderMoveDirection == 0) {
//    horizontalBackslashDirection = 1;
//  } else {
//    lastHorizontalEncoderMoveDirection = -1;
//  }
//}
//
//void moveMeasuringBackslash(
//  FastAccelStepper *motor, 
//  long& lastEncoderBackslashPosition, 
//  long encoderPosition, 
//  int& backslashFinished, 
//  int& backslashMoveCount, 
//  int& backslashMoveDirection, 
//  int& encoderMoveDirection, 
//  long& backlashValue) {
//  //
//  
//  Serial.print("Backslash parameters (");
//  Serial.print("encoderPosition: ");
//  Serial.print(encoderPosition);
//  Serial.print(", lastEncoderBackslashPosition: ");
//  Serial.print(lastEncoderBackslashPosition);
//  Serial.print(", backslashFinished: ");
//  Serial.print(backslashFinished);
//  Serial.print(", backslashMoveCount: ");
//  Serial.print(backslashMoveCount);
//  Serial.print(", backslashMoveDirection: ");
//  Serial.print(backslashMoveDirection);
//  Serial.print(", encoderMoveDirection: ");
//  Serial.print(encoderMoveDirection);
//  Serial.print(", backlashValue: ");
//  Serial.print(backlashValue);
//  Serial.print(")");
//  Serial.println();
//  
//  // Still moving in the same direction
//  if (backslashMoveDirection != encoderMoveDirection) {
//    Serial.print("Backslash different direction... ");
//    if (abs(lastHorizontalEncoderBackslashPosition - horizontalEncoderPosition) > 10) {
//      motor->stopMove();
//      backslashMoveDirection = encoderMoveDirection * -1;
//    } else {
//      lastHorizontalEncoderBackslashPosition = horizontalEncoderPosition;
//      motor->setSpeedInHz(4000);
//      motor->applySpeedAcceleration();
//      if (backslashMoveDirection > 0) {
//        motor->runForward();
//      } else {
//        motor->runBackward();
//      }
//      long delta = abs(encoderPosition - lastHorizontalEncoderBackslashPosition);
//      if (delta > backlashValue) {
//        Serial.print("New delta is higher than backslash value");
//        Serial.print(", delta: ");
//        Serial.print(delta);
//        Serial.print(", backlashValue: ");
//        Serial.print(backlashValue);
//        Serial.println();
//        backlashValue = delta;
//      }
//    }
//  } else {
//    Serial.println("Backslash same direction..");
//    if (abs(lastHorizontalEncoderBackslashPosition - horizontalEncoderPosition) > 10) {
//      motor->stopMove();
//      backslashMoveCount++;
//      if (backslashMoveCount > 2) {
//        Serial.print("Backslash measure finished");
//        Serial.print(", backlashValue: ");
//        Serial.print(backlashValue);
//        Serial.println();
//        backslashFinished = 1;
//      } else {
//        Serial.println("Backslash measure inverting");
//        backslashMoveDirection = encoderMoveDirection * -1;
//      }
//    } else {
//      Serial.print("Same direction, no difference, keep moving");
//      motor->setSpeedInHz(4000);
//      motor->applySpeedAcceleration();
//      if (backslashMoveDirection > 0) {
//        motor->runForward();
//      } else {
//        motor->runBackward();
//      }
//    }
//  }
//}

//  Serial.print("Motor: ");
//  Serial.print(motorNo);
//  Serial.print(", current: ");
//  Serial.print(currentEncoderPos);
//  Serial.print(", target: ");
//  Serial.print(targetEncoderPos);
//  Serial.print(", diff: ");
//  Serial.print(diff);
//  Serial.print(", encoderDirection: ");
//  Serial.print(encoderDirection);
//  Serial.print(", motorDirection: ");
//  Serial.print(motorDirection);
//  Serial.print(", currentSpeed: ");
//  Serial.print(currentSpeed);
//  Serial.print(", backSlashing: ");
//  Serial.print(backSlashing);
//  Serial.print(", backslashLeft: ");
//  Serial.print(backslashLeft);
//  Serial.print(", motorStartBackslash: ");
//  Serial.print(motorStartBackslash);
//  Serial.print(", standardBackslash: ");
//  Serial.print(standardBackslash);
//  Serial.println();
//  
//  if (encoderDirection != motorDirection) {
//    if (!backSlashing) {
//      backslashLeft = standardBackslash;
//      motorStartBackslash = motor->getCurrentPosition();
//      backSlashing = 1;
//      motor->setSpeedInHz(7000);
//    } else {
//      long motorDiff = abs(motor->getCurrentPosition() - motorStartBackslash);
//      backslashLeft -= motorDiff;
//      if (backslashLeft > 20000) {
//        motor->setSpeedInHz(7000);
//      } else if(backslashLeft > 15000) {
//        motor->setSpeedInHz(6000);
//      } else if(backslashLeft > 15000) {
//        motor->setSpeedInHz(6000);
//      } else if(backslashLeft > 10000) {
//        motor->setSpeedInHz(3000);
//      } else if(backslashLeft > 5000) {
//        motor->setSpeedInHz(1000);
//      } else {
//        motor->setSpeedInHz(500);
//      }
//    }
//  } else {
//    backSlashing = 0;
//    if (diff < 10) {
//      motor->setSpeedInHz(500);
//    } else if (diff > 10 && diff < 20) {
//      motor->setSpeedInHz(1000);
//    } else {
//      motor->setSpeedInHz(2000);
//    }
//  } 
//  motor->applySpeedAcceleration();
//  if (encoderDirection > 0) {
//    motor->runForward();
//  } else {
//    motor->runBackward();
//  }

//
// Code to move coordinates
//

//      _coordinateSpeed = mapDouble(speed, -100, +100, -1, +1) / 500.0; 
//      long horizontalMotorDiff = abs(readMotorPosition) - newMotorHorizontalPos);
//      if (horizontalMotorDiff < MAX_HORIZONTAL_SPEED) {
//        ra += horizontalCoordinateSpeed;
//      }
//      long verticalMotorDiff = abs(readVerticalMotorPosition() - newMotorVerticalPos);
//      if (verticalMotorDiff < MAX_VERTICAL_SPEED) {
//        dec += verticalCoordinateSpeed;
//      }

```