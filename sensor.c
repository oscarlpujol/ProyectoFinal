/*

*/

#include "sensor.h"

static pthread_mutex_t mutex;
static TipoFlags jarvan;
static TipoSensor sgp30;

int calculate_CRC(int numberone, int numbertwo);

enum states {
	IDLE, // initial state
	WAIT_8BITS_1,
	WAIT_8BITS_2,
	MAQ,
	MSG_MAQ/*,
	MRS,
	MSG_MRS*/
}

// Int

static int
check_start_and_bits (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.start_cond && jarvan.bits_received);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_bits (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.bits_received);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_IAQ_and_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.start_cond && jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_I2C_address(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = jarvan.I2C_address_wrong;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_incorrect_command(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.incorrect_command || jarvan.timeout);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_correct_command(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = jarvan.correct_command;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_MAQ (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = jarvan.MAQ;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_ACK_and_MAQ_left (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.ack && jarvan.msg_MAQ_left);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_C02_or_TVOC_sent (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.CO2_sent || jarvan.TVOC_sent);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_XCK_and_notMAQ_and_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (jarvan.xck && !jarvan.msg_MAQ_left && jarvan.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

// Void

static void
I2C_address_success (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	pritnf("Direccion I2C recibida con exito!\n");
	printf("I2C address = %d \n", p_sgp30->receiver);
	fflush(stdout);

	if(p_sgp30->receiver == p_sgp30->I2C_ADDRESS_SENSOR)
	{
		pthread_mutex_lock (&mutex);
		jarvan.I2C_address_wrong = 1;
		pthread_mutex_unlock (&mutex);
	}

	pthread_mutex_lock (&mutex);
	jarvan.start_cond = 0;
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}

static void
IAQ_received (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	printf("\nSensor has received IAQ order\n");
	fflush(stdout);

	tmr_startms((tmr_t*)(p_sgp30->tmr_real_measures), REALMEASURES_TIME);

	pthread_mutex_lock (&mutex);
	jarvan.IAQ = 0;
	jarvan.stop_cond = 0;
	pthread_mutex_unlock (&mutex);
}

static void
wrong_I2C_address (fsm_t* this)
{
	printf("\nI2C address sent by IRIS is wrong\n");
	printf("\nSensor currently waiting for new commands\n");
	fflush(stdout);

	pthread_mutex_lock (&mutex);
	jarvan.I2C_address_wrong = 0;
	pthread_mutex_unlock (&mutex);
}

static void
MAQ_received (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	int CO2, TVOC;

	printf("\nMAQ command has been received\n");
	fflush(stdout);

	if((p_sgp30->realmeasures) == 0){
		p_sgp30->measures[0] = 400;
		p_sgp30->measures[1] = 400;
		p_sgp30->measures[3] = 400;
		p_sgp30->measures[4] = 400;
	}
	else{
		CO2 = rand() % 255;
		TVOC = rand() % 255;

		p_sgp30->measures[0] = CO2;
		p_sgp30->measures[1] = CO2;
		p_sgp30->measures[3] = TVOC;
		p_sgp30->measures[4] = TVOC;
	}

	pthread_mutex_lock (&mutex);
	jarvan.MAQ = 0;
	pthread_mutex_unlock (&mutex);
}


static void
I2C_address_received (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	p_sgp30->I2C_ADDRESS_IRIS = *(p_sgp30->receiver);

	pthread_mutex_lock (&mutex);
	jarvan.ack = 0;
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}

static void
send_msg_2IRIS (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	socket_send(&(p_sgp30->measures[(p_sgp30->address)]));
	p_sgp30->address += 1;

	if(p_sgp30->address == 2){
		pthread_mutex_lock (&mutex);
		jarvan.CO2_sent = 1;
		pthread_mutex_unlock (&mutex);
	}

	if(p_sgp30->address == 5){
		pthread_mutex_lock (&mutex);
		jarvan.CO2_sent = 1;
		pthread_mutex_unlock (&mutex);
	}

	pthread_mutex_lock (&mutex);
	jarvan.ack = 0;
	jarvan.msg_MAQ_left = 1;
	pthread_mutex_unlock (&mutex);
}

static void
calculate_sent_CRC (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	int CRC;
	CRC = calculate_CRC(p_sgp30->measures[(p_sgp30->address) -2], p_sgp30->measures[(p_sgp30->address)-1]);
	p_sgp30->measures[(p_sgp30->address)] = CRC;

	socket_send(&(p_sgp30->measures(p_sgp30->address)));
	p_sgp30->address += 1;

	if(p_sgp30_address == 5){
		pthread_mutex_lock (&mutex);
		jarvan.msg_MAQ_left = 0;
		pthread_mutex_unlock (&mutex);
	}

	pthread_mutex_lock (&mutex);
	jarvan.TVOC_sent = 0;
	jarvan.CO2_sent = 0;
	pthread_mutex_unlock (&mutex);
}

static void
MAQ_success (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	for (int i = 0; i <= 5; ++i){
    p_sgp30->address -= 1;
    p_sgp30->measures[(p_sgp30->address)]-> = 0;
  }

	pthread_mutex_lock (&mutex);
	jarvan.stop_cond = 0;
	jarvan.xck = 0;
	jarvan.msg_MAQ_left = 1;
	pthread_mutex_unlock (&mutex);
}

static void
wrong_command(fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	message = "XCK\n";
	socket_send(&(message));
	pritnf("Command not recognised\n")

	pthread_mutex_lock (&mutex);
	jarvan.incorrect_command = 0;
	pthread_mutex_unlock (&mutex);
}

static void
correct_command (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	pthread_mutex_lock (&mutex);
	jarvan.correct_command = 0;
	pthread_mutex_unlock (&mutex);
}

static void
process_bits_1 (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	if((p_sgp30->receiver) == "20"){
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
	pthread_mutex_unlock (&mutex);
}

static void
process_bits_2 (fsm_t* this)
{
	TipoSensor *p_sgp30;
	p_sgp30 = (TipoSensor*)(this->user_data);

	if((p_sgp30->receiver) == "03"){
		pthread_mutex_lock (&mutex);
		jarvan.IAQ = 1;
		pthread_mutex_unlock (&mutex);
	}else if((p_sgp30->receiver) == "08")
		pthread_mutex_lock (&mutex);
		jarvan.MAQ = 1;
		pthread_mutex_unlock (&mutex);
	}else{
		pthread_mutex_lock (&mutex);
		jarvan.incorrect_command = 1;
		pthread_mutex_unlock (&mutex);
	}

	pthread_mutex_lock (&mutex);
	jarvan.bits_received = 0;
	pthread_mutex_unlock (&mutex);
}
/*
Interruption functions
*/

static void
initial_timer (union sigval value){

	pthread_mutex_lock (&mutex);
	sgp30.realmeasures  = 1;
	pthread_mutex_unlock (&mutex);
	tmr_destroy(&(sgp30.tmr_real_measures))
}

int
sensor_init(TipoSensor *p_sgp30, TipoFlags *flags)
{
	jarvan = *flags;
	sgp_30 = *p_sgp30;

	sgp30->realmeasures = 0;
	sgp30->tmr_real_measures = tmr_new (initial_timer); // timer starter when IAQ, turns realmeasures into 1
	sgp30->I2C_ADDRESS_IRIS = 0; // IRIS I2C address
	sgp30->I2C_ADDRESS_SENSOR = OWN_ADDRESS; // SENSOR I2C address
	sgp30->measures [6] = {0,0,0,0,0,0};
	pthread_mutex_init(&mutex, NULL);


	printf("\nSystem init complete!\n");
	fflush(stdout);

	return 0;
}


fsm_t*
fsm_new_sensor (/*int* validp, int pir, int alarm*/)
{
    static fsm_trans_t alarm_tt[] = {
        {  IDLE, check_start_and_bits, WAIT_8BITS_1, I2C_address_success},
        {  WAIT_8BITS_2, check_IAQ_and_stop, IDLE, IAQ_received},
				{  WAIT_8BITS_1, check_incorrect_command, IDLE, wrong_command},
				{  WAIT_8BITS_1, check_bits, WAIT_8BITS_1, process_bits_1},
				{  WAIT_8BITS_2, check_bits, WAIT_8BITS_2, process_bits_2},
				{  WAIT_8BITS_2, check_incorrect_command, IDLE, wrong_command},
        {  WAIT_8BITS_1, check_I2C_address, IDLE, wrong_I2C_address},
				{  WAIT_8BITS_1, check_correct_command, WAIT_8BITS_2, correct_command},
        {  WAIT_8BITS_2, check_MAQ, MAQ, MAQ_received},
        //{  WAIT_16BITS, check_MRS, MRS, MRS_received},
        {  MAQ, check_start_and_bits, MSG_MAQ, I2C_address_received},
        {  MSG_MAQ, check_ACK_and_MAQ_left, MSG_MAQ, send_msg_2IRIS},
        {  MSG_MAQ, check_C02_or_TVOC_sent, MSG_MAQ, calculate_sent_CRC},
        {  MSG_MAQ, check_XCK_and_notMAQ_and_stop, IDLE, MAQ_success},
        /*	MRS message send
        {  MRS, check_start_and_bits, MSG_MRS, I2C_address_received},
        {  MSG_MRS, check_ACK_and_MRS_left, MSG_MRS, send_msg_2IRIS},
        {  MSG_MRS, check_H2_or_ethanol_sent, MSG_MRS, calculate_sent_CRC},
        {  MSG_MRS, check_XCK_and_notMRS_and_stop, IDLE, MRS_success},
        */
        { -1, NULL, -1, NULL },
    };
}

int
calculate_CRC(int numberone, int numbertwo){
	return numberone-numbertwo;
}
