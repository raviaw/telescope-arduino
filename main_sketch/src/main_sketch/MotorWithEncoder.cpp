#include <Arduino.h>
#include "MotorWithEncoder.h"

MotorWithEncoder::MotorWithEncoder(HardwareSerial* encoderSerialPort, FastAccelStepper* stepper, int motorStepPin, int motorDirectionPin) {
  _encoderSerialPort = encoderSerialPort;
  _stepper = stepper;
  _motorStepPin = motorStepPin;
  _motorDirectionPin = motorDirectionPin;
}

void MotorWithEncoder::updateEncoderFromSerial() {
  //
  // Reads the buffer from the serial port
  while(encoderSerialPort->available())
  { 
    int nextChar = encoderSerialPort->read();
    if (nextChar != -1) { 
      // Serial.print("Available encoder: ");
      // Serial.println(nextChar);
      if ((nextChar & 0x10) == 0x10) {
        if (verticalEncoderBufferPointer >= 7) {
          // encoderBufferPointer++; // To do the correct math
          long n8 = nextChar;
          long n7 = verticalEncoderBuffer[verticalEncoderBufferPointer - 1];
          long n6 = verticalEncoderBuffer[verticalEncoderBufferPointer - 2];
          long n5 = verticalEncoderBuffer[verticalEncoderBufferPointer - 3];
          long n4 = verticalEncoderBuffer[verticalEncoderBufferPointer - 4];
          long n3 = verticalEncoderBuffer[verticalEncoderBufferPointer - 5];
          long n2 = verticalEncoderBuffer[verticalEncoderBufferPointer - 6];
          long n1 = verticalEncoderBuffer[verticalEncoderBufferPointer - 7];
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
            setVerticalEncoderPosition(sum);
          }
          // Serial.print("Encoder bytesc: ");
          // Serial.print(n1);
          // Serial.print(".");
          // Serial.print(c1);
          // Serial.print(", ");
          // Serial.print(n2);
          // Serial.print(".");
          // Serial.print(c2);
          // Serial.print(", ");
          // Serial.print(n3);
          // Serial.print(".");
          // Serial.print(c3);
          // Serial.print(", ");
          // Serial.print(n4);
          // Serial.print(".");
          // Serial.print(c4);
          // Serial.print(", ");
          // Serial.print(n5);
          // Serial.print(".");
          // Serial.print(c5);
          // Serial.print(", ");
          // Serial.print(n6);
          // Serial.print(".");
          // Serial.print(c6);
          // Serial.print(", ");
          // Serial.print(n7);
          // Serial.print(".");
          // Serial.print(c7);
          // Serial.print(", ");
          // Serial.print(n8);
          // Serial.print(".");
          // Serial.print(c8);
          // Serial.print(", ");
          // Serial.print(sum);
          // Serial.print(", ");
          // Serial.print(encoderPosition);
          // Serial.println();
        }
        verticalEncoderBufferPointer = 0;
      } else if(verticalEncoderBufferPointer < sizeof(verticalEncoderBuffer) -1) {
        verticalEncoderBuffer[verticalEncoderBufferPointer] = nextChar;
        verticalEncoderBufferPointer++;
      } else {
        verticalEncoderBufferPointer = 0;
      }
    }
  }
}