/** File name   :sensor_ack.c
  * Description :fsm that acks whenever sensor has to
  */

// Includes
#include "sensor.h"

// Private variables
static pthread_mutex_t mutex;
static TipoFlags jarvan;
static TipoFlags jarvan_sensor;
static TipoSensor sgp30;

// FSM states
enum states {
	IDLE, // initial state
	ACK
};

// Flag checking functions
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
	result = (jarvan.bits_received && !(jarvan.ack || jarvan.xck) && !jarvan.timeout && !jarvan.stop_cond && !jarvan.start_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_flag (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.bits_received && (jarvan.ack || jarvan.xck || jarvan.start_cond) && !jarvan.timeout && !jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.bits_received && !(jarvan.ack || jarvan.xck) && !jarvan.timeout && jarvan.stop_cond && !jarvan.start_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_timeout (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.bits_received && !(jarvan.ack || jarvan.xck) && jarvan.timeout && !jarvan.stop_cond && !jarvan.start_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

// Function for the FSM

/**
* @brief Begins watching for new messages
*/
static void
begin (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("Begin\n");
	fflush(stdout);

	pthread_mutex_lock (&mutex);
	jarvan.start_cond = 0;
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Send ack
*/
static void
send_ACK (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("Ack enviado\n");
	fflush(stdout);

	char message[20] = "ACK";
	pthread_mutex_lock(&mutex_socket);
	write(p_sgp30->socket_desc,&message,20);
	pthread_mutex_unlock(&mutex_socket);

	pthread_mutex_lock (&mutex);
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Do not count as a message if its an ack, xck or start cond
*/
static void
do_not_count (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("Do not count\n");
	fflush(stdout);

	pthread_mutex_lock (&mutex);
	jarvan.ack = 0;
	jarvan.xck = 0;
	jarvan.start_cond = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Stop watching for new messages
*/
static void
halt (fsm_t* this)
{
	printf("Halt\n");
	fflush(stdout);

	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	pthread_mutex_lock (&mutex);
	jarvan.stop_cond = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Begins watching for new messages
*/
static void
send_XCK (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("Xck enviado\n");
	fflush(stdout);

	char message[20] = "XCK";
	pthread_mutex_lock(&mutex_socket);
	write(p_sgp30->socket_desc,&message,20);
	pthread_mutex_unlock(&mutex_socket);

	pthread_mutex_lock (&mutex);
	jarvan.timeout = 0;
	pthread_mutex_unlock (&mutex);
}

// ISR
// Interruption functions for the timers and the observer
static void
timeout_timer (union sigval value)
{
	pthread_mutex_lock (&mutex);
	jarvan.timeout = 1;
	timeout_isr();
	pthread_mutex_unlock (&mutex);

	printf("TIMEOUT\n");
}

void
start_ack_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.start_cond = 1;
	pthread_mutex_unlock (&mutex);
}

void
stop_ack_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.stop_cond = 1;
	pthread_mutex_unlock (&mutex);
}

void
ack_ack_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.ack = 1;
	pthread_mutex_unlock (&mutex);
}

void
xck_ack_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.xck = 1;
	pthread_mutex_unlock (&mutex);
}

void
bits_ack_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.bits_received = 1;
	pthread_mutex_unlock (&mutex);
}

// Initialization functions
int
sensor_ack_init(TipoSensor *p_sgp30, TipoFlags *flags)
{
	jarvan = *flags;
	sgp30 = *p_sgp30;

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
