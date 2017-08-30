int trigPin = 10;
int echoPin = 9;

//Ultrasonic Value Struct
struct ultrareturn {
  int ok;
  double distance;
};

struct ultrareturn okDistance;
struct ultrareturn lastOkDistance;
long timeDiff; //time diff between triggered sound and recieved echo (in microseconds)
double distance; //distance in m

int numWarnings = 0;

//CONSTANTS
double threshold = .1;


void setup() {
  lastOkDistance.ok = 0;
  lastOkDistance.distance = 0.0;
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  okDistance = checkUltrasonic();
  if(okDistance.ok == 1) {
    Serial.print("OK: ");
    Serial.println(okDistance.distance);
    lastOkDistance.ok = okDistance.ok;
    lastOkDistance.distance = okDistance.distance;
    numWarnings = 0;
  }
  else if(okDistance.ok == 0) {
    Serial.print("WARNING: ");
    Serial.println(okDistance.distance);
    lastOkDistance.ok = okDistance.ok;
    lastOkDistance.distance = okDistance.distance;
    numWarnings += 1;
    if(numWarnings > 20) {
      Serial.println("HALT");
      delay(1000);
    }
  }
  else {
    if(lastOkDistance.ok == 1) {
      Serial.print("OK: ");
      Serial.println(okDistance.distance);
    }
    else {
      Serial.print("WARNING: ");
      Serial.println(okDistance.distance);
      numWarnings += 1;
      if(numWarnings > 20) {
        Serial.println("HALT");
        delay(1000);
      }
    }
  }
}

struct ultrareturn checkUltrasonic() {
  struct ultrareturn okDistance;
  
  //send out 10-microsecond signal
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  
  //get time to echo
  timeDiff = pulseIn(echoPin, HIGH); 
  //get distance to object
  distance = timeDiff * .00034 / 2.000; 
  okDistance.distance = distance;

  //filter out signal blockage
  if(distance <= 0.0 || distance > 20.0) {
    //no distance implies electrical interruption, high distance implies echo is unrecieved
    okDistance.ok = -1; //send out 'unknown' signal
  }
  //check for excess distance
  else if(distance > .1) {
    //distance between sensor and ground should not exceed 10cm.
    okDistance.ok = 0;
  }
  //otherwise, distance is acceptable.
  else {
    okDistance.ok = 1;
  }
  return okDistance;
}

