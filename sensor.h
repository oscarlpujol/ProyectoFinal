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
#include "esp_common.h"
#include "gpio.h"
#include "freertos/task.h"

// #include <wiringPi.h>

#include "fsm.h"
#include "tmr.h"

// Variables

#define OWN_ADDRESS 5000
#define CLK_MS 5
#define REALMEASURES_TIME 400
#define TIMEOUT_TIME 50
#define SEED 2

typedef struct{

	unsigned int realmeasures : 1; // if 0 -> measures are not real, 400 standard; if 1 -> now you can read
	tmr_t* tmr_real_measures; // timer starter when IAQ, turns realmeasures into 1
	tmr_t* tmr_timeout;
	int I2C_ADDRESS_IRIS; // IRIS I2C address
	int I2C_ADDRESS_SENSOR; // SENSOR I2C address
	int measures [6]; // CO2, CO2, CRC_CO2, TVOC, TVOC, CRC_TVOC
	int socket_desc;
	int address;
	char* receiver;

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
	unsigned int H2_sent : 1;
	unsigned int ethanol_sent : 1;
	unsigned int MAQ : 1;
	unsigned int msg_MAQ_left : 1; // 1 messages left, 0 not messages left
	unsigned int CO2_sent : 1;
	unsigned int TVOC_sent : 1;

}TipoFlags;

void
socket_receive(char* receiver, void *socket_desc);
void
socket_send(char* sender, void *socket_desc);

#endif /* _SENSOR_H_ */
