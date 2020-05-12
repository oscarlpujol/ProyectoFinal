/*

*/

#include "sensor.h"

static pthread_mutex_t mutex;
static TipoFlags jarvan;
static TipoFlags jarvan_sensor;
static TipoSensor sgp30;

enum states {
	IDLE, // initial state
	ACK
};

// Int

static int
check_start (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = jarvan.start_cond;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_8_bits(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.bits_received && !(jarvan.ack || jarvan.xck) && !jarvan.timeout && !jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_flag (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.bits_received && (jarvan.ack || jarvan.xck) && !jarvan.timeout && !jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.bits_received && !(jarvan.ack || jarvan.xck) && !jarvan.timeout && jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_timeout (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.bits_received && !(jarvan.ack || jarvan.xck) && jarvan.timeout && !jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

// Void

static void
begin (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	tmr_startms((tmr_t*)(p_sgp30->tmr_timeout), TIMEOUT_TIME);

	pthread_mutex_lock (&mutex);
	jarvan.start_cond = 0;
	pthread_mutex_unlock (&mutex);
}

static void
send_ACK (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	char message;
	message = "ACK";
	write(p_sgp30->socket_desc,&message,20);

	tmr_startms((tmr_t*)(p_sgp30->tmr_timeout), TIMEOUT_TIME);

	pthread_mutex_lock (&mutex);
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);

}

static void
do_not_count (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	jarvan.ack = 0;
	jarvan.xck = 0;
	pthread_mutex_unlock (&mutex);
}

static void
halt (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	tmr_destroy(p_sgp30->tmr_timeout); // Puede volver a encenderse otra vez?

	pthread_mutex_lock (&mutex);
	jarvan.stop_cond = 0;
	pthread_mutex_unlock (&mutex);
}


static void
send_XCK (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	char message;
	message = "XCK";
	write(p_sgp30->socket_desc,&message,20);

	//tmr_destroy(&(p_sgp30->tmr_timeout)); // Puede volver a encenderse otra vez?

	pthread_mutex_lock (&mutex);
	jarvan.timeout = 0;
	pthread_mutex_unlock (&mutex);
}


/*
Interruption functions
*/


static void
timeout_timer (union sigval value)
{
	pthread_mutex_lock (&mutex);
	jarvan.timeout = 1;
	jarvan_sensor.timeout = 1;
	pthread_mutex_unlock (&mutex);

	printf("TIMEOUT\n");
}

int
sensor_ack_init(TipoSensor *p_sgp30, TipoFlags *flags, TipoFlags *flags_sensor)
{
	jarvan = *flags;
	jarvan_sensor = *flags_sensor;
	sgp30 = *p_sgp30;

	p_sgp30->tmr_timeout = tmr_new (timeout_timer); // timer starter when IAQ, turns realmeasures into 1

	pthread_mutex_init(&mutex, NULL);

	printf("\nSensor ack init complete!\n");
	fflush(stdout);

	return 0;
}

fsm_t*
fsm_new_sensor_ack ()
{
    static fsm_trans_t sensor_ack_tt[] = {
        {  IDLE, check_start, ACK, begin},
        {  ACK, check_8_bits, ACK, send_ACK},
        {  ACK, check_flag, ACK, do_not_count},
        {  ACK, check_stop, IDLE, halt},
        {  ACK, check_timeout, IDLE, send_XCK},
        { -1, NULL, -1, NULL },
    };

		return fsm_new (IDLE, sensor_ack_tt, &sgp30);
}
