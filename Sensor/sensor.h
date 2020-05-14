/*
 */

#ifndef _SENSOR_H_
#define _SENSOR_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <time.h>
#include <sys/time.h>


#include "../task.h"
#include "../fsm.h"
#include "../tmr.h"

// Variables

#define OWN_ADDRESS "5000"
#define CLK_MS 1
#define REALMEASURES_TIME 40000
#define TIMEOUT_TIME 1000
#define SEED 2
#define SOCKETNUMBER 8888

static pthread_mutex_t mutex_socket;

typedef struct{

	unsigned int realmeasures : 1; // if 0 -> measures are not real, 400 standard; if 1 -> now you can read
	tmr_t* tmr_real_measures; // timer starter when IAQ, turns realmeasures into 1
	tmr_t* tmr_timeout;
	char I2C_ADDRESS_IRIS[20]; // IRIS I2C address
	char I2C_ADDRESS_SENSOR[20]; // SENSOR I2C address
	char measures [6][20]; // CO2, CO2, CRC_CO2, TVOC, TVOC, CRC_TVOC
	int socket_desc;
	int address;
	char receiver[20];

}TipoSensor;

typedef struct{

	unsigned int bits_received : 1;
	unsigned int ack : 1;
	unsigned int xck : 1;
	unsigned int start_cond : 1;
	unsigned int stop_cond : 1;
	unsigned int timeout : 1;
	unsigned int I2C_address_wrong : 1;
	unsigned int IAQ : 1;
	unsigned int MRS : 1;
	unsigned int msg_MRS_left : 1;
	unsigned int MAQ : 1;
	unsigned int msg_MAQ_left : 1; // 1 messages left, 0 not messages left
	unsigned int incorrect_command : 1;
	unsigned int correct_command : 1;
	unsigned int process1 : 1;
	unsigned int process2 : 1;

}TipoFlags;

void
start_ack_isr();
void
stop_ack_isr();
void
ack_ack_isr();
void
xck_ack_isr();
void
bits_ack_isr();
void
start_isr();
void
stop_isr();
void
ack_isr();
void
xck_isr();
void
bits_isr();
void
new_msg(char* msg);
void
timeout_isr();
void
powerOff_isr();

#endif /* _SENSOR_H_ */
