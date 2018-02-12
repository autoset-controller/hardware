#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define C_TYPE 0 //-1 = in-place rotation (-1:1), 0 = tight circle (0:1), 1 = straight line (1:1)

#define debug 1
#define DEBUG(args ...) if(debug) { printf(args); }

//system info
double TICKS_PER_CYCLE = 4096;
double radius = .2; //radius of encoder wheels (m)
double base = .5; //distance from encoder wheels to center (m)

//pos info
double dR;
double dL;

double x, y, theta; //current position (m, m, rad)
double dx, dy, dtheta; //change in position (m, m, rad)
double checksum;

char x_str[128];
char y_str[128];
char theta_str[128];
char checksum_str[128];

//setup info
int serialPort;
int update;

//Test program that send the position of a platform driving in circles.
int main(int argc, char* argv)  {

	DEBUG("Encoder Test:\n");

	wiringPiSetup();
	serialPort = serialOpen("/dev/ttyUSB0", 38400);

	x = 0; y = 0; theta = 0;
	dx = 0; dy = 0; dtheta = 0;

	while(1) {
		//for testing purposes, assume that position is updated every cycle.
		update = 1;
		//Check for serial data
		if(serialDataAvail(serialPort) > 0) {
			while(serialDataAvail(serialPort) > 0) {
				DEBUG("%c", (char)serialGetchar(serialPort));
			}
		}
		//update change in encoder position
		dR = 64 / TICKS_PER_CYCLE;
		dL = 64 * C_TYPE / TICKS_PER_CYCLE;

		//DEBUG("%f, %f\n", dR, dL);

		//calculate change in x,y,theta
		if(update != 0) {
			dtheta = (radius / (2 * base)) * (dR - dL);
			theta = theta + dtheta;
			theta = fmod(theta, 2 * M_PI);

			// |x| = |x0| + |cos(theta) cos(theta)| * |dR| = |x0| + r/2 * |cos(theta)*dR + cos(theta) * dL|
			// |y|   |y0|   |sin(theta) sin(theta)|   |dL|   |y0| +       |sin(theta)*dR + sin(theta) * dL|

			// x = x0 + r/2 * cos(theta) * (dR + dL)
			// y = y0 + r/2 * sin(theta) * (dR + dL)

			x = x + radius / 2 * cos(theta) * (dR + dL);
			y = y + radius / 2 * sin(theta) * (dR + dL);

			checksum = theta + x;
			checksum = checksum + y;

			update = 0;

			//format position data for serial output
			sprintf(x_str, "%f", x);
			sprintf(y_str, "%f", y);
			sprintf(theta_str, "%f", theta);
			sprintf(checksum_str, "%f", checksum);

			char packet[128];
			sprintf(packet, "1234|0|%s|%s|%s|%s\n", x_str, y_str, theta_str, checksum_str);

			DEBUG("sending packet: %s", packet);
			serialPuts(serialPort, packet);
			//delay(500);
		}
	}

	return 0;
}
