#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdio.h>

int main(int argc, char* argv) {

	int serial = serialOpen("dev/AMA1", 9600);

	int baud = 170;

	int addr = 128;

	int comm = 0;

	int speed = 0;

	int cycle = 0;

	printf("establishing baud rate...\n");
	serialPutchar(serial, baud);
	delay(5000);

	printf("speeding up...\n");
	while(cycle == 0) {
		serialPutchar(serial, (char)addr);
		serialPutchar(serial, (char)comm);
		serialPutchar(serial, (char)speed);
		int valid = (addr + comm + speed) & 0b0111111;
		serialPutchar(serial, (char)valid);

		speed += 4;
		delay(1000);
		if(speed >= 64) {
			cycle = 1;
		}
	}

	printf("slowing down...\n");
	while(cycle == 1) {
		serialPutchar(serial, (char)addr);
		serialPutchar(serial, (char)comm);
		serialPutchar(serial, (char)speed);
		int valid = (addr + comm + speed) & 0b0111111;
		serialPutchar(serial, (char)valid);

		speed -= 4;
		delay(1000);
		if(speed < 0) {
			cycle = 2;
		}
	}

	return 0;
}
