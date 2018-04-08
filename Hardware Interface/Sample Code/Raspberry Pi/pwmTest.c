#include <wiringPi.h>
#include <stdio.h>

#define dirPin 0
#define pwmPin 1

#define STEP_SIZE 64

double MAX_SPEED = 1024;

int cycle = 0;
int speed = 0;

double percent = 0;

int main(int argc, char* argv) {

	wiringPiSetup();

	//Initialize Pin Modes
	pinMode(pwmPin, PWM_OUTPUT);
	pinMode(dirPin, OUTPUT);

	printf("speeding up...\n");
	while(cycle == 0) {

		percent = (double)speed / MAX_SPEED * 100.0;

		printf("speed = %d\n", speed);

		pwmWrite(pwmPin, speed);

		speed += STEP_SIZE;
		delay(1000);
		if(speed >= MAX_SPEED) {
			cycle = 1;
		}
	}

	printf("slowing down...\n");
	while(cycle == 1) {

		percent = (double)speed / MAX_SPEED * 100.0;

		printf("speed = %d\n", speed);

		pwmWrite(pwmPin, speed);

		speed -= STEP_SIZE;
		delay(1000);
		if(speed < 0) {
			cycle = 2;
		}
	}

	speed = 0;

	digitalWrite(dirPin, 1);

	printf("speeding up (reverse)...\n");

	while(cycle == 2) {

		percent = (double)speed / MAX_SPEED * 100.0;

		printf("speed = %d\n", speed);

		pwmWrite(pwmPin, speed);

		speed += STEP_SIZE;
		delay(1000);
		if(speed >= MAX_SPEED) {
			cycle = 3;
		}
	}

	printf("slowing down (reverse)...\n");
	while(cycle == 3) {

		percent = (double)speed / MAX_SPEED * 100.0;

		printf("speed = %d\n", speed);

		pwmWrite(pwmPin, speed);

		speed -= STEP_SIZE;
		delay(1000);
		if(speed < 0) {
			cycle = 4;
		}
	}

	digitalWrite(dirPin, 1);

	return 0;
}
