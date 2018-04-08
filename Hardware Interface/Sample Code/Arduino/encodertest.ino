const byte ccwLedPin = 7;
const byte cwLedPin = 6;
const byte posLedPin = 5;

const byte phaseA = 3;
const byte phaseB = 2;

volatile double pos = 0; // can be values from 0-1023
volatile int ccw = LOW; // if encoder is being rotated counter-clockwise
volatile int cw = LOW; // if encoder is being rotated clockwise

volatile byte stateA = LOW; // state of phaseA
volatile byte stateB = LOW; // state of phaseB

void setup() {
  // put your setup code here, to run once:
  pinMode(ccwLedPin, OUTPUT);
  pinMode(cwLedPin, OUTPUT);
  pinMode(posLedPin, OUTPUT);

  pinMode(phaseA, INPUT);
  pinMode(phaseB, INPUT);

  attachInterrupt(digitalPinToInterrupt(phaseA), incrementA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(phaseB), incrementB, CHANGE);

  Serial.begin(9600);
}

void loop() {
  digitalWrite(ccwLedPin, ccw);
  digitalWrite(cwLedPin, cw);
  int displayPos = pos * (255.0 / 4096.0);
  analogWrite(posLedPin, displayPos);
}

void incrementA() {
  stateA = digitalRead(phaseA);
  stateB = digitalRead(phaseB);

  //get direction
  if (stateA == LOW) {
    if (stateB == LOW) { // 00A = CCW
      cw = LOW;
      ccw = HIGH;
    }
    else {              // 01A = CW
      cw = HIGH;
      ccw = LOW;
    }
  }
  else {
    if (stateB == LOW) { // 10A = CW
      cw = HIGH;
      ccw = LOW;
    }
    else {              // 11A = CCW
      cw = LOW;
      ccw = HIGH;
    }
  }

  //update pos
  if (cw == HIGH) { // if moving cw, increment pos between 0-4095
    if (pos == 4095) {
      pos = 0;
    }
    else {
      pos += 1;
    }
  }
  else {
    if (pos == 0) { // if moving ccw, decrement pos between 0-4095
      pos = 4095;
    }
    else {
      pos -= 1;
    }
  }
}

void incrementB() {
  stateA = digitalRead(phaseA);
  stateB = digitalRead(phaseB);

  //get direction
  if (stateA == LOW) {
    if (stateB == LOW) { // 00B = CW
      cw = HIGH;
      ccw = LOW;
    }
    else {              // 01B = CCW
      cw = LOW;
      ccw = HIGH;
    }
  }
  else {
    if (stateB == LOW) { // 10B = CCW
      cw = LOW;
      ccw = HIGH;
    }
    else {              // 11B = CW
      cw = HIGH;
      ccw = LOW;
    }
  }

  //update pos
  if (cw == HIGH) { // if moving cw, increment pos between 0-4095
    if (pos == 4095) {
      pos = 0;
    }
    else {
      pos += 1;
    }
  }
  else {
    if (pos == 0) { // if moving ccw, decrement pos between 0-4095
      pos = 4095;
    }
    else {
      pos -= 1;
    }
  }
}
