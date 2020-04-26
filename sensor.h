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

typedef struct{

	int realmeasures; // if 0 -> measures are not real, 400 standard; if 1 -> now you can read
	tmr_t* tmr_real_measures; // timer starter when IAQ, turns realmeasures into 1
	int I2C_ADDRESS_IRIS; // IRIS I2C address
	int I2C_ADDRESS_SENSOR; // SENSOR I2C address
	int measures [6]; // CO2, CO2, CRC_CO2, TVOC, TVOC, CRC_TVOC
	int* address;

}TipoSensor;



#endif /* _SENSOR_H_ */
