#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

//Serial Port Values
int serialPort;

int main(int argc, char* argv)  {

	wiringPiSetup();
	serialPort = serialOpen("/dev/ttyUSB0", 38400);

	//Initialize serial

	if(serialPort != -1) {
		printf("serial init successful.\n");
	}
	else {
		printf("serial init failed.\n");
	}
	serialFlush(serialPort);

	while(1) {
		if(serialDataAvail(serialPort) > 0) {
			while(serialDataAvail(serialPort) > 0) {
				char c = (char)serialGetchar(serialPort);
				printf("%c", c);
			}
		}
		delay(3000);
		char* egg = "egg\n";
		printf("Sending egg...\n");
		serialPuts(serialPort, egg);
	}

	return 0;
}
