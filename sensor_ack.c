/*

*/

#include "sensor.h"


static pthread_mutex_t mutex;

enum states {
	IDLE, // initial state
	ACK
}

// Int

static int
check_start (fsm_t* this)
{
    /*
	
    */
}

static int
check_8_bits(fsm_t* this)
{

}

static int
check_flag (fsm_t* this)
{
    
}

static int
check_stop (fsm_t* this)
{
    
}

static int
check_timeout (fsm_t* this)
{
    
}


// Void

static void
begin (fsm_t* this)
{
    /*
 	El sensor esta listo para enviar ACKs
    */
}

static void
send_ACK (fsm_t* this)
{
    /*
	Envia ACK
    */
}

static void
do_not_count (fsm_t* this)
{
	/*
	No interpretarlo como bit, continua mandando ACKs
    */
}

static void
halt (fsm_t* this)
{
    /*
	Esperas a que vuelva a haber conexión
    */
}


static void
send_XCK (fsm_t* this)
{
    /*
	Enviar ACK
	Esperar a que vuelva a haber conexión
    */
}


/*
Interruption functions
*/

static void initial_timer (union sigval value)
{
	pthread_mutex_lock (&mutex);
	// flags |= (FLAG_TIME_OUT_MEDIDA);
	pthread_mutex_unlock (&mutex);
	// num_bits_recibidos = 0;
	//sgp30.realmeasures = 1;
	// tmr_destroy()
	printf("TIMEOUT\n");
}


int
sensor_ack_init(TipoProyecto *p_sgp30)
{
	/*
	p_sgp30->realmeasures = 0;
	p_sgp30->tmr_real_measures = tmr_new (initial_timer); // timer starter when IAQ, turns realmeasures into 1
	p_sgp30->I2C_ADDRESS_IRIS = 0; // IRIS I2C address
	p_sgp30->I2C_ADDRESS_SENSOR = OWN_ADDRESS; // SENSOR I2C address
	p_sgp30->measures [6] = {0,0,0,0,0,0};
	pthread_mutex_init(&mutex, NULL);
*/
	printf("\nSystem init complete!\n");
	fflush(stdout);

	return 0;
}


int
fsm_new_sensor (/*int* validp, int pir, int alarm*/)
{
    static fsm_trans_t alarm_tt[] = {
        {  IDLE, check_start, ACK, begin},
        {  ACK, check_8_bits, ACK, send_ACK},
        {  ACK, check_flag, ACK, do_not_count},
        {  ACK, check_stop, IDLE, halt},
        {  ACK, check_timeout, IDLE, send_XCK},
        { -1, NULL, -1, NULL },
    };
    sensor_ack_init(&sgp30);
}
