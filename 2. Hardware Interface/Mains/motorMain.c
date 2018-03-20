#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define debug 1
#define DEBUG(args ...) if(debug) { printf(args); }

#define addr_l 128
#define addr_r 135

#define active_l 0
#define active_r 1

#define MOTOR_MIN 0
#define MOTOR_MAX 127

#define PACKET_ARGS 9
#define ALL_TYPES 999

//Forward Decl
void getSerialData();
int processCommand(int, int, int, int, int);
void updateSpeeds();
void clearLine(int);
void clearPacket();
void writeMotors();
void sendMotorPacket(int,int,int,int);

//Serial Port Values
int serialXBee;
int serialMotor;

//Motor Active Values
int targetL, targetR;
int speedL, speedR;
int accelDelayL, accelDelayR;

//Motor Limits
int minL, minR;
int maxL, maxR;

//Computer Info
char delim = '|'; //delimeter for packet processing
int key = 1234;
int platform = 0;
int type = 0; //type = 0 for motor brain, 1 for encoder brain.
char mac_address[256]; //MAC address of brain

//values for packet input
char key_in_str[128] = "\0";
char plat_in_str[128] = "\0";
char target_in_str[128] = "\0";
char id_in_str[128] = "\0";
char comm_in_str[128] = "\0";
char target_L_in_str[128] = "\0";
char target_R_in_str[128] = "\0";
char accel_time_in_str[128] = "\0";
char checkSum_in_str[128] = "\0";

int key_in; //value should be positive int matching key
int plat_in; //value should be positive int matching platform
int target_in; //value should be positive int matching type
int id_in;
int comm_in;
int target_L_in; //value should be int in range of -1024 to 1024
int target_R_in; //value should be int in range of -1024 to 1024
int accel_time_in; //value should be int greater than 0
int checkSum_in; //value should match sum of target_L_in and target_R_in

int valuesLoaded = 0;

int update = 0;

int main(int argc, char* argv)  {

	DEBUG("Booting Motor Controllers...\n");
	delay(2000);
	DEBUG(" Done\n");

	DEBUG("Starting Motor Brain...\n");

	DEBUG("MAC address: ");
	FILE *mac = fopen("/sys/class/net/eth0/address", "r");
	fgets(mac_address, 255, mac);
	fclose(mac);
	DEBUG("%s\n", mac_address);

	wiringPiSetup();
	serialXBee = serialOpen("/dev/ttyUSB0", 57600);
	serialMotor = serialOpen("/dev/ttyS0", 9600);

	//Initialize values
	targetL = 0;
	targetR = 0;

	speedL = 0;
	speedR = 0;

	accelDelayL = 0;
	accelDelayR = 0;

	minL = 0;
	minR = 0;

	maxL = MOTOR_MAX;
	maxR = MOTOR_MAX;

	DEBUG("Activating Motors...\n");
	sendMotorPacket(addr_l, 0, 0, 0);
	sendMotorPacket(addr_r, 0, 0, 0);

	digitalWrite(active_l, HIGH);
	digitalWrite(active_r, HIGH);


	//Initialize serial

	if(serialXBee != -1 && serialMotor != -1) {
		DEBUG("serial init successful.\n");
	}
	else {
		DEBUG("serial init failed.\n");
	}
	serialFlush(serialXBee);

	DEBUG("Motor Brain Started.\n");

	//MAIN LOOP
	while(1) {
		//Process all data in buffer
		if(serialDataAvail(serialXBee) > 0) {
			getSerialData();
		}
		//Set speeds closer to target speeds
		if(update > 0) {
			updateSpeeds();
			writeMotors();
		}
	}
	return 0;
}

//PROCESS SERIAL INPUT:
//Processes any waiting serial input and loads it into a packet
//Standard packet 0: sets target speeds to target_R and target_L, sets corresponding accelDelay.
//Standard packet 1: modifies attributes: 0 = key (int), 1 = platform id (int)
//E_STOP: sets target speeds, speeds, and accelDelay to 0, writes values to motors.
//PAUSE: sets target speeds to 0, accelDelays to 2 seconds (overall), calls motor update.
void getSerialData() {
	while(serialDataAvail(serialXBee) > 0) {
		char c = (char)serialGetchar(serialXBee);
		DEBUG("%c", c);
		if(c == '|') {
			valuesLoaded += 1;
			//preemptively ignore packets
			switch(valuesLoaded) {
				case 1:
					key_in = atoi(key_in_str);
					if(key_in != key) {
						DEBUG("ignoring packet: key mismatch\n");
						clearLine(serialXBee);
						clearPacket();
					}
					break;
				case 2:
					DEBUG("Checking for E_STOP...\n");
					if(strcmp(plat_in_str, "E_STOP") == 0) {
						DEBUG("emergency stop recieved!\n");
						//Set all values to stop values
						targetL = 0;
						targetR = 0;
						speedL = 0;
						speedR = 0;
						accelDelayL = 0;
						accelDelayR = 0;
						writeMotors();
						DEBUG("emergency stop processed\n");
					}
					DEBUG("Checking for PAUSE...\n");
					if(strcmp(plat_in_str, "PAUSE") == 0) {
						DEBUG("pause command recieved!\n");
						//Set all target values to stop
						targetL = 0;
						targetR = 0;
						accelDelayL = (int)(1000.0 / (double)speedL);
						accelDelayR = (int)(1000.0 / (double)speedR);
						update = 1;
						DEBUG("pause processed\n");
					}

					plat_in = atoi(plat_in_str);
					if(plat_in != platform) {
						DEBUG("ignoring packet: platform mismatch\n");
						clearLine(serialXBee);
						clearPacket();
					}
					break;
				case 3:
					target_in = atoi(target_in_str);
					if(target_in != type && target_in != ALL_TYPES) {
						DEBUG("ignoring packet: type mismatch\n");
						clearLine(serialXBee);
						clearPacket();
					}
					break;
			}
		}
		//PROCESS COMPLETE PACKET:
		//Packet structure:
		//key|platform|target|id|target_L|target_R|accel_time|checkSum
		//key: key lock for establishing valid packet sending
		//platform: id of target platform: must match current platform id
		//target: target brain: 0 = motor brain, 1 = encoder brain
		//id: id of packet - used for handshake
		//target_L, target_R: target speeds for each motor
		//accel_time: time (in seconds) to accelerate to target speeds
		//checkSum: validation value == target_L + target_R + accel_time
		else if(c == '\n') {
			//Display packet info
			DEBUG("key_in: %s\n", key_in_str);
			DEBUG("plat_in: %s\n", plat_in_str);
			DEBUG("target_in: %s\n", target_in_str);
			DEBUG("id_in: %s\n", id_in_str);
			DEBUG("comm_in: %s\n", comm_in_str);
			DEBUG("target_L_in: %s\n", target_L_in_str);
			DEBUG("target_R_in: %s\n", target_R_in_str);
			DEBUG("accel_time_in: %s\n", accel_time_in_str);
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
				comm_in = atoi(comm_in_str);
				target_L_in = atoi(target_L_in_str);
				target_R_in = atoi(target_R_in_str);
				accel_time_in = atoi(accel_time_in_str);
				checkSum_in = atoi(checkSum_in_str);
				isBad = processCommand(comm_in, target_L_in, target_R_in, accel_time_in, checkSum_in);
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
				}
				//acknowledge packet
				DEBUG("sending packet %s...\n", pString);
				serialPuts(serialXBee, pString);
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

				//load platform entry
				case 1:
					strcat(plat_in_str, tmp); //add next char to platform
					break;

				//load target entry
				case 2:
					strcat(target_in_str, tmp); //add next char to target
					break;

				//load id entry
				case 3:
					strcat(id_in_str, tmp); //add next char to id
					break;

				//load comm entry
				case 4:
					strcat(comm_in_str, tmp); //add next char to comm
					break;

				//load target_L entry
				case 5:
					strcat(target_L_in_str, tmp); //add next char to target_L
					break;

				//load target_R entry
				case 6:
					strcat(target_R_in_str, tmp); //add next char to target_R
					break;

				//load target_R entry
				case 7:
					strcat(accel_time_in_str, tmp); //add next char to target_R
					break;

				//load checkSum entry
				case 8:
					strcat(checkSum_in_str, tmp); //add next char to checksum
					break;

				//all values loaded
				default:

					break;
			}
		}
	}
}

//Process Command
int processCommand(int comm_in, int val1, int val2, int val3, int checkSum) {
	int isBad = 0;
	switch(comm_in) {
		//SET MOTOR VALUES
		case 0:
			//target_L should be a number smaller than the MOTOR_MAX. Otherwise, packet is bad.
			if(val1 < -MOTOR_MAX || val1 > MOTOR_MAX) {
				isBad = 1;
				DEBUG("target_L out of bounds: %d\n", val1);
			}
			//target_R should be a number smaller than the MOTOR_MAX. Otherwise, packet is bad.
			if(val2 < -MOTOR_MAX || val2 > MOTOR_MAX) {
				isBad = 1;
				DEBUG("target_R out of bounds: %d\n", val2);
			}
			if(val3 < 0) {
				isBad = 1;
				DEBUG("accel_time out of bounds: %d\n", val3);
			}
			//checkSum should be equal to the value of target_L_in + target_R_in. Otherwise, packet is bad.
			if(checkSum != target_L_in + target_R_in + accel_time_in) {
				isBad = 1;
				DEBUG("checkSum does not match: %d != %d + %d + %d\n", checkSum, val1, val2, val3);
			}
			//set motor values
			if(!isBad) {
				targetL = target_L_in;
				targetR = target_R_in;
				float diffR, diffL;
				diffL = abs(speedL - targetL);
				diffR = abs(speedR - targetR);
				float accelDelay = (float)accel_time_in * 1000.0 * 2 / (diffL + diffR);
				printf("delay: %f\n", accelDelay);
				accelDelayL = (float)accelDelay * diffL / (diffL + diffR);
				accelDelayR = (float)accelDelay * diffR / (diffL + diffR);
				printf("left delay: %d, right delay %d\n", accelDelayL, accelDelayR);
				update = 1;
			}
			break;
		case 1:
			switch(val1) {
				//set key
				case 0:
					key = val2;
					DEBUG("new key: %d\n", key);
					break;
				//set platform id
				case 1:
					platform = val2;
					DEBUG("new platform id: %d\n", platform);
					break;
				default:
					DEBUG("unknown value\n");
					break;
			}
	}
	return isBad;
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
	//DEBUG("targets: %d | %d\n", targetL, targetR);
}

//Sets motor speeds to current speed values
void writeMotors() {
	if(speedL > 0) {
		sendMotorPacket(addr_l, 0, speedL, accelDelayL);
		//pwmWrite(pwmPinL, speedL);
		//digitalWrite(dirPinL, 0);
	}
	else {
		sendMotorPacket(addr_l, 1, -speedL, accelDelayL);
		//pwmWrite(pwmPinL, -speedL);
		//digitalWrite(dirPinL, 1);
	}
	if(speedR > 0) {
		sendMotorPacket(addr_r, 0, speedR, accelDelayR);
		//pwmWrite(pwmPinR, speedR);
		//digitalWrite(dirPinR, 0);
	}
	else {
		sendMotorPacket(addr_r, 1, -speedR, accelDelayR);
		//pwmWrite(pwmPinR, -speedR);
		//digitalWrite(dirPinR, 1);
	}
	DEBUG("setting speeds: %d | %d\n", speedL, speedR);
}

void sendMotorPacket(int address, int command, int speed, int accelDelay) {
	serialPutchar(serialMotor, (char)address);
	serialPutchar(serialMotor, (char)command);
	serialPutchar(serialMotor, (char)speed);
	int valid = (address + command + speed) & 0b0111111;
	serialPutchar(serialMotor, (char)valid);
	DEBUG("sending comm %d at speed %d to %d\n", command, speed, address);
	if(accelDelay > 0) {
		delay(accelDelay);
	}
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
	comm_in_str[0] = '\0';
	target_L_in_str[0] = '\0';
	target_R_in_str[0] = '\0';
	accel_time_in_str[0] = '\0';
	checkSum_in_str[0] = '\0';

	valuesLoaded = 0;
}
