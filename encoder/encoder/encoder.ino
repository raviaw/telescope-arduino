#include <elapsedMillis.h>
#include <Encoder.h>

Encoder encoder(2, 3);
long newValue = 0;

elapsedMillis processTime;

void setup() {
  Serial.begin(57600);
}

void loop() {
  if (processTime > 100) {
    long newValue = encoder.read();
    // newValue += newEncoderValue;
    // newValue = 1500;
    long val1 = ((newValue & 0xF0000000) >> 28) | 0x80;
    long val2 = ((newValue & 0x0F000000) >> 24) | 0x40;
    long val3 = ((newValue & 0x00F00000) >> 20) | 0x40;
    long val4 = ((newValue & 0x000F0000) >> 16) | 0x40;
    long val5 = ((newValue & 0x0000F000) >> 12) | 0x40;
    long val6 = ((newValue & 0x00000F00) >> 8) | 0x40;
    long val7 = ((newValue & 0x000000F0) >> 4) | 0x20;
    long val8 = (newValue & 0x0000000F) | 0x10;

    // int encoderBuffer[] = { val1, val2, val3, val4, val5, val6, val7, val8 };
    // int encoderBufferPointer = 7;
    // long n8 = encoderBuffer[encoderBufferPointer];
    // long n7 = encoderBuffer[encoderBufferPointer - 1];
    // long n6 = encoderBuffer[encoderBufferPointer - 2];
    // long n5 = encoderBuffer[encoderBufferPointer - 3];
    // long n4 = encoderBuffer[encoderBufferPointer - 4];
    // long n3 = encoderBuffer[encoderBufferPointer - 5];
    // long n2 = encoderBuffer[encoderBufferPointer - 6];
    // long n1 = encoderBuffer[encoderBufferPointer - 7];
    // long sum =
    //     (((n1 & 0x0F) << 28) & 0xF0000000L) +
    //     (((n2 & 0x0F) << 24) & 0x0F000000L) +
    //     (((n3 & 0x0F) << 20) & 0x00F00000L) +
    //     (((n4 & 0x0F) << 16) & 0x000F0000L) +
    //     (((n5 & 0x0F) << 12) & 0x0000F000L) +
    //     (((n6 & 0x0F) << 8) & 0x00000F00L) +
    //     (((n7 & 0x0F) << 4) & 0x000000F0L) +
    //     ((n8 & 0x0F) & 0x0000000FL);
    // int c1 = (n1 & 0x80) == 0x80;
    // int c2 = (n2 & 0x40) == 0x40;
    // int c3 = (n3 & 0x40) == 0x40;
    // int c4 = (n4 & 0x40) == 0x40;
    // int c5 = (n5 & 0x40) == 0x40;
    // int c6 = (n6 & 0x40) == 0x40;
    // int c7 = (n7 & 0x20) == 0x20;
    // int c8 = (n8 & 0x10) == 0x10;

    // Serial.print(newValue & 0xF0000000L);
    // Serial.print(", ");
    // Serial.print(newValue & 0x0F000000L);
    // Serial.print(", ");
    // Serial.print(newValue & 0x00F00000L);
    // Serial.print(", ");
    // Serial.print(newValue & 0x000F0000L);
    // Serial.print(", ");
    // Serial.print(newValue & 0x0000F000L);
    // Serial.print(", ");
    // Serial.print(newValue & 0x00000F00L);
    // Serial.print(", ");
    // Serial.print(newValue & 0x000000F0L);
    // Serial.print(", ");
    // Serial.print(newValue & 0x0000000FL);
    // Serial.print(" - ");
    // Serial.print((long) newValue);
    // Serial.print(" > ");
    // Serial.print((long) sum);
    // Serial.print(", ");
    // Serial.print((long) val1);
    // Serial.print(", ");
    // Serial.print((long) val2);
    // Serial.print(", ");
    // Serial.print((long) val3);
    // Serial.print(", ");
    // Serial.print((long) val4);
    // Serial.print(", ");
    // Serial.print((long) val5);
    // Serial.print(", ");
    // Serial.print((long) val6);
    // Serial.print(", ");
    // Serial.print((long) val7);
    // Serial.print(", ");
    // Serial.print((long) val8);
    // Serial.println();
    Serial.write((byte) val1);
    Serial.write((byte) val2);
    Serial.write((byte) val3);
    Serial.write((byte) val4);
    Serial.write((byte) val5);
    Serial.write((byte) val6);
    Serial.write((byte) val7);
    Serial.write((byte) val8);
    processTime = 0;
  }
}
