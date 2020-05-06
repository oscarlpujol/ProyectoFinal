/*

*/

#include "sensor.h"

static pthread_mutex_t mutex;

enum states {
	IDLE, // initial state
	WAIT_16BITS,
	MAQ,
	MSG_MAQ,
	MRS,
	MSG_MRS
}

// Int

static int
check_start_and_bits (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (flags.start_cond && flags.bits_received);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_IAQ_and_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (flags.start_cond && flags.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_I2C_address(fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = flags.I2C_address_wrong;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_MAQ (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = flags.MAQ;
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_ACK_and_MAQ_left (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (flags.ack && flags.msg_MAQ_left);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_C02_or_TVOC_sent (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (flags.CO2_sent || flags.TVOC_sent);
	pthread_mutex_unlock (&mutex);
	return result;
}

static int
check_XCK_and_notMAQ_and_stop (fsm_t* this)
{
	pthread_mutex_lock (&mutex);
	int result = 0:
	result = (flags.xck && !flags.msg_MAQ_left && flags.stop_cond);
	pthread_mutex_unlock (&mutex);
	return result;
}

// Void

static void
I2C_address_success (fsm_t* this)
{
	TipoProyecto *p_sgp30;
	p_sgp30 = (TipoProyecto*)(this->user_data);

	pritnf("Direccion I2C recibida con exito!\n");
	printf("I2C address = %d \n", p_sgp30->receiver);

	if(p_sgp30->receiver == p_sgp30->I2C_ADDRESS_SENSOR)
	{
		pthread_mutex_lock (&mutex);
		flags.I2C_address_wrong = 1;
		pthread_mutex_unlock (&mutex);
	}
}

static void
IAQ_received (fsm_t* this)
{
    /*
	Inicializar el sensor y mensaje de EXIT
	Inicializar reloj para saber si los valores son reales o no
    */
}

static void
wrong_I2C_address (fsm_t* this)
{
	/*
	Enviar mensaje de que la dirección era errónea.
    */
}

static void
MAQ_received (fsm_t* this)
{
    /*
	Enviar mensaje de que recibes la orden MAQ
	Calcular valores TVOC y CO2 (teniendo en cuenta si son reales o no)
    */
}


static void
I2C_address_received (fsm_t* this)
{
    /*
	Guardar dirección I2C de la IRIS.
    */
}

static void
send_msg_2IRIS (fsm_t* this)
{

	TipoProyecto *p_sgp30;
	p_sgp30 = (TipoProyecto*)(this->user_data);

	socket_send(&(p_sgp30->address), &(sgp30->socket_desc));
	p_sgp30->address += 1;
    /*
	Enviar mensaje a la IRIS con las mediciones y mover puntero
    */
}

static void
calculate_sent_CRC (fsm_t* this)
{

	TipoProyecto *p_sgp30;
	p_sgp30 = (TipoProyecto*)(this->user_data);

	int CRC;
	CRC = calculate_CRC((int)* p_sgp30->measures[(p_sgp30->address) -2], (int)* p_sgp30->measures[(p_sgp30->address)-1]);
	p_sgp30->measures[(p_sgp30->address)] = CRC;

	socket_send(&(p_sgp30->address), &(sgp30->socket_desc));
	p_sgp30->address += 1;
}

static void
MAQ_success (fsm_t* this)
{
    for (int i = 0; i < 5; ++i)
    {
    address -= 1;
    *address = 0;
    }
    /*
	Mensaje completado
	Borrar datos recogidos y reiniciar puntero
    */
}

/*
Interruption functions
*/

static void
initial_timer (union sigval value){

	pthread_mutex_lock (&mutex);
	p_sgp30.realmeasures  = 1;
	pthread_mutex_unlock (&mutex);
	// tmr_destroy()
	printf("TIMEOUT\n");
}

int
sensor_init(TipoProyecto *p_sgp30)
{
	p_sgp30->realmeasures = 0;
	p_sgp30->tmr_real_measures = tmr_new (initial_timer); // timer starter when IAQ, turns realmeasures into 1
	p_sgp30->I2C_ADDRESS_IRIS = 0; // IRIS I2C address
	p_sgp30->I2C_ADDRESS_SENSOR = OWN_ADDRESS; // SENSOR I2C address
	p_sgp30->measures [6] = {0,0,0,0,0,0};
	pthread_mutex_init(&mutex, NULL);

	printf("\nSystem init complete!\n");
	fflush(stdout);

	return 0;
}


fsm_t*
fsm_new_sensor (/*int* validp, int pir, int alarm*/)
{
    static fsm_trans_t alarm_tt[] = {
        {  IDLE, check_start_and_bits, WAIT_16BITS, I2C_address_success},
        {  WAIT_16BITS, check_IAQ_and_stop, IDLE, IAQ_received},
        {  WAIT_16BITS, check_I2C_address, IDLE, wrong_I2C_address}
        {  WAIT_16BITS, check_MAQ, MAQ, MAQ_received},
        {  WAIT_16BITS, check_MRS, MRS, MRS_received},
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
