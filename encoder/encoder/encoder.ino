#include <elapsedMillis.h>
#include <Encoder.h>

Encoder encoder(2, 3);
long lastValue = 0L;
long newValue;
int clock = -1;

elapsedMillis processTime;

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

// array to store binary number 
int binaryNumber[9]; 

void decToBinary(int n) 
{ 
  int useNumber = abs(n);
  binaryNumber[0] = 0;
  binaryNumber[1] = 0;
  binaryNumber[2] = 0;
  binaryNumber[3] = 0;
  binaryNumber[4] = 0;
  binaryNumber[5] = 0;
  binaryNumber[6] = 0;
  binaryNumber[7] = 0;
  // counter for binary array 
  int i = 0; 
  while (useNumber > 0 && i < 8) { 
    // storing remainder in binary array 
    binaryNumber[i] = useNumber % 2; 
    useNumber = useNumber / 2; 
    i++; 
  } 
  if (useNumber < 0) {
    binaryNumber[7] = 1;
  }
  if (i >= 8) {
    Serial.print("WILL OVERFLOW WITH ");
    Serial.print(n);
    Serial.println();
  }
} 


void setup() {
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  if (processTime > 250) {
    clock++;
    if (clock > 3) {
      clock = 0;
    }

    Serial.print("CYCLE");
    if (clock == 0) {
      Serial.print("[00], ");
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
    } else if (clock == 1) {
      Serial.print("[01], ");
      digitalWrite(4, HIGH);
      digitalWrite(5, LOW);
    } else if (clock == 2) {
      Serial.print("[10], ");
      digitalWrite(4, LOW);
      digitalWrite(5, HIGH);
    } else if (clock == 3) {
      Serial.print("[11], ");
      digitalWrite(4, HIGH);
      digitalWrite(5, HIGH);
    }

    newValue = encoder.read();
    long delta = newValue - lastValue;
    lastValue = newValue;
    decToBinary(delta);

    Serial.print("C: ");
    Serial.print(clock);
    Serial.print(".");
    Serial.print(lastValue);
    Serial.print(".");
    Serial.print(delta);
    Serial.print("[");
    for (int i = 0; i < 8; i++) {
      int number = binaryNumber[i];
      if (number == 0) {
        digitalWrite(6 + i, LOW);
        Serial.print("0");
      } else {
        digitalWrite(6 + i, HIGH);
        Serial.print("1");
      }
    }
    Serial.println("]");
    processTime = 0;
  }
}
