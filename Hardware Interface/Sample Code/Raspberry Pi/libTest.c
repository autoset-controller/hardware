#include <wiringPi.h>
#include <stdio.h>

int pin = 1;

int main(int argc, char* argv)  {

	printf("WiringPi Test:\n");

	wiringPiSetup();
	printf("Testing pin %d...\n", pin);
	pinMode(pin, OUTPUT);
	int count = 0;
	while(count < 5) {
		digitalWrite(pin, HIGH);
		delay(2000);
		digitalWrite(pin, LOW);
		delay(2000);
		count++;
	}
	return 0;
}
