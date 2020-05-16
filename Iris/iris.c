/** File name   :iris.c
  * Description :fsm functions and interruption function
  */

// Includes
#include "iris.h"

// Private variables
static pthread_mutex_t mutex;
static TipoFlags jarvan;
static TipoIris iris;

static char iaq_message[3][20] = {"5000","20","03"};
static char maq_message[5][20] = {"5000", "20", "08", "StartCond", "1234"};
static char mrs_message[5][20] = {"5000", "20", "50", "StartCond", "1234"};

char CRC_aux[20];
char res[20];

// FSM states
enum states {
  SLEEP, // initial state
  IDLE,
	MSG_IAQ,
	MSG_MAQ,
  MSG_MRS,
  LISTEN_MAQ,
  LISTEN_MRS,
	CHECK_CRC_MAQ,
  CHECK_CRC_MRS
};

// Private functon prototypes
char* calculate_CRC(char* numberone, char* numbertwo);
char* getDate();

// Checking functions
static int
check_button_on (fsm_t* this)
{
  pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.button_on);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_button_off(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.button_off);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_time_on_not_init(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.time_on && !jarvan.initialized);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_msg_IAQ(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.ack && !jarvan.msg_IAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_not_msg_IAQ(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.ack && jarvan.msg_IAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_init_timeout_measure(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.initialized && (jarvan.timeout_MAQ || jarvan.MAQ_now));
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_init_measure_mrs(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.initialized && jarvan.MRS_now);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_msg_MAQ_left(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.ack && !jarvan.msg_MAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_msg_MRS_left(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.ack && !jarvan.msg_MRS_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_not_msg_MAQ_left(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.ack && jarvan.msg_MAQ_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_flag_ACK_not_msg_MRS_left(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.ack && jarvan.msg_MRS_left);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_bits_received_and_not_all_msg_received(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.bits_received && !jarvan.all_msg_received);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_all_msg_received_IRIS_not_msg_checked(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.all_msg_received && !jarvan.msg_checked);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_msg_checked(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.msg_checked);
  pthread_mutex_unlock (&mutex);
  return result;
}

static int
check_all_msg_received_and_checked(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  int result = 0;
  result = (jarvan.all_msg_received && jarvan.msg_checked);
  pthread_mutex_unlock (&mutex);
  return result;
}

// Function for the FSM

/**
* @brief Starts system
*/
static void
power_on(fsm_t* this)
{
  TipoIris *p_iris;
	p_iris = (TipoIris*)(this->user_data);

  printf("\nPowering on ...\n\n");
	fflush(stdout);

  tmr_startms((tmr_t*)(p_iris->tmr_on), TIME_ON); // a sufficient time has to go by until sensor can receive instructions

  pthread_mutex_lock (&mutex);
  p_iris->state = 1; // turning this to 1 means iris is ON
	jarvan.button_on = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Stops system
*/
static void
power_off(fsm_t* this)
{
  TipoIris *p_iris;
	p_iris = (TipoIris*)(this->user_data);

  printf("\nShutting off...\n");
	fflush(stdout);

  tmr_startms((tmr_t*)(p_iris->tmr_MAQ), 100*TIME_MAQ);

  pthread_mutex_lock(&mutex);
  char message[20] = "PowerOff"; // Wouldnt happen in real system but this "tells" the sensor its off
  write(p_iris->socket_desc,&message,20);
  pthread_mutex_unlock(&mutex);

  pthread_mutex_lock (&mutex);
  p_iris->state = 0; // turning this to 0 means iris is OFF
  jarvan.initialized = 0;
	jarvan.button_off = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Prepares Iris to send IAQ commands
*/
static void
iaq_start(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nSending IAQ command\n\n");
	fflush(stdout);

  p_iris->address = &(iaq_message[0][0]);
  p_iris->length_next_msg = 3;
  p_iris->num_sent = 0;

  pthread_mutex_lock(&mutex);
  char message[20] = "StartCond";
  write(p_iris->socket_desc,&message,20);
  pthread_mutex_unlock(&mutex);

  //printf("Se envia StartCond\n");
  fflush(stdout);

  pthread_mutex_lock (&mutex);
	jarvan.time_on = 0;
  jarvan.ack = 1; //mandatory to enter un sending loop in MSG_IAQ
	pthread_mutex_unlock (&mutex);
}

/**
* @brief IAQ commands sended and everything has gone right
*/
static void
iaq_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nIAQ success\n\n");
	fflush(stdout);

  pthread_mutex_lock (&mutex);
  char message[20] = "StopCond";
  write(p_iris->socket_desc,&message,20);
  pthread_mutex_unlock (&mutex);

  //printf("Se envia StopCond\n");
  fflush(stdout);

  tmr_startms((tmr_t*)(p_iris->tmr_MAQ), TIME_MAQ);// sets timer for next MAQ petition

  pthread_mutex_lock (&mutex);
	jarvan.ack = 0;
  jarvan.msg_IAQ_left = 0;
  jarvan.initialized = 1; //IAQ must only be sent one time everytime sensor its powered on
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Prepares Iris to send MAQ commands
*/
static void
maq_start(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nSending MAQ command\n\n");
	fflush(stdout);

  tmr_startms((tmr_t*)(p_iris->tmr_MAQ), TIME_MAQ);

  p_iris->address = &(maq_message[0][0]);
  p_iris->length_next_msg = 5;
  p_iris->num_sent = 0;

  pthread_mutex_lock (&mutex);
  char message[20] = "StartCond";
  write(p_iris->socket_desc,&(message),20);
  pthread_mutex_unlock (&mutex);

  //printf("Se envia StartCond\n");
  fflush(stdout);

  pthread_mutex_lock (&mutex);
	jarvan.timeout_MAQ = 0;
  jarvan.MAQ_now = 0;
  jarvan.ack = 1; //mandatory to enter un sending loop in MSG_MAQ
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Prepares Iris to send MRS commands
*/
static void
mrs_start(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  printf("\nSending MRS command\n\n");
	fflush(stdout);

  tmr_startms((tmr_t*)(p_iris->tmr_MAQ), TIME_MAQ);

  p_iris->address = &(mrs_message[0][0]);
  p_iris->length_next_msg = 5;
  p_iris->num_sent = 0;

  pthread_mutex_lock (&mutex);
  char message[20] = "StartCond";
  write(p_iris->socket_desc,&(message),20);
  pthread_mutex_unlock (&mutex);

  //printf("Se envia StartCond\n");
  fflush(stdout);

  pthread_mutex_lock (&mutex);
  jarvan.MRS_now = 0;
  jarvan.ack = 1; //mandatory to enter un sending loop in MSG_MAQ
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Send messages to sensor through socket
*/
static void
send_msg_2sensor(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  pthread_mutex_lock (&mutex);
  write(p_iris->socket_desc,(p_iris->address),20);
  pthread_mutex_unlock (&mutex);

  //printf("Se envia %s\n",(p_iris->address));
  fflush(stdout);

  p_iris->address += 20;
  p_iris->num_sent += 1;

  if((p_iris->num_sent) >= (p_iris->length_next_msg)){ //if all messages have been sent activates flag
    if(this->current_state == MSG_IAQ){
      pthread_mutex_lock (&mutex);
      jarvan.msg_IAQ_left = 1;
    	pthread_mutex_unlock (&mutex);
    }else if(this->current_state == MSG_MAQ){
      pthread_mutex_lock (&mutex);
      jarvan.msg_MAQ_left = 1;
    	pthread_mutex_unlock (&mutex);
    }else if(this->current_state == MSG_MRS){
      pthread_mutex_lock (&mutex);
      jarvan.msg_MRS_left = 1;
    	pthread_mutex_unlock (&mutex);
    }
  }
  pthread_mutex_lock (&mutex);
  jarvan.ack = 0;
  pthread_mutex_unlock (&mutex);
}

/**
* @brief MAQ commands have been received, waiting for the sensor to send the measures
*/
static void
init_maq_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  //printf("\nMAQ command sent, waiting measures\n\n");
	fflush(stdout);

  p_iris->num_msg = 0; //this counter will increase as measures are received

  pthread_mutex_lock (&mutex);
	jarvan.msg_MAQ_left = 0;
  jarvan.ack = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief MRS commands have been received, waiting for the sensor to send the measures
*/
static void
init_mrs_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  //printf("\nMRS command sent, waiting measures\n\n");
	fflush(stdout);

  p_iris->num_msg = 0; //this counter will increase as measures are received

  pthread_mutex_lock (&mutex);
	jarvan.msg_MRS_left = 0;
  jarvan.ack = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief storages the measures sent by the sensor
*/
static void
received_data_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  strcpy(p_iris->measures[p_iris->num_msg], p_iris->receiver);

  //printf("Recibido vale %s\n", p_iris->measures[p_iris->num_msg]);
  fflush(stdout);

  p_iris->num_msg += 1;

  if(p_iris->num_msg <= 5){ //ack is not needed for the last message
    pthread_mutex_lock (&mutex);
    char message[20] = "ACK";
    write(p_iris->socket_desc,&message,20);
    pthread_mutex_unlock (&mutex);

    //printf("Se envia ACK\n");
    fflush(stdout);
  }

  if(p_iris->num_msg == 6){ //checks if every message has been received
    pthread_mutex_lock (&mutex);
  	jarvan.all_msg_received = 1;
  	pthread_mutex_unlock (&mutex);
  }

  pthread_mutex_lock (&mutex);
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief enter to the checking phase
*/
static void
check_msg(fsm_t* this)
{
  pthread_mutex_lock (&mutex);
  jarvan.msg_checked = 1;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief checks if CRC received value agrees to the CRC calculated with the measures received
*/
static void
msg_checked_success(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  char CRC1[20];
  char CRC2[20];

  strcpy(CRC1, calculate_CRC(p_iris->measures[0],p_iris->measures[1]));
  strcpy(CRC2, calculate_CRC(p_iris->measures[3],p_iris->measures[4]));

  if(atoi(CRC1)==atoi(p_iris->measures[2]) && atoi(CRC2)==atoi(p_iris->measures[5])){
    printf("\nMSG checked...correct\n\n");
  	fflush(stdout);
  }else{
    printf("\nMSG checked...incorrect\n\n");
  	fflush(stdout);
  }
}

/**
* @brief ends MAQ procedure
*/
static void
send_XCK_2sensor_stop_show_results_maq(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  pthread_mutex_lock(&mutex);
  char message2[20] = "StopCond";
  write(p_iris->socket_desc,&message2,20);
  pthread_mutex_unlock(&mutex);

  //printf("Se envia StopCond\n\n");
  fflush(stdout);

  //printf("CO2 = %s%s ppm\n", p_iris->measures[0],p_iris->measures[1]);
  fflush(stdout);
  //printf("TVOC = %s%s ppb\n\n", p_iris->measures[3],p_iris->measures[4]);
  fflush(stdout);

  char str[1024] = "Info from "; // message that would be sent to GW
  strcat(str, p_iris->id);
  strcat(str, " at ");
  strcat(str, getDate());
  strcat(str, "\nCO2 = ");
  strcat(str,p_iris->measures[0]);
  strcat(str,p_iris->measures[1]);
  strcat(str, " ppm\nTVOC = ");
  strcat(str,p_iris->measures[3]);
  strcat(str,p_iris->measures[4]);
  strcat(str, " ppb\n");

  /*pthread_mutex_lock (&mutex);
  write(p_iris->socket_desc_GW, str, sizeof(str));
  pthread_mutex_unlock (&mutex);*/
  printf("******Message that would be sent to GW*******\n");
  printf("%s",str);
  printf("*********************************************\n");

  pthread_mutex_lock (&mutex);
  jarvan.all_msg_received = 0;
  jarvan.msg_checked = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief ends MRS procedure
*/
static void
send_XCK_2sensor_stop_show_results_mrs(fsm_t* this)
{
  TipoIris *p_iris;
  p_iris = (TipoIris*)(this->user_data);

  pthread_mutex_lock(&mutex);
  char message2[20] = "StopCond";
  write(p_iris->socket_desc,&message2,20);
  pthread_mutex_unlock(&mutex);

  //printf("Se envia StopCond\n\n");
  fflush(stdout);

  //printf("H2 = %s%s ppm\n", p_iris->measures[0],p_iris->measures[1]);
  fflush(stdout);
  //printf("Ethanol = %s%s ppm\n\n", p_iris->measures[3],p_iris->measures[4]);
  fflush(stdout);

  char str[1024] = "Info from ";
  strcat(str, p_iris->id);
  strcat(str, " at ");
  strcat(str, getDate());
  strcat(str, "\nH2 = ");
  strcat(str,p_iris->measures[0]);
  strcat(str,p_iris->measures[1]);
  strcat(str, " ppm\nEthanol = ");
  strcat(str,p_iris->measures[3]);
  strcat(str,p_iris->measures[4]);
  strcat(str, " ppm\n");

  /*pthread_mutex_lock (&mutex);
  write(p_iris->socket_desc_GW, str, sizeof(str));
  pthread_mutex_unlock (&mutex);*/

  printf("******Message that would be sent to GW*******\n");
  printf("%s",str);
  printf("*********************************************\n");

  pthread_mutex_lock (&mutex);
  jarvan.all_msg_received = 0;
  jarvan.msg_checked = 0;
	pthread_mutex_unlock (&mutex);
}


// ISR
// Interruption functions for the timers, the observer and the button interruptions
static void
initial_timer (union sigval value){
	pthread_mutex_lock (&mutex);
	jarvan.time_on = 1;
	pthread_mutex_unlock (&mutex);
  tmr_startms((tmr_t*)(iris.tmr_on), 10000*TIME_ON);
}

static void
maq_timer (union sigval value){
	pthread_mutex_lock (&mutex);
	jarvan.timeout_MAQ = 1;
	pthread_mutex_unlock (&mutex);
}

void
button_onoff_isr(void) {
  if (iris.state){
    pthread_mutex_lock (&mutex);
  	jarvan.button_off = 1;
  	pthread_mutex_unlock (&mutex);
  }else{
    pthread_mutex_lock (&mutex);
  	jarvan.button_on = 1;
  	pthread_mutex_unlock (&mutex);
  }
}

void
button_MAQnow_isr() {
  pthread_mutex_lock (&mutex);
  //printf("MAQ forced\n");
  jarvan.MAQ_now = 1;
  pthread_mutex_unlock (&mutex);
}

void
button_MRSnow_isr() {
  pthread_mutex_lock (&mutex);
  //printf("MRS forced\n");
  jarvan.MRS_now = 1;
  pthread_mutex_unlock (&mutex);
}

void
ack_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.ack = 1;
	pthread_mutex_unlock (&mutex);
}

void
xck_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.xck = 1;
	pthread_mutex_unlock (&mutex);
}

void
bits_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.bits_received = 1;
	pthread_mutex_unlock (&mutex);
}

void
new_msg (char* msg){
	strcpy(iris.receiver, msg);
}

// Initialization functions
int
iris_init(TipoIris *p_iris, TipoFlags *flags)
{
  jarvan = *flags;
  iris = *p_iris;

	pthread_mutex_init(&mutex, NULL);

  iris.state = 0;
  strcpy(iris.id, "Quirofano 1");
  iris.tmr_MAQ = tmr_new (maq_timer);
  iris.tmr_on = tmr_new(initial_timer);
  strcpy(iris.I2C_ADDRESS_IRIS, OWN_ADDRESS);
  strcpy(iris.I2C_ADDRESS_SENSOR, SENSOR_ADDRESS);
  memset(iris.measures,0,sizeof(iris.measures));
  memset(iris.receiver,0,sizeof(iris.receiver));
  iris.address = NULL;
  iris.length_next_msg = 0;
  iris.num_msg = 0;
  iris.num_sent = 0;

	//printf("\nIris init complete!\n");
	fflush(stdout);

	return 0;
}


fsm_t*
fsm_new_iris ()
{
    static fsm_trans_t iris_tt[] = {
        {  SLEEP, check_button_on, IDLE, power_on},
        {  IDLE, check_button_off, SLEEP, power_off},
        {  IDLE, check_time_on_not_init, MSG_IAQ, iaq_start},
        {  MSG_IAQ, check_flag_ACK_not_msg_IAQ, IDLE, iaq_success},
        {  MSG_IAQ, check_flag_ACK_msg_IAQ, MSG_IAQ, send_msg_2sensor},
        {  IDLE, check_init_timeout_measure, MSG_MAQ, maq_start},
        {  IDLE, check_init_measure_mrs, MSG_MRS, mrs_start},
        {  MSG_MAQ, check_flag_ACK_msg_MAQ_left, MSG_MAQ, send_msg_2sensor},
        {  MSG_MRS, check_flag_ACK_msg_MRS_left, MSG_MRS, send_msg_2sensor},
        {  MSG_MAQ, check_flag_ACK_not_msg_MAQ_left, LISTEN_MAQ, init_maq_success},
        {  MSG_MRS, check_flag_ACK_not_msg_MRS_left, LISTEN_MRS, init_mrs_success},
        {  LISTEN_MAQ, check_bits_received_and_not_all_msg_received, LISTEN_MAQ, received_data_success},
        {  LISTEN_MRS, check_bits_received_and_not_all_msg_received, LISTEN_MRS, received_data_success},
        {  LISTEN_MAQ, check_all_msg_received_IRIS_not_msg_checked, CHECK_CRC_MAQ, check_msg},
        {  LISTEN_MRS, check_all_msg_received_IRIS_not_msg_checked, CHECK_CRC_MRS, check_msg},
        {  CHECK_CRC_MAQ, check_msg_checked, LISTEN_MAQ, msg_checked_success},
        {  CHECK_CRC_MRS, check_msg_checked, LISTEN_MRS, msg_checked_success},
        {  LISTEN_MAQ, check_all_msg_received_and_checked, IDLE, send_XCK_2sensor_stop_show_results_maq},
        {  LISTEN_MRS, check_all_msg_received_and_checked, IDLE, send_XCK_2sensor_stop_show_results_mrs},
        { -1, NULL, -1, NULL },
    };

    return fsm_new (SLEEP, iris_tt, &iris);
}

/**
* @brief calculates CRC expected
* @params messages received
* @returns the CRC calculated with the messages introduced in the params
*/
char*
calculate_CRC(char* numberone, char* numbertwo){
  memset(CRC_aux,0,sizeof(CRC_aux));
	int aux = abs(atoi(numberone)-atoi(numbertwo));
	sprintf(CRC_aux, "%d", aux);
	return CRC_aux;
}

char* getDate(){
  time_t now;
  time(&now);
  struct tm tm = *localtime(&now);
  sprintf(res,"%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  return res;
}
