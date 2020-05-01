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
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = flags.start_cond;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_8_bits(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (flags.bits_received && !(flags.ack || flags.xck) && !flags.timeout && !flags.stop);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_flag (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (!flags.bits_received && (flags.ack || flags.xck) && !flags.timeout && !flags.stop);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (!flags.bits_received && !(flags.ack || flags.xck) && !flags.timeout && flags.stop);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_timeout (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!flags.bits_received && !(flags.ack || flags.xck) && flags.timeout && !flags.stop);
	pthread_mutex_unlock (&mutex);
	return result;
}

// Void

static void
begin (fsm_t* this)
{
	TipoProyecto *p_sgp30;
	p_sgp30 = (TipoProyecto*)(this->user_data);

	tmr_startms((tmr_t*)(p_sgp30->tmr_timeout), TIMEOUT_TIME);

	pthread_mutex_lock (&mutex);
	flags.start_cond = 0;
	pthread_mutex_unlock (&mutex);
}

static void
send_ACK (fsm_t* this)
{
	TipoProyecto *p_sgp30;
	p_sgp30 = (TipoProyecto*)(this->user_data);

	char message;
	message = "ACK";
	socket_send(&message);

	tmr_startms((tmr_t*)(p_sgp30->tmr_timeout), TIMEOUT_TIME);

	pthread_mutex_lock (&mutex);
	flags.bits_received = 0;
	pthread_mutex_unlock (&mutex);

}

static void
do_not_count (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	flags.ack = 0;
	flags.xck = 0;
	pthread_mutex_unlock (&mutex);
}

static void
halt (fsm_t* this)
{
	TipoProyecto *p_sgp30;
	p_sgp30 = (TipoProyecto*)(this->user_data);
    /*
	Esperas a que vuelva a haber conexión
	Apagar el timeout (¿tmr_destroy  o tmr_stop?)
    */
	pthread_mutex_lock (&mutex);
	flags.stop_cond = 0;
	pthread_mutex_unlock (&mutex);
}


static void
send_XCK (fsm_t* this)
{
	char message;
	message = "XCK";
	socket_send(&message);
    /*
	Apagar el timeout
    */
	pthread_mutex_lock (&mutex);
	flags.timeout = 0;
	pthread_mutex_unlock (&mutex);
}


/*
Interruption functions
*/


static void timeout_timer (union sigval value)
{
	pthread_mutex_lock (&mutex);
	flags.timeout = 1;
	pthread_mutex_unlock (&mutex);

	printf("TIMEOUT\n");
}

int
sensor_ack_init(TipoProyecto *p_sgp30)
{

	p_sgp30->tmr_timeout = tmr_new (timeout_timer); // timer starter when IAQ, turns realmeasures into 1

	pthread_mutex_init(&mutex, NULL);

	printf("\nSystem init complete!\n");
	fflush(stdout);

	return 0;
}

int
fsm_new_sensor_ack (/*int* validp, int pir, int alarm*/)
{
    static fsm_trans_t alarm_tt[] = {
        {  IDLE, check_start, ACK, begin},
        {  ACK, check_8_bits, ACK, send_ACK},
        {  ACK, check_flag, ACK, do_not_count},
        {  ACK, check_stop, IDLE, halt},
        {  ACK, check_timeout, IDLE, send_XCK},
        { -1, NULL, -1, NULL },
    };
}
