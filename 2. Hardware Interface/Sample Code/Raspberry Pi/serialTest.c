#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define TICKS_PER_CYCLE 4096

int phaseAPin = 0;
int phaseBPin = 1;

int pos;
int A;
int B;

int update = 0;
int serialPort;

void phaseA() {
	A = digitalRead(phaseAPin);
	B = digitalRead(phaseBPin);

	/*State options for phase A interrupt:
	00 = CCW
	01 = CW
	10 = CW
	11 = CCW
	*/

	if(A+B == 1) {
		pos += 1; //Move position clockwise
	}
	else {
		pos += TICKS_PER_CYCLE - 1; //Move position counterclockwise
	}

	pos = pos % TICKS_PER_CYCLE;
	update++;
	//printf("pos = %d\n", pos);
}

void phaseB() {
	A = digitalRead(phaseAPin);
	B = digitalRead(phaseBPin);

	/*State options for phase A interrupt:
	00 = CW
	01 = CCW
	10 = CCW
	11 = CW
	*/

	if(A+B != 1) {
		pos += 1; //Move position clockwise
	}
	else {
		pos += TICKS_PER_CYCLE - 1; //Move position counterclockwise
	}

	pos = pos % TICKS_PER_CYCLE;
	update++;
	//printf("pos = %d\n", pos);
}

int main(int argc, char* argv)  {

	printf("Encoder Test:\n");

	wiringPiSetup();
	serialPort = serialOpen("/dev/ttyUSB0", 38400);

	//Initialize pins
	phaseAPin = 0;
	phaseBPin = 1;

	pinMode(phaseAPin, INPUT);
	pinMode(phaseBPin, INPUT);

	//Initialize values
	pos = 0;

	//Initialize serial
	/*
	int serial = serialOpen("dev/ttyAMA0", 152000);

	if(serial != -1) {
		printf("serial output initialized.\n");
	}
	else {
		printf("serial output failed.\n");
	}
	*/
	//Initialize function for encoder interrupts
	int phaseAFunc = wiringPiISR(phaseAPin, INT_EDGE_BOTH, phaseA);
	int phaseBFunc = wiringPiISR(phaseBPin, INT_EDGE_BOTH, phaseB);

	while(1) {
		if(serialDataAvail(serialPort) > 0) {
			while(serialDataAvail(serialPort) > 0) {
				printf("%c", (char)serialGetchar(serialPort));
			}
			printf("\n");
		}
		if(update != 0) {
			serialPrintf(serialPort, "pos = %d\n", pos);
			printf("pos = %d\n", pos);
			update = 0;
		}
	}

	return 0;
}
