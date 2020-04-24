/*

*/

#include "sensor.h"

static TipoSensor sgp30;
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
    /*
	
    */
}

static int
check_IAQ_and_stop (fsm_t* this)
{
    
}

static int
check_I2C_address(fsm_t* this)
{

}

static int
check_MAQ (fsm_t* this)
{
    
}

static int
check_ACK_and_MAQ_left (fsm_t* this)
{
    
}

static int
check_C02_or_TVOC_sent (fsm_t* this)
{
    
}

static int
check_XCK_and_notMAQ_and_stop (fsm_t* this)
{
    
}

// Void

static void
I2C_address_success (fsm_t* this)
{
    /*
	Enviar mensaje de que hemos recibido 8 bits (la dirección I2C suponemos)		
    */
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
    /*
	Enviar mensaje a la IRIS con las mediciones y mover puntero
    */
}

static void
calculate_sent_CRC (fsm_t* this)
{
    /*
	Calcular CRC y enviarlo
    */
}

static void
MAQ_success (fsm_t* this)
{
    /*
	Mensaje completado
	Borrar datos recogidos y reiniciar puntero
    */
}

/*
Interruption functions
*/

static void initial_timer (union sigval value){

	pthread_mutex_lock (&mutex);
	// flags |= (FLAG_TIME_OUT_MEDIDA);
	pthread_mutex_unlock (&mutex);
	// num_bits_recibidos = 0;
	sgp30.realmeasures = 1;
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


int
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
    sensor_init(&sgp30);
}
