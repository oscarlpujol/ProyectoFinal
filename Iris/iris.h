/** File name   :iris.h
  * Description :header for iris realated files
  */

#ifndef _IRIS_H_
#define _IRIS_H_

// Includes
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
#define CLK_MS 1
#define TIME_ON 1000
#define TIME_MAQ 20000
#define OWN_ADDRESS "1234"
#define SENSOR_ADDRESS "5000"
#define SOCKETNUMBER 8888
#define SOCKETNUMBERGW 8889

// Iris structure
typedef struct{

	unsigned int state : 1; // 1 -> ON, 0 ->OFF
	tmr_t* tmr_MAQ;
	tmr_t* tmr_on;
	char I2C_ADDRESS_IRIS[20]; // IRIS I2C address
	char I2C_ADDRESS_SENSOR[20]; // SENSOR I2C address
	char measures[6][20];
	int socket_desc;
	int socket_desc_GW;
	char* address;
	int length_next_msg;
	int num_msg;
	int num_sent;
	char receiver[20];

}TipoIris;

// Flags
typedef struct{

	unsigned int button_on : 1;
	unsigned int button_off : 1;
	unsigned int time_on : 1;
	unsigned int initialized : 1;
	unsigned int ack : 1;
	unsigned int xck : 1;
	unsigned int msg_IAQ_left : 1;
	unsigned int MAQ_now : 1;
	unsigned int MRS_now : 1;
	unsigned int timeout_MAQ : 1;
	unsigned int msg_MAQ_left : 1;
	unsigned int msg_MRS_left : 1;
	unsigned int bits_received : 1;
	unsigned int msg_checked : 1;
	unsigned int all_msg_received : 1;
	unsigned int msg_received : 1;

}TipoFlags;

// Function used between files
void *
button_onoff_interruption();
void
button_onoff_isr();
void *
button_MAQnow_interruption();
void
button_MAQnow_isr();
void *
button_MRSnow_interruption();
void
button_MRSnow_isr();
void
ack_isr();
void
xck_isr();
void
bits_isr();
void
new_msg(char* msg);
int
kbhit();

#endif /* _IRIS_H_ */
