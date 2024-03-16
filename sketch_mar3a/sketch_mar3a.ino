// Define stepper motor connections:
#define dirPin 3
#define stepPin 4

void setup () {  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  // Declare pins as output:
  pinMode ( stepPin, OUTPUT ) ;
  pinMode ( dirPin, OUTPUT ) ;
  // Set the CW/CCW rotation direction:
  digitalWrite ( dirPin, HIGH ) ;
}
void loop () {  
  digitalWrite ( dirPin, HIGH ) ;

  // 500 us delay = 20 seconds for 90 degrees
  // 180 degrees per 12 hours
  // 15 degrees per hour
  // 0.25 degrees per minute
  // 0.08333 degrees every 20 seconds
  // 1080 
  // How much time should I take to turn 90 degrees? 
  // 6 hours
  // 1080 times slower?

  // 500 us = 20
  // X us = 0.08333
  // X = (500 * 0.08333) / 20
  
  // 500 us for 20 seconds
  // 540000 us for 6 hours
  int value = analogRead(A0);
  int testValue = 0;
  Serial.println();
  Serial.print(value);
  Serial.print(",");
  if (value < 512) {
    Serial.print("HIGH");
    Serial.print(",");
    digitalWrite ( dirPin, HIGH ) ;
    testValue = 512 - value;
  } else {
    Serial.print("LOW");
    Serial.print(",");
    digitalWrite ( dirPin, LOW ) ;
    testValue = value - 512;
  }
  Serial.print(testValue);
  Serial.print(",");
  int compareValue = testValue / 80;
  Serial.print(compareValue);
  Serial.print(",");

  int pulseWidthMicros = 20;

  for (int i = 0; i < 10; i++) {
    // Set the CW/CCW rotation direction:
    if (compareValue == 0) {
      digitalWrite ( stepPin, HIGH ) ;
      delay ( 100 ) ;
      digitalWrite ( stepPin, LOW ) ;
      delay ( 100 ) ;
    } else if (compareValue == 1) {
      digitalWrite ( stepPin, HIGH ) ;
      delay ( 10 ) ;
      digitalWrite ( stepPin, LOW ) ;
      delay ( 10 ) ;
    } else if (compareValue == 2) {
      digitalWrite ( stepPin, HIGH ) ;
      delayMicroseconds ( 5000 ) ;
      digitalWrite ( stepPin, LOW ) ;
      delayMicroseconds ( 5000 ) ;
    } else if (compareValue == 3) {
      digitalWrite ( stepPin, HIGH ) ;
      delayMicroseconds ( 2000 ) ;
      digitalWrite ( stepPin, LOW ) ;
      delayMicroseconds ( 2000 ) ;
    } else if (compareValue == 4) {
      digitalWrite ( stepPin, HIGH ) ;
      delayMicroseconds ( 1000 ) ;
      digitalWrite ( stepPin, LOW ) ;
      delayMicroseconds ( 1000 ) ;
    } else if (compareValue == 5) {
      digitalWrite ( stepPin, HIGH ) ;
      delayMicroseconds ( 500 ) ;
      digitalWrite ( stepPin, LOW ) ;
      delayMicroseconds ( 500 ) ;
    } else {
      digitalWrite ( stepPin, HIGH ) ;
      delayMicroseconds ( 200 ) ;
      digitalWrite ( stepPin, LOW ) ;
      delayMicroseconds ( 200 ) ;
    }
  }
}