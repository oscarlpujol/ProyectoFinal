/*
 */

#ifndef _IRIS_H_
#define _IRIS_H_

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
//#include "esp_common.h"
//#include "gpio.h"
//#include "freertos/task.h"

// #include <wiringPi.h>

#include "fsm.h"
#include "tmr.h"

// Variables

#define CLK_MS 5
#define TIME_ON 15
#define TIME_MAQ 30000
#define OWN_ADDRESS 1234
#define SENSOR_ADDRESS 5000
#define SOCKETNUMBERSEND 8889
#define SOCKETNUMBERRECEIVE 8888

typedef struct{

	unsigned int state : 1; // 1 -> ON, 0 ->OFF
	tmr_t* tmr_MAQ;
	tmr_t* tmr_on;
	int I2C_ADDRESS_IRIS; // IRIS I2C address
	int I2C_ADDRESS_SENSOR; // SENSOR I2C address
	int measures[6];
	int socket_desc_send;
	int socket_desc_receive;
	int* address;
	int length_next_msg;
	char* receiver;
	int num_msg;
	int num_sent;

}TipoIris;

typedef struct{

	unsigned int button_on : 1;
	unsigned int button_off : 1;
	unsigned int time_on : 1;
	unsigned int initialized : 1;
	unsigned int ack : 1;
	unsigned int msg_IAQ_left : 1;
	unsigned int MAQ_now : 1;
	unsigned int timeout_MAQ : 1;
	unsigned int msg_MAQ_left : 1;
	unsigned int bits_received : 1;
	unsigned int msg_checked : 1;
	unsigned int all_msg_received : 1;
	unsigned int msg_received : 1;

}TipoFlags;

void
socket_receive(char* receiver);
void
socket_send(char* sender);
void
button_onoff_interruption(void);
void
button_onoff_isr(void);
void
button_MAQnow_interruption();
void
button_MAQnow_isr(void);

#endif /* _IRIS_H_ */