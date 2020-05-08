/*

*/

#include "iris.h"

static pthread_mutex_t mutex;
static TipoFlags jarvan;

int calculate_CRC(int numberone, int numbertwo);

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
check_button_on (fsm_t* this)
{
  pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.button_on);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_button_off(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.button_off);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_time_on_not_init(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.time_on && !jarvan.initialized);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_msg_IAQ(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.ack && jarvan.msg_IAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_not_msg_IAQ(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.ack && !jarvan.msg_IAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_init_timeout_measure(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.initialized && (jarvan.timeout_MAQ || jarvan.MAQ_now));
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_msg_MAQ_left(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.ack && jarvan.msg_MAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_not_msg_MAQ_left(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.ack && !jarvan.msg_MAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_eight_bits_received(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.bits_received);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_msg_received_IRIS_not_msg_checked_eight_bits(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.msg_received_IRIS && !jarvan.msg_checked && jarvan.bits_received);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_msg_checked(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.msg_checked);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_all_msg_received(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0:
  result = (jarvan.CO2_received && jarvan.TVOC_received);
  pthread_mutex_unlock (&mutex);
  return result;
}

//Void

static void
power_on(fsm_t* this)
{
  TipoIris *p_iris;
	p_iris = (TipoIris*)(this->user_data);

  printf("\nInitializating sensor\n");
	fflush(stdout);

  tmr_startms((tmr_t*)(p_sgp30->tmr_on), TIME_ON);

  pthread_mutex_lock (&mutex);
	jarvan.button_on = 0;
	pthread_mutex_unlock (&mutex);
}

static void
power_off(fsm_t* this)
{
  TipoIris *p_iris;
	p_iris = (TipoIris*)(this->user_data);

  printf("\nShutting off sensor\n");
	fflush(stdout);

  pthread_mutex_lock (&mutex);
	jarvan.button_off = 0;
	pthread_mutex_unlock (&mutex);
}

static void
iaq_start(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nSending IAQ command\n");
	fflush(stdout);

  //Preparar mensajes

  pthread_mutex_lock (&mutex);
	jarvan.time_on = 0;
  jarvan.ack = 1;
	pthread_mutex_unlock (&mutex);
}

static void
iaq_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nIAQ success\n");
	fflush(stdout);

  tmr_startms((tmr_t*)(p_sgp30->tmr_MAQ), TIME_MAQ);

  pthread_mutex_lock (&mutex);
	jarvan.ack = 0;
  jarvan.msg_IAQ_left = 1;
  jarvan.initialized = 1;
	pthread_mutex_unlock (&mutex);
}

static void
maq_start(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nSending MAQ command\n");
	fflush(stdout);

  //Preparar mensajes

  pthread_mutex_lock (&mutex);
	jarvan.timeout_MAQ = 0;
  jarvan.MAQ_now _= 0;
  jarvan.ack = 1;
	pthread_mutex_unlock (&mutex);
}

static void
send_msg_2sensor(fsm_t* this)
{
  /* code */
}

static void
init_maq_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nMAQ command sent, waiting measures\n");
	fflush(stdout);

  pthread_mutex_lock (&mutex);
	jarvan.msg_MAQ_left = 1;
  jarvan.ack = 0;
	pthread_mutex_unlock (&mutex);
}

static void
received_data_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\n8 bits received\n");
	fflush(stdout);

  pthread_mutex_lock (&mutex);
	jarvan.bits_received = 1;
	pthread_mutex_unlock (&mutex);
}

static void
check_msg(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  pthread_mutex_lock (&mutex);
	jarvan.bits_received = 0;
  jarvan.msg_received = 0;
  jarvan.msg_checked = 1;
	pthread_mutex_unlock (&mutex);
}

static void
msg_checked(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  if(check_CRC(p_iris->measures[address-2],p_iris->measures[address-1]) == p_iris->measures[address]){
    printf("\nMSG checked...correct\n");
  	fflush(stdout);
  }else{
    printf("\nMSG checked...incorrect\n");
  	fflush(stdout);
  }

  p_iris->num_msg += 1;

  if(p_iris->num_msg == 2){
    pthread_mutex_lock (&mutex);
    jarvan.all_msg_received = 1;
  	pthread_mutex_unlock (&mutex);
  }

  pthread_mutex_lock (&mutex);
  jarvan.msg_checked = 0;
	pthread_mutex_unlock (&mutex);
}

static void
send_XCK_2sensor_stop_show_results_maq(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  char message = "XCK\n";
  socket_send(&message, &(p_iris->socket_desc));

  char message = "StopCond\n";
  socket_send(&message, &(p_iris->socket_desc));

  printf("CO2 = %d and %d \n", p_iris->measures[0],p_iris->measures[1]);
  fflush(stdout);
  printf("TVOC = %d and %d \n", p_iris->measures[3],p_iris->measures[4]);
  fflush(stdout);

  tmr_startms((tmr_t*)(p_sgp30->tmr_MAQ), TIME_MAQ);

  pthread_mutex_lock (&mutex);
  jarvan.all_msg_received = 0;
	pthread_mutex_unlock (&mutex);
}

//ISR

static void
initial_timer (union sigval value){

	pthread_mutex_lock (&mutex);
	iris.time_on = 1;
	pthread_mutex_unlock (&mutex);
  tmr_destroy(iris.tmr_on);
}

static void
maq_timer (union sigval value){

	pthread_mutex_lock (&mutex);
	iris.timeout_MAQ = 1;
	pthread_mutex_unlock (&mutex);
}

//Init

int
iris_init(TipoIris *p_iris, TipoFlags *flags)
{
  jarvan = *flags;
  iris = *p_iris;

	pthread_mutex_init(&mutex, NULL);

  iris->tmr_MAQ = tmr_new (maq_timer);
  iris->tmr_on = tmr_new(initial_timer);
  iris.I2C_ADDRESS_IRIS = OWN_ADDRESS;
  iris.I2C_ADDRESS_SENSOR = SENSOR_ADDRESS;
  iris.address = 0;
  iris.num_msg = 0;

	printf("\nSystem init complete!\n");
	fflush(stdout);

	return 0;
}


fsm_t*
fsm_new_iris (/*int* validp, int pir, int alarm*/)
{
    static fsm_trans_t alarm_tt[] = {
        {  SLEEP, check_button_on, IDLE, power_on},
        {  IDLE, check_button_off, SLEEP, power_off},
        {  IDLE, check_time_on_not_init, MSG_IAQ, iaq_start},
        {  MSG_IAQ, check_flag_ACK_not_msg_IAQ, IDLE, iaq_success},
        {  MSG_IAQ, check_flag_ACK_msg_IAQ, MSG_IAQ, send_msg_2sensor},
        {  IDLE, check_init_timeout_measure, MSG_MAQ, maq_start},
        {  MSG_MAQ, check_flag_ACK_msg_MAQ_left, MSG_MAQ, send_msg_2sensor},
        {  MSG_MAQ, check_flag_ACK_not_msg_MAQ_left, LISTEN, init_maq_success},
        {  LISTEN, check_eight_bits_received, LISTEN, received_data_success},
        {  LISTEN, check_msg_received_IRIS_not_msg_checked_eight_bits, CHECK_CRC, check_msg},
        {  CHECK_CRC, check_msg_checked, LISTEN, msg_checked_success},
        {  LISTEN, check_all_msg_received, IDLE, send_XCK_2sensor_stop_show_results_maq},
        { -1, NULL, -1, NULL },
    };
}
int
calculate_CRC(int numberone, int numbertwo){
	return numberone-numbertwo;
}
