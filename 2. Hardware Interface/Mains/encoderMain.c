#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define debug 1
#define DEBUG(args ...) if(debug) { printf(args); }

//pin values
#define phaseAPinR 0
#define phaseBPinR 1
#define phaseAPinL 2
#define phaseBPinL 3

//system info
double TICKS_PER_CYCLE = 4096;
double radius = .25 * .3048; //radius of encoder wheels (m)
double base = 1 * .3048; //distance from encoder wheels to center (m)

//interrupt info
double dR;
double dL;

int A_L, B_L, A_R, B_R;

//pos info
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

//INTERRUPT FUNCTIONS
void phaseA_R() {
	A_R = digitalRead(phaseAPinR);
	B_R = digitalRead(phaseBPinR);

	/*State options for phase A interrupt:
	00 = CCW
	01 = CW
	10 = CW
	11 = CCW
	*/

	if(A_R+B_R == 1) {
		dR += 1; //Move position clockwise
	}
	else {
		dR += TICKS_PER_CYCLE - 1; //Move position counterclockwise
	}

	dR = fmod(dR, TICKS_PER_CYCLE);
	update++;
	//printf("pos = %d\n", pos);
}

void phaseB_R() {
	A_R = digitalRead(phaseAPinR);
	B_R = digitalRead(phaseBPinR);

	/*State options for phase A interrupt:
	00 = CW
	01 = CCW
	10 = CCW
	11 = CW
	*/

	if(A_R+B_R != 1) {
		dR += 1; //Move position clockwise
	}
	else {
		dR += TICKS_PER_CYCLE - 1; //Move position counterclockwise
	}

	dR = fmod(dR, TICKS_PER_CYCLE);
	update++;
	//printf("pos = %d\n", pos);
}

void phaseA_L() {
	A_L = digitalRead(phaseAPinL);
	B_L = digitalRead(phaseBPinL);

	/*State options for phase A interrupt:
	00 = CCW
	01 = CW
	10 = CW
	11 = CCW
	*/

	if(A_L+B_L == 1) {
		dL += 1; //Move position clockwise
	}
	else {
		dL += TICKS_PER_CYCLE - 1; //Move position counterclockwise
	}

	dL = fmod(dL, TICKS_PER_CYCLE);
	update++;
	//printf("pos = %d\n", pos);
}

void phaseB_L() {
	A_L = digitalRead(phaseAPinL);
	B_L = digitalRead(phaseBPinL);

	/*State options for phase A interrupt:
	00 = CW
	01 = CCW
	10 = CCW
	11 = CW
	*/

	if(A_L+B_L != 1) {
		dL += 1; //Move position clockwise
	}
	else {
		dL += TICKS_PER_CYCLE - 1; //Move position counterclockwise
	}

	dL = fmod(dL, TICKS_PER_CYCLE);
	update++;
	//printf("pos = %d\n", pos);
}

//Test program that send the position of a platform driving in circles.
int main(int argc, char* argv)  {

	DEBUG("Encoder Test:\n");

	wiringPiSetup();
	serialPort = serialOpen("/dev/ttyUSB0", 38400);

	//Initialize encoder pins
	pinMode(phaseAPinL, INPUT);
	pinMode(phaseBPinL, INPUT);
	pinMode(phaseAPinR, INPUT);
	pinMode(phaseBPinR, INPUT);

	//Initialize function for encoder interrupts
	int phaseAFuncR = wiringPiISR(phaseAPinR, INT_EDGE_BOTH, phaseA_R);
	int phaseBFuncR = wiringPiISR(phaseBPinR, INT_EDGE_BOTH, phaseB_R);
	int phaseAFuncL = wiringPiISR(phaseAPinL, INT_EDGE_BOTH, phaseA_L);
	int phaseBFuncL = wiringPiISR(phaseBPinL, INT_EDGE_BOTH, phaseB_L);

	x = 0; y = 0; theta = 0;
	dx = 0; dy = 0; dtheta = 0;

	while(1) {
		//Check for serial data
		if(serialDataAvail(serialPort) > 0) {
			while(serialDataAvail(serialPort) > 0) {
				DEBUG("%c", (char)serialGetchar(serialPort));
			}
		}
		//DEBUG("%f, %f\n", dR, dL);

		//calculate change in x,y,theta
		if(update != 0) {
			DEBUG("updates: %d\n", update);
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
			dR = 0;
			dL = 0;

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
