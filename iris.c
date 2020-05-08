/*

*/

#include "iris.h"

static pthread_mutex_t mutex;

enum states {
  SLEEP, // initial state
  IDLE,
	MSG_IAQ,
	MSG_MAQ,
  LISTEN,
	CHECK_CRC
}

// Int

static int
button_on (fsm_t* this)
{
    /* code */
}
static int
button_off(fsm_t* this)
{
    /* code */
}
static int
time_on_not_init(fsm_t* this)
{
    /* code */
}
static int
flag_ACK_not_msg_IAQ(fsm_t* this)
{
    /* code */
}
static int
init_timeout_measure(fsm_t* this)
{
    /* code */
}
static int
flag_ACK_msg_MAQ_left(fsm_t* this)
{
    /* code */
}
static int
flag_ACK_not_msg_MAQ_left(fsm_t* this)
{
    /* code */
}
static int
eight_bits_received(fsm_t* this)
{
    /* code */
}
static int
msg_received_IRIS_not_msg_checked_eight_bits(fsm_t* this)
{
    /* code */
}
static int
msg_checked(fsm_t* this)
{
    /* code */
}
static int
all_msg_received(fsm_t* this)
{
    /* code */
}


//Void

static void
power_on(fsm_t* this)
{
  /* code */
}
static void
power_off(fsm_t* this)
{
  /* code */
}
static void
iaq_start(fsm_t* this)
{
  /* code */
}
static void
iaq_success(fsm_t* this)
{
  /* code */
}
static void
maq_start(fsm_t* this)
{
  /* code */
}
static void
send_msg_2sensor(fsm_t* this)
{
  /* code */
}
static void
init_maq_success(fsm_t* this)
{
  /* code */
}
static void
received_data_success(fsm_t* this)
{
  /* code */
}
static void
check_msg(fsm_t* this)
{
  /* code */
}
static void
msg_checked(fsm_t* this)
{
  /* code */
}
static void
send_XCK_2sensor_stop_show_results_maq(fsm_t* this)
{
  /* code */
}

fsm_t*
fsm_new_sensor (/*int* validp, int pir, int alarm*/)
{
    static fsm_trans_t alarm_tt[] = {
        {  SLEEP, button_on, IDLE, power_on},
        {  IDLE, button_off, SLEEP, power_off},
        {  IDLE, time_on_not_init, MSG_IAQ, iaq_start},
        {  MSG_IAQ, flag_ACK_not_msg_IAQ, IDLE, iaq_success},
        {  IDLE, init_timeout_measure, MSG_MAQ, maq_start},
        {  MSG_MAQ, flag_ACK_msg_MAQ_left, MSG_MAQ, send_msg_2sensor},
        {  MSG_MAQ, flag_ACK_not_msg_MAQ_left, LISTEN, init_maq_success},
        {  LISTEN, eight_bits_received, LISTEN, received_data_success},
        {  LISTEN, msg_received_IRIS_not_msg_checked_eight_bits, CHECK_CRC, check_msg},
        {  CHECK_CRC, msg_checked, LISTEN, msg_checked_success},
        {  LISTEN, all_msg_received, IDLE, send_XCK_2sensor_stop_show_results_maq},
        { -1, NULL, -1, NULL },
    };
}
