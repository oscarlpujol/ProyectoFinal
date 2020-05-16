/** File name   :sensor.c
  * Description :intializes and starts the iris system and the interruption
  */

// Includes
#include "sensor.h"

// Private variables
static pthread_mutex_t mutex;
static TipoFlags jarvan;
static TipoSensor sgp30;

char CRC_aux[20];

char iaq[20] = "03";
char maq[20] = "08";
char mrs[20] = "50";

// FSM states
enum states {
	IDLE, // initial state
	WAIT_8BITS_1,
	WAIT_8BITS_2,
	MAQ,
	MSG_MAQ,
	MRS,
	MSG_MRS
};

// Private functon prototypes
char* calculate_CRC(int numberone, int numbertwo);

// Flag checking fucntions
static int
check_start_and_bits (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.start_cond && jarvan.bits_received);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_bits_not_proc1 (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.bits_received && !jarvan.process1);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_bits_not_proc2 (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.bits_received && !jarvan.process2);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_IAQ_and_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.IAQ && jarvan.stop_cond && !jarvan.MAQ && !jarvan.bits_received && !jarvan.MRS  && !jarvan.incorrect_command);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_I2C_address(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = jarvan.I2C_address_wrong;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_incorrect_command(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.incorrect_command && !jarvan.correct_command && !jarvan.bits_received);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_correct_command(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.correct_command && !jarvan.incorrect_command);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_MAQ (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.MAQ && !jarvan.bits_received && !jarvan.MRS && !jarvan.IAQ && !jarvan.incorrect_command);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_MRS (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.MAQ && !jarvan.bits_received && jarvan.MRS && !jarvan.IAQ && !jarvan.incorrect_command);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_ACK_and_MAQ_left (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.ack && jarvan.msg_MAQ_left && !jarvan.xck);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_ACK_and_MRS_left (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (jarvan.ack && jarvan.msg_MRS_left && !jarvan.xck);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_XCK_and_notMAQ_and_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.msg_MAQ_left && jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_XCK_and_notMRS_and_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (!jarvan.msg_MRS_left && jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

// Function for the FSM

/**
* @brief Checks if the address sent is correct
*/
static void
I2C_address_success (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	//printf("\nDireccion I2C recibida con exito!\n\n");
	//printf("I2C address = %s \n", p_sgp30->receiver);
	fflush(stdout);

	if(strcmp(p_sgp30->receiver,p_sgp30->I2C_ADDRESS_SENSOR) != 0) //strcmp returns 0 if both strings are equal
	{
		//printf("Dirección incorrecta!\n");
		fflush(stdout);

		pthread_mutex_lock (&mutex);
		jarvan.I2C_address_wrong = 1;
		pthread_mutex_unlock (&mutex);
	}else{
		//printf("Dirección correcta!\n");
		fflush(stdout);
	}

	pthread_mutex_lock (&mutex);
	jarvan.start_cond = 0;
	jarvan.bits_received = 0;
	jarvan.incorrect_command = 0;
	jarvan.correct_command = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Successufully received the IAQ commands
*/
static void
IAQ_received (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("\nSensor has received IAQ order\n\n");
	fflush(stdout);

	tmr_startms((tmr_t*)(p_sgp30->tmr_real_measures), REALMEASURES_TIME); //for REALMEASURES_TIME, measures from the sensor are not real (happens everytime sensor is restarted)

	pthread_mutex_lock (&mutex);
	jarvan.IAQ = 0;
	jarvan.bits_received = 0;
	jarvan.stop_cond = 0;
	jarvan.process1 = 0;
	jarvan.process2 = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Sensor address sent by Iris doesnt agree with actual value
*/
static void
wrong_I2C_address (fsm_t* this)
{
	//printf("\nI2C address sent by IRIS is wrong\n");
	//printf("\nSensor currently waiting for new commands\n");
	fflush(stdout);

	pthread_mutex_lock (&mutex);
	jarvan.I2C_address_wrong = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief MAQ commands have been received. MAQ measures calculated
*/
static void
MAQ_received (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	char aux0[20] = "04";
	char aux1[20] = "00";
	char CRC[20]= "4";

	printf("\n\nMAQ command has been received\n");
	fflush(stdout);

	if((p_sgp30->realmeasures) == 0){ //as said before during REALMEASURES_TIME measures are set to a fixed value
		strcpy(p_sgp30->measures[0], aux0);
		strcpy(p_sgp30->measures[1], aux1);
		strcpy(p_sgp30->measures[2], CRC);
		strcpy(p_sgp30->measures[3], aux1);
		strcpy(p_sgp30->measures[4], aux1);
		strcpy(p_sgp30->measures[5], aux1);
	}
	else{ //measures are simulated with random numbers

		int aux11_rand = rand() % 99;
		int aux21_rand = rand() % 99;
		int aux12_rand = rand() % 99;
		int aux22_rand = rand() % 99;

		char aux11 [20];
		char aux21 [20];
		char aux12 [20];
		char aux22 [20];
		char CRC1[20];
		char CRC2[20];

		strcpy(CRC1, calculate_CRC(aux11_rand,aux12_rand));
		strcpy(CRC2, calculate_CRC(aux21_rand,aux22_rand));

		sprintf(aux11, "%d", aux11_rand);
		sprintf(aux21, "%d", aux21_rand);
		sprintf(aux12, "%d", aux12_rand);
		sprintf(aux22, "%d", aux22_rand);

		strcpy(p_sgp30->measures[0], aux11);
		strcpy(p_sgp30->measures[1], aux12);
		strcpy(p_sgp30->measures[2], CRC1);
		strcpy(p_sgp30->measures[3], aux21);
		strcpy(p_sgp30->measures[4], aux22);
		strcpy(p_sgp30->measures[5], CRC2);
	}

	//printf("CO2 medido vale %s%s \n", p_sgp30->measures[0],p_sgp30->measures[1]);
	//printf("TVOC medido vale %s%s \n", p_sgp30->measures[3], p_sgp30->measures[4]);
	fflush(stdout);

	char message[20] = "ACK";
	pthread_mutex_lock(&mutex_socket);
	write(p_sgp30->socket_desc,&message,20);
	pthread_mutex_unlock(&mutex_socket);

	pthread_mutex_lock (&mutex);
	jarvan.MAQ = 0;
	jarvan.process1 = 0;
	jarvan.process2 = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief MRS commands have been received. MRS measures calculated
*/
static void
MRS_received (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	char aux0[20] = "04";
	char aux1[20] = "00";
	char CRC[20]= "4";

	printf("\n\nMRS command has been received\n");
	fflush(stdout);

	if((p_sgp30->realmeasures) == 0){
		strcpy(p_sgp30->measures[0], aux0);
		strcpy(p_sgp30->measures[1], aux1);
		strcpy(p_sgp30->measures[2], CRC);
		strcpy(p_sgp30->measures[3], aux1);
		strcpy(p_sgp30->measures[4], aux1);
		strcpy(p_sgp30->measures[5], aux1);
	}
	else{

		int aux11_rand = rand() % 99;
		int aux21_rand = rand() % 99;
		int aux12_rand = rand() % 99;
		int aux22_rand = rand() % 99;

		char aux11 [20];
		char aux21 [20];
		char aux12 [20];
		char aux22 [20];
		char CRC1[20];
		char CRC2[20];

		strcpy(CRC1, calculate_CRC(aux11_rand,aux12_rand));
		strcpy(CRC2, calculate_CRC(aux21_rand,aux22_rand));

		sprintf(aux11, "%d", aux11_rand);
		sprintf(aux21, "%d", aux21_rand);
		sprintf(aux12, "%d", aux12_rand);
		sprintf(aux22, "%d", aux22_rand);

		strcpy(p_sgp30->measures[0], aux11);
		strcpy(p_sgp30->measures[1], aux12);
		strcpy(p_sgp30->measures[2], CRC1);
		strcpy(p_sgp30->measures[3], aux21);
		strcpy(p_sgp30->measures[4], aux22);
		strcpy(p_sgp30->measures[5], CRC2);
	}

	//printf("H2 medido vale %s%s \n", p_sgp30->measures[0],p_sgp30->measures[1]);
	//printf("Ethanol medido vale %s%s \n", p_sgp30->measures[3], p_sgp30->measures[4]);
	fflush(stdout);

	char message[20] = "ACK";
	pthread_mutex_lock(&mutex_socket);
	write(p_sgp30->socket_desc,&message,20);
	pthread_mutex_unlock(&mutex_socket);

	pthread_mutex_lock (&mutex);
	jarvan.MRS = 0;
	jarvan.process1 = 0;
	jarvan.process2 = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Iris I2C address received
*/
static void
I2C_address_received (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	//printf("Direccion I2C de IRIS recibida con exito!\n\n");
	//printf("IRIS I2C address = %s \n", p_sgp30->receiver);
	fflush(stdout);

	strcpy(p_sgp30->I2C_ADDRESS_IRIS, p_sgp30->receiver);

	pthread_mutex_lock (&mutex);
	jarvan.ack = 1;
	jarvan.msg_MAQ_left = 1;
	jarvan.msg_MRS_left = 1;
	jarvan.start_cond = 0;
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}

static void
send_msg_2IRIS (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	char measure[20]; strcpy(measure,(p_sgp30->measures[(p_sgp30->address)]));
	//printf("Se envia %s\n", measure);
	fflush(stdout);
	pthread_mutex_lock(&mutex_socket);
	write(p_sgp30->socket_desc, &measure,20);
	pthread_mutex_unlock(&mutex_socket);
	p_sgp30->address += 1;

	if(p_sgp30->address == 6){
		if(this->current_state == MSG_MAQ){
      pthread_mutex_lock (&mutex);
      jarvan.msg_MAQ_left = 0;
    	pthread_mutex_unlock (&mutex);
    }else if(this->current_state == MSG_MRS){
      pthread_mutex_lock (&mutex);
      jarvan.msg_MRS_left = 0;
    	pthread_mutex_unlock (&mutex);
    }
	}

	pthread_mutex_lock (&mutex);
	jarvan.ack = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief MAQ measures sent, all measures from the structure are set to 0
*/
static void
MAQ_success (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("\nMAQ command success\n");
	fflush(stdout);

	for (int i = 0; i <= 5; ++i){
    p_sgp30->address -= 1;
    strcpy(p_sgp30->measures[(p_sgp30->address)], "0");
  }

	pthread_mutex_lock (&mutex);
	jarvan.stop_cond = 0;
	jarvan.xck = 0;
	jarvan.msg_MAQ_left = 1;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief MRS measures sent, all measures from the structure are set to 0
*/
static void
MRS_success (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("\nMRS command success\n");
	fflush(stdout);


	for (int i = 0; i <= 5; ++i){
    p_sgp30->address -= 1;
    strcpy(p_sgp30->measures[(p_sgp30->address)], "0");
  }

	pthread_mutex_lock (&mutex);
	jarvan.stop_cond = 0;
	jarvan.xck = 0;
	jarvan.msg_MRS_left = 1;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Commands sent by Iris do not agree with known commands
*/
static void
wrong_command(fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	char message[20] = "XCK";
	pthread_mutex_lock(&mutex_socket);
	write(p_sgp30->socket_desc,&message,20);
	pthread_mutex_unlock(&mutex_socket);
	//printf("Command not recognised\n");

	pthread_mutex_lock (&mutex);
	jarvan.incorrect_command = 0;
	jarvan.start_cond = 0;
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief First command is correct
*/
static void
correct_command (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	jarvan.correct_command = 0;
	jarvan.MAQ = 0;
	jarvan.MRS = 0;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Check if the first command message fits with any command known
*/
static void
process_bits_1 (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	char aux[20] = "20";

	//printf("El primer mensaje de la orden vale %s\n", p_sgp30->receiver);
	fflush(stdout);

	if(!strcmp((p_sgp30->receiver),aux)){
		fflush(stdout);
		pthread_mutex_lock (&mutex);
		jarvan.correct_command = 1;
		pthread_mutex_unlock (&mutex);
	}else{
		pthread_mutex_lock (&mutex);
		jarvan.incorrect_command = 1;
		pthread_mutex_unlock (&mutex);
	}

	pthread_mutex_lock (&mutex);
	jarvan.bits_received = 0;
	jarvan.process1 = 1;
	pthread_mutex_unlock (&mutex);
}

/**
* @brief Check if the second command message fits with any command known
*/
static void
process_bits_2 (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	//printf("El segundo mensaje de la orden vale %s\n", p_sgp30->receiver);
	fflush(stdout);

	if(!strcmp(p_sgp30->receiver,iaq)){
		pthread_mutex_lock (&mutex);
		jarvan.IAQ = 1;
		pthread_mutex_unlock (&mutex);
	}else if(!strcmp(p_sgp30->receiver,maq)){
		pthread_mutex_lock (&mutex);
		jarvan.MAQ = 1;
		pthread_mutex_unlock (&mutex);
	}else if(!strcmp(p_sgp30->receiver,mrs)){
		pthread_mutex_lock (&mutex);
		jarvan.MRS = 1;
		pthread_mutex_unlock (&mutex);
	}else{
		pthread_mutex_lock (&mutex);
		jarvan.incorrect_command = 1;
		pthread_mutex_unlock (&mutex);
	}

	pthread_mutex_lock (&mutex);
	jarvan.bits_received = 0;
	jarvan.process2 = 1;
	pthread_mutex_unlock (&mutex);
}

// ISR
// Interruption functions for the timers and the observer
static void
initial_timer (union sigval value){
	pthread_mutex_lock (&mutex);
	sgp30.realmeasures  = 1;
	pthread_mutex_unlock (&mutex);
}

void
start_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.start_cond = 1;
	pthread_mutex_unlock (&mutex);
}

void
stop_isr() {
	pthread_mutex_lock (&mutex);
	jarvan.stop_cond = 1;
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
	strcpy(sgp30.receiver, msg);
}

void
timeout_isr(){
	pthread_mutex_lock (&mutex);
	jarvan.timeout = 1;
	pthread_mutex_unlock (&mutex);
}

void
powerOff_isr(){
	pthread_mutex_lock (&mutex);
	sgp30.realmeasures = 0;
	pthread_mutex_unlock (&mutex);
}

// Initialization functions
int
sensor_init(TipoSensor *p_sgp30, TipoFlags *flags)
{
	jarvan = *flags;
	sgp30 = *p_sgp30;

	char aux[20] = OWN_ADDRESS;

	sgp30.realmeasures = 0;
	sgp30.tmr_real_measures = tmr_new (initial_timer); // timer starter when IAQ, turns realmeasures into 1
	strcpy(sgp30.I2C_ADDRESS_IRIS, "0"); // IRIS I2C address
	strcpy(sgp30.I2C_ADDRESS_SENSOR, aux); // SENSOR I2C address
	memset(sgp30.measures,0,sizeof(sgp30.measures));
	memset(sgp30.receiver,0, sizeof(sgp30.receiver));
	pthread_mutex_init(&mutex, NULL);
	sgp30.address = 0;

	//printf("\nSensor init complete!\n");
	fflush(stdout);

	return 0;
}

fsm_t*
fsm_new_sensor ()
{
    static fsm_trans_t sensor_tt[] = {
        {  IDLE, check_start_and_bits, WAIT_8BITS_1, I2C_address_success},
        {  WAIT_8BITS_2, check_IAQ_and_stop, IDLE, IAQ_received},
				{  WAIT_8BITS_1, check_incorrect_command, IDLE, wrong_command},
				{  WAIT_8BITS_1, check_bits_not_proc1, WAIT_8BITS_1, process_bits_1},
				{  WAIT_8BITS_2, check_bits_not_proc2, WAIT_8BITS_2, process_bits_2},
				{  WAIT_8BITS_2, check_incorrect_command, IDLE, wrong_command},
        {  WAIT_8BITS_1, check_I2C_address, IDLE, wrong_I2C_address},
				{  WAIT_8BITS_1, check_correct_command, WAIT_8BITS_2, correct_command},
        {  WAIT_8BITS_2, check_MAQ, MAQ, MAQ_received},
        {  WAIT_8BITS_2, check_MRS, MRS, MRS_received},
        {  MAQ, check_start_and_bits, MSG_MAQ, I2C_address_received},
				{  MRS, check_start_and_bits, MSG_MRS, I2C_address_received},
        {  MSG_MAQ, check_ACK_and_MAQ_left, MSG_MAQ, send_msg_2IRIS},
        {  MSG_MAQ, check_XCK_and_notMAQ_and_stop, IDLE, MAQ_success},
				{  MSG_MRS, check_ACK_and_MRS_left, MSG_MRS, send_msg_2IRIS},
        {  MSG_MRS, check_XCK_and_notMRS_and_stop, IDLE, MRS_success},
        { -1, NULL, -1, NULL },
    };

		return fsm_new (IDLE, sensor_tt, &sgp30);
}

/**
* @brief calculates CRC expected
* @params measures
* @returns the CRC calculated with the measures introduced in the params
*/
char*
calculate_CRC(int numberone, int numbertwo){
	memset(CRC_aux,0,sizeof(CRC_aux));
	int aux = abs(numberone-numbertwo);
	sprintf(CRC_aux, "%d", aux);
	return CRC_aux;
}
