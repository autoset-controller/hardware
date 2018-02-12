#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define debug 1
#define DEBUG(args ...) if(debug) { printf(args); }

#define pwmPinL 1
#define pwmPinR 26
#define dirPinL 0
#define dirPinR 2

//Forward Decl
void updateSpeeds();
void clearLine(int);
void clearPacket();
void writeMotors();

//Serial Port Values
int serialPort;

//Motor Active Values
int targetL, targetR;
int speedL, speedR;

//Motor Limits
int minL, minR;
int maxL, maxR;

//Computer Info
char delim = '|'; //delimeter for packet processing
int key = 1234;
int platform = 0;
int type = 0; //type = 0 for motor brain, 1 for encoder brain.

//values for packet input
char key_in_str[128] = "\0";
char plat_in_str[128] = "\0";
char target_in_str[128] = "\0";
char id_in_str[128] = "\0";
char target_L_in_str[128] = "\0";
char target_R_in_str[128] = "\0";
char checkSum_in_str[128] = "\0";

int key_in; //value should be positive int matching key
int plat_in; //value should be positive int matching platform
int target_in; //value should be positive int matching type
int id_in;
int target_L_in; //value should be int in range of -1024 to 1024
int target_R_in; //value should be int in range of -1024 to 1024
int checkSum_in; //value should match sum of target_L_in and target_R_in

int valuesLoaded = 0;

int update = 0;

int main(int argc, char* argv)  {

	DEBUG("Starting Motor Brain...\n");

	wiringPiSetup();
	serialPort = serialOpen("/dev/ttyUSB0", 38400);

	//Initialize pins
	pinMode(pwmPinL, PWM_OUTPUT);
	pinMode(pwmPinR, PWM_OUTPUT);
	pinMode(dirPinL, OUTPUT);
	pinMode(dirPinR, OUTPUT);

	//Initialize serial strings

	//Initialize values
	targetL = 0;
	targetR = 0;

	speedL = 0;
	speedR = 0;

	minL = 80;
	minR = 80;

	maxL = 1024;
	maxR = 1024;

	//Initialize serial

	if(serialPort != -1) {
		DEBUG("serial init successful.\n");
	}
	else {
		DEBUG("serial init failed.\n");
	}
	serialFlush(serialPort);

	while(1) {
		//Process all data in buffer
		if(serialDataAvail(serialPort) > 0) {
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
							if(plat_in != platform) {
								DEBUG("ignoring packet: platform mismatch\n");
								clearLine(serialPort);
								clearPacket();
							}
							break;
						case 3:
							target_in = atoi(target_in_str);
							if(target_in != type) {
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
					DEBUG("target_in: %s\n", target_in_str);
					DEBUG("id_in: %s\n", id_in_str);
					DEBUG("target_L_in: %s\n", target_L_in_str);
					DEBUG("target_R_in: %s\n", target_R_in_str);
					DEBUG("checkSum_in: %s\n", checkSum_in_str);

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
						//Commented tests have already been checked
						/*
						//key should be equal to key stored in system. Otherwise, ignore packet.
						key_in = atoi(key_in_str);
						if(key_in != key) {
							ignore = 1;
							DEBUG("key does not match: %d != %d\n", key_in, key);
						}
						//platform should be equal to platform id stored in system. Otherwise, ignore packet.
						plat_in = atoi(plat_in_str);
						if(plat_in != platform) {
							ignore = 1;
							DEBUG("platform does not match: %d != %d\n", plat_in, platform);
						}
						//target should be equal to type stored in system. Otherwise, ignore packet.
						target_in = atoi(target_in_str);
						if(target_in != type) {
							ignore = 1;
							DEBUG("type does not match: %d != %d\n", target_in, type);
						}*/
						//target_L should be a number between -1024 and 1024. Otherwise, packet is bad.
						target_L_in = atoi(target_L_in_str);
						if(target_L_in < -1024 || target_L_in > 1024) {
							isBad = 1;
							DEBUG("target_L out of bounds: %d\n", target_L_in);
						}
						//target_R should be a number between -1024 and 1024. Otherwise, packet is bad.
						target_R_in = atoi(target_R_in_str);
						if(target_R_in < -1024 || target_R_in > 1024) {
							isBad = 1;
							DEBUG("target_R out of bounds: %d\n", target_R_in);
						}
						//checkSum should be equal to the value of target_L_in + target_R_in. Otherwise, packet is bad.
						checkSum_in = atoi(checkSum_in_str);
						if(checkSum_in != target_L_in + target_R_in) {
							isBad = 1;
							DEBUG("checkSum does not match: %d != %d + %d\n", checkSum_in, target_L_in, target_R_in);
						}
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
							//set motor values
							targetL = target_L_in;
							targetR = target_R_in;
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
							strcat(target_in_str, tmp); //add next char to target
							break;

						case 3:
							strcat(id_in_str, tmp); //add next char to id
							break;

						//process target_L entry
						case 4:
							strcat(target_L_in_str, tmp); //add next char to target_L
							break;

						//process target_R entry
						case 5:
							strcat(target_R_in_str, tmp); //add next char to target_R
							break;

						//process checkSum entry
						case 6:
							strcat(checkSum_in_str, tmp); //add next char to checksum
							break;

						//all values processed
						default:

							break;
					}
				}
			}
		}
		//Set speeds closer to target speeds
		if(update > 0) {
			updateSpeeds();
			writeMotors();
		}
	}

	return 0;
}

///PIN FUNCTIONS:
//Moves actual motor speeds closer to target speeds
void updateSpeeds() {
	update = 0;
	if(targetL > speedL) {
		speedL += 1;
		update += 1;
	}
	else if(targetL < speedL) {
		speedL -= 1;
		update += 1;
	}
	if(targetR > speedR) {
		speedR += 1;
		update += 1;
	}
	else if(targetR < speedR) {
		speedR -= 1;
		update += 1;
	}
	DEBUG("targets: %d | %d\n", targetL, targetR);
}

//Sets motor speeds to current speed values
void writeMotors() {
	if(speedL > 0) {
		pwmWrite(pwmPinL, speedL);
		digitalWrite(dirPinL, 0);
	}
	else {
		pwmWrite(pwmPinL, -speedL);
		digitalWrite(dirPinL, 1);
	}
	if(speedR > 0) {
		pwmWrite(pwmPinR, speedR);
		digitalWrite(dirPinR, 0);
	}
	else {
		pwmWrite(pwmPinR, -speedR);
		digitalWrite(dirPinR, 1);
	}
	DEBUG("setting speeds: %d | %d\n", speedL, speedR);
}

///PACKET FUNCTIONS:
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
	target_in_str[0] = '\0';
	id_in_str[0] = '\0';
	target_L_in_str[0] = '\0';
	target_R_in_str[0] = '\0';
	checkSum_in_str[0] = '\0';

	valuesLoaded = 0;
}
