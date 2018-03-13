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
#define XBEE_BAUD 57600
#define UPDATE_SIZE_MAX = 128;
#define UPDATE_SIZE_MIN = 16;
#define MATCH_LIMIT = 16;
double TICKS_PER_CYCLE = 4096;

int update_size = 64 //minimum # updates to trigger a packet send
int update_match = 0;
double radius = .25 * .3048; //radius of encoder wheels (m)
double base = 1 * .3048; //distance from encoder wheels to center (m)

//forward decl
void phaseA_R();
void phaseB_R();
void phaseA_L();
void phaseB_L();

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

//Test program that send the position of a platform driving in circles.
int main(int argc, char* argv)  {

	DEBUG("Encoder Test:\n");

	wiringPiSetup();
	serialPort = serialOpen("/dev/ttyUSB0", XBEE_BAUD);

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

	double dR_in, dL_in;
	int updates_in;

	int wait = 0;

	while(1) {
		//Check for serial data
		if(serialDataAvail(serialPort) > 0) {
			while(serialDataAvail(serialPort) > 0) {
				//TODO: Process serial input
				//SET_POSITION: Set x, y, and theta to incoming position data
				//MODIFY_ATTRIBUTE: Set key, platform, radius, or base to incoming data
				DEBUG("%c", (char)serialGetchar(serialPort));
			}
		}

		//calculate change in x,y,theta
		if(update >= update_size) {
			//DEBUG("updates: %d\n", update);
			//Process update and clear transfer values
			dR_in = dR;
			dL_in = dL;
			updates_in = update;

			dR = 0;
			dL = 0;
			update = 0;

			//interrupts can now safely modify these values
			DEBUG("%d: %f, %f\n", updates_in, dR_in, dL_in);

			//update update size to process updates efficiently
			if(update <= update_size) { // if updates were processed without delay
				update_match++;
				//if updates are consistently processed without delay, attempt to lower the update size
				if(update_match >= match_limit) {
					update_match = 0;
					if(update_size >= UPDATE_SIZE_MIN * 2) {
						update_size /= 2;
					}
					else if(update_size >= UPDATE_SIZE_MIN) {
						update_size -= 1;
					}
				}
			}
			else { //if updates were processed with some delay
				if(update_size <= UPDATE_SIZE_MAX / 2) {
					update_size *= 2;
				}
				else if(update_size <= UPDATE_SIZE_MAX) {
					update_size += 1;
				}
			}

			dR_in = dR_in * 2 * M_PI / TICKS_PER_CYCLE;
			dL_in = dL_in * 2 * M_PI / TICKS_PER_CYCLE;

			dtheta = (radius / (2 * base)) * (dR_in - dL_in);
			theta = theta + dtheta;
			theta = fmod(theta, 2 * M_PI);

			// |x| = |x0| + |cos(theta) cos(theta)| * |dR| = |x0| + r/2 * |cos(theta)*dR + cos(theta) * dL|
			// |y|   |y0|   |sin(theta) sin(theta)|   |dL|   |y0| +       |sin(theta)*dR + sin(theta) * dL|

			// x = x0 + r/2 * cos(theta) * (dR + dL)
			// y = y0 + r/2 * sin(theta) * (dR + dL)

			x = x + radius / 2 * cos(theta) * (dR_in + dL_in);
			y = y + radius / 2 * sin(theta) * (dR_in + dL_in);

			checksum = theta + x;
			checksum = checksum + y;

			//format position data for serial output
			sprintf(x_str, "%f", x);
			sprintf(y_str, "%f", y);
			sprintf(theta_str, "%f", theta);
			sprintf(checksum_str, "%f", checksum);

			char packet[128];
			sprintf(packet, "1234|0|%s|%s|%s|%s\n", x_str, y_str, theta_str, checksum_str);

			//DEBUG("sending packet: %s", packet);
			serialPuts(serialPort, packet);
		}
	}

	return 0;
}

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
		dR -= 1; //Move position counterclockwise
	}
	update++;
	//DEBUG("dR = %f\n", dR);
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
		dR -= 1; //Move position counterclockwise
	}
	update++;
	//DEBUG("dR = %f\n", dR);
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
		dL -= 1; //Move position counterclockwise
	}
	update++;
	//DEBUG("dL = %f\n", dL);
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
		dL -= 1; //Move position counterclockwise
	}
	update++;
	//DEBUG("dL = %f\n", dL);
	//printf("pos = %d\n", pos);
}