#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define debug 1
#define DEBUG(args ...) if(debug) { printf(args); }

//pin values
#define phaseAPinL 0
#define phaseBPinL 1
#define phaseAPinR 2
#define phaseBPinR 3

//system info
#define XBEE_BAUD 57600
#define UPDATE_SIZE_MAX 128
#define UPDATE_SIZE_MIN 64
#define ALL_TYPES 999

//CONSTANTS
int update_size = 64; //minimum # updates to trigger a packet send
int update_match = 0;
double radius = .2625 * .3048; //radius of encoder wheels (m)
double encoderDist = 1.484375 * .3048; //distance between encoder wheels (m)
double motorDist = 2.5416666 * .3048; //distance between motor wheels
double TICKS_PER_CYCLE = 4096; //P/R on encoder * 4

//forward decl
void getSerialData();
int processPacket(int, double, double, double, double);
void sendPositionPacket(double, double, double);
void clearLine(int);
void clearPacket();

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

//Computer Info
char delim = '|';
int key = 1234;
int platform = 0;
int type = 1; //0 for motor brain, 1 for encoder brain
char mac_address[256]; //MAC address (eth0) of brain

//serial packet info
int valuesLoaded = 0;

char key_in_str[128];
char plat_in_str[128];
char type_in_str[128];
char id_in_str[128];
char comm_in_str[128];
char val1_in_str[128];
char val2_in_str[128];
char val3_in_str[128];
char checksum_in_str[128]; //checksum = comm + val1 + val2 + val3

int key_in;
int plat_in;
int type_in;
int id_in;
int comm_in;
double val1_in;
double val2_in;
double val3_in;
double checksum_in;

//Test program that send the position of a platform driving in circles.
int main(int argc, char* argv)  {

	DEBUG("Encoder Test:\n");

	DEBUG("MAC address: ");
	FILE* mac = fopen("/sys/class/net/eth0/address", "r");
	//DEBUG("mac open\n");
	fgets(mac_address, 255, mac);
	//DEBUG("got mac_address\n");
	fclose(mac);
	DEBUG("%s\n", mac_address);

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
			//interpret serial data and act on it
			getSerialData();
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
			DEBUG("%d: %f, %f\n", updates_in, dL_in, dR_in);

			//update update size to process updates efficiently
			if(updates_in <= update_size) { // if updates were processed without delay
				update_match++;
				//if updates are consistently processed without delay, attempt to lower the update size
				if(update_match >= update_size) {
					update_match = 0;
					if(update_size > UPDATE_SIZE_MIN * 2) {
						update_size /= 2;
					}
					else if(update_size > UPDATE_SIZE_MIN) {
						update_size -= 1;
					}
				}
			}
			else { //if updates were processed with some delay
				if(update_size < UPDATE_SIZE_MAX / 2) {
					update_size *= 2;
				}
				else if(update_size < UPDATE_SIZE_MAX) {
					update_size += 1;
				}
			}

			dR_in = dR_in * 2 * M_PI / TICKS_PER_CYCLE;
			dL_in = dL_in * 2 * M_PI / TICKS_PER_CYCLE;

			dtheta = (radius / (2 * encoderDist)) * (dR_in - dL_in);
			theta = theta + dtheta;
			theta = fmod(theta, 2 * M_PI);

			// |x| = |x0| + |cos(theta) cos(theta)| * |dR| = |x0| + r/2 * |cos(theta)*dR + cos(theta) * dL|
			// |y|   |y0|   |sin(theta) sin(theta)|   |dL|   |y0| +       |sin(theta)*dR + sin(theta) * dL|

			// x = x0 + r/2 * cos(theta) * (dR + dL)
			// y = y0 + r/2 * sin(theta) * (dR + dL)

			x = x + radius / 2 * cos(theta) * (dR_in + dL_in);
			y = y + radius / 2 * sin(theta) * (dR_in + dL_in);

			sendPositionPacket(x, y, theta);
		}
	}

	return 0;
}

///PACKET FUNCTIONS:

void getSerialData() {
	while(serialDataAvail(serialPort) > 0) {
		char c = (char)serialGetchar(serialPort);
		DEBUG("%c", c);
		if(c == '|') {
			valuesLoaded += 1;
			//preemptively ignore packets
			switch(valuesLoaded) {
				case 1:
					key_in = atoi(key_in_str);
					if(key_in != key) {
						DEBUG("ignoring packet: key mismatch\n");
						clearLine(serialPort);
						clearPacket();
					}
					break;
				case 2:
					plat_in = atoi(plat_in_str);
					if(plat_in != platform && plat_in) {
						DEBUG("ignoring packet: platform mismatch\n");
						clearLine(serialPort);
						clearPacket();
					}
					break;
				case 3:
					type_in = atoi(type_in_str);
					if(type_in != type && type_in != ALL_TYPES) {
						DEBUG("ignoring packet: type mismatch\n");
						clearLine(serialPort);
						clearPacket();
					}
					break;
			}
		}
		else if(c == '\n') {
			//Display packet info
			DEBUG("key_in: %s\n", key_in_str);
			DEBUG("plat_in: %s\n", plat_in_str);
			DEBUG("type_in: %s\n", type_in_str);
			DEBUG("id_in: %s\n", id_in_str);
			DEBUG("comm_in: %s\n", comm_in_str);
			DEBUG("val1_in: %s\n", val1_in_str);
			DEBUG("val2_in: %s\n", val2_in_str);
			DEBUG("val3_in: %s\n", val3_in_str);
			DEBUG("checksum_in: %s\n", checksum_in_str);

			//process all inputs
			int ignore = 0;
			int isBad = 0;
			//If fewer than 4 values have been loaded, we cannot tell if this packet was sent from the controller.
			if(valuesLoaded < 4) {
				//ignore packet
				DEBUG("too few values loaded, ignoring: %d\n", valuesLoaded);
				ignore = 1;
			}
			//Otherwise, we need to check and see if the values are valid.
			else {
				//target_L should be an integer between 0 and 1. Otherwise, packet is bad.
				comm_in = atoi(comm_in_str);
				if(comm_in < 0 || comm_in > 1) {
					isBad = 1;
					DEBUG("comm out of bounds: %d\n", comm_in);
				}
				//val1-val3 should be doubles. Otherwise, packet is bad.
				val1_in = atof(val1_in_str);
				val2_in = atof(val2_in_str);
				val3_in = atof(val3_in_str);
				DEBUG("values: %f, %f, %f\n", val1_in, val2_in, val3_in);
				//checkSum should be equal to the value of target_L_in + target_R_in. Otherwise, packet is bad.
				checksum_in = atof(checksum_in_str);
				
				isBad = processPacket(comm_in, val1_in, val2_in, val3_in, checksum_in);
			}
			if(!ignore) {
				char badString[8] = "bad:";
				char goodString[8] = "good:";
				char* pString;
				if(isBad) {
					strcat(badString, id_in_str);
					pString = badString;
				}
				else {
					strcat(goodString, id_in_str);
					pString = goodString;
					update = 1;
				}
				//acknowledge packet
				DEBUG("sending packet %s...\n", pString);
				serialPuts(serialPort, pString);
			}
			else {
				DEBUG("ignoring packet\n");
			}
			clearPacket();
		}
		else {
			char tmp[2] = "\0\0";
			tmp[0] = c;

			switch(valuesLoaded) {
				//process key entry
				case 0:
					strcat(key_in_str, tmp); //add next char to key
					break;

				//process platform entry
				case 1:
					strcat(plat_in_str, tmp); //add next char to platform
					break;

				//process target entry
				case 2:
					strcat(type_in_str, tmp); //add next char to type
					break;

				case 3:
					strcat(id_in_str, tmp); //add next char to id
					break;

				//process comm entry
				case 4:
					strcat(comm_in_str, tmp); //add next char to comm
					break;

				//process val1 entry
				case 5:
					strcat(val1_in_str, tmp); //add next char to val1
					break;

				//process val1 entry
				case 6:
					strcat(val2_in_str, tmp); //add next char to val2
					break;

				//process val1 entry
				case 7:
					strcat(val3_in_str, tmp); //add next char to val3
					break;

				//process checkSum entry
				case 8:
					strcat(checksum_in_str, tmp); //add next char to checksum
					break;

			//all values processed
			default:

				break;
			}
		}
	}
}

int processPacket(int comm, double val1, double val2, double val3, double checksum) {
	int isBad = 0;

	//checkSum should be equal to the value of target_L_in + target_R_in. Otherwise, packet is bad.
	if(checksum < val1 + val2 + val3 - 0.000001 || checksum > val1 + val2 + val3 + 0.000001) {
		isBad = 1;
		DEBUG("checkSum does not match: %d != %d + %d + %d\n", checksum, val1, val2, val3);
		return isBad;
	}

	//Interpret Command
	switch(comm) {
		int attribute;
		double value, valueCheck;
		//Set Position
		case 0:
			x = val1;
			y = val2;
			theta = val3;
			DEBUG("new position: x = %f, y = %f, theta = %f\n", x, y, theta);

			sendPositionPacket(x, y, theta);
			break;

		//Set Attribute
		case 1:
			attribute = (int)val1;
			value = val2;
			valueCheck = val3;
			//Interpret Attribute Type
			switch((int)val1) {
				//KEY
				case 0:
					if((int)value == (int)valueCheck && value > 0) {
						key = (int)value;
						DEBUG("new key: %d\n", key);
					}
					else {
						DEBUG("key set failed: %d != %d\n", value, valueCheck);
					}
					break;
				//PLATFORM ID
				case 1:
					if((int)value == (int)valueCheck && (int)value >= 0) {
						platform = (int)value;
						DEBUG("new platform id: %d\n", platform);
					}
					else {
						DEBUG("platform id set failed: %d != %d\n", (int)value, (int)valueCheck);
						isBad = 1;
					}
					break;
				default:
				//BASE (Distance b/t encoder wheels)
				case 2:
					if(value == valueCheck && value > 0) {
						encoderDist = (int)value;
						DEBUG("new base: %f\n", encoderDist);
					}
					else {
						DEBUG("base set failed: %f != %f\n", value, valueCheck);
					}
					break;
				//RADIUS (Radius of encoder wheels)
				case 3:
					if(value == valueCheck && value > 0) {
						radius = (int)value;
						DEBUG("new radius: %f\n", radius);
					}
					else {
						DEBUG("radius set failed: %f != %f\n", value, valueCheck);
					}
					break;
				//TICKS_PER_CYCLE (P/R of encoders * 4)
				case 4:
					if((int)value % 128 == 0 && (int)value == (int)valueCheck) {
						TICKS_PER_CYCLE = (int)value;
						DEBUG("new P/R: %d\n", TICKS_PER_CYCLE);
					}
					else {
						DEBUG("invalid P/R resolution: %d\n", (int)value);
					}
					break;
			}
			break;
	}
	return isBad;
}

void sendPositionPacket(double x, double y, double theta) {
	checksum = theta + x + y;

	//format position data for serial output
	sprintf(x_str, "%f", x);
	sprintf(y_str, "%f", y);
	sprintf(theta_str, "%f", theta);
	sprintf(checksum_str, "%f", checksum);

	char packet[128];
	sprintf(packet, "%d|%d|%s|%s|%s|%s\n", key, platform, x_str, y_str, theta_str, checksum_str);

	//DEBUG("sending packet: %s", packet);
	serialPuts(serialPort, packet);
}

//Clears buffer up to next newline
void clearLine(int serialPort) {
	char x = ' ';
	while(serialDataAvail > 0 && x != '\n') {
		x = (char)serialGetchar(serialPort);
	}
}

//Resets packet values
void clearPacket() {
	key_in_str[0] = '\0';
	plat_in_str[0] = '\0';
	type_in_str[0] = '\0';
	id_in_str[0] = '\0';
	comm_in_str[0] = '\0';
	val1_in_str[0] = '\0';
	val2_in_str[0] = '\0';
	val3_in_str[0] = '\0';
	checksum_in_str[0] = '\0';

	valuesLoaded = 0;
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
		dL -= 1; //Move position clockwise
	}
	else {
		dL += 1; //Move position counterclockwise
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
		dL -= 1; //Move position clockwise
	}
	else {
		dL += 1; //Move position counterclockwise
	}
	update++;
	//DEBUG("dL = %f\n", dL);
	//printf("pos = %d\n", pos);
}
