//Arduino serial comunication happens through the USB or on the pins TX and RX
//including this file allows us to create another Serial port using pins 6 and 7
#include <SoftwareSerial.h> 
#define Rx    6                // DOUT to pin 6
#define Tx    7                // DIN to pin 7
SoftwareSerial Xbee (Rx, Tx);
 
//Some variables for the LEDs and reading the Serial
char incoming ='empty';

const byte phaseA = 3;
const byte phaseB = 2;

const int ticksPerCycle = 4096;
int ticks = 0;

String radioHandle = "A";

volatile int pos = 0; // can be values from 0-4095
volatile int ccw = LOW; // if encoder is being rotated counter-clockwise
volatile int cw = LOW; // if encoder is being rotated clockwise

volatile int stateA = 0; // state of phaseA
volatile int stateB = 0; // state of phaseB

volatile int posChanged = 1;

int counter = 0;
 
void setup(){
  // put your setup code here, to run once:
  pinMode(phaseA, INPUT);
  pinMode(phaseB, INPUT);
  
  Xbee.begin(9600);
  Serial.begin(9600);

  Xbee.write("Beginning Transmission\n");
  Serial.write("Beginning Transmission\n");

  attachInterrupt(digitalPinToInterrupt(phaseA), incrementA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(phaseB), incrementB, CHANGE);
  
}
   
void loop(){
  ticks += 1;
  if(ticks > 16 && posChanged == 1){ //send approx. 60 lines/second
    String output = radioHandle + ": " + (String)pos;
    Xbee.println(output);
    ticks = 0;
    posChanged = 0;
  }
}

void incrementA() {

  int state = (int)digitalRead(phaseA) + (int)digitalRead(phaseB);
  //00A = 0 = CCW
  //01A = 1 = CW
  //10A = 1 = CW
  //11A = 2 = CCW

  if(state == 1) { //01A || 10A == CW
    pos += 1;
  }
  else { //00A || 11A == CCW
    pos += 4095; //equivalent to -= 1, but maintains positive value
  }
  pos = pos % ticksPerCycle;
  posChanged = 1;
}

void incrementB() {
  
  int state = (int)digitalRead(phaseA) + (int)digitalRead(phaseB);
  //00B = 0 = CW
  //01B = 1 = CCW
  //10B = 1 = CCW
  //11B = 2 = CW

  if(state == 1) { //01B || 10B == CCW
    pos += 4095; //equivalent to -= 1, but maintains positive value
  }
  else { //00B || 11B == CW
    pos += 1;
  }
  pos = pos % ticksPerCycle;
  posChanged = 1;
}
