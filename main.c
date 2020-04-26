
#include "sensor.h"

static TipoSensor sgp30;

fsm_t* fsm_new_sensor ();
fsm_t* fsm_new_sensor_ack ();
void delay_until(unsigned int next, unsigned int now);

static void
total_sensor_control (void* ignore)
{
    fsm_t* sensor_fsm = fsm_new_sensor ();
    fsm_t* sensor_ack_fsm = fsm_new_sensor_ack ();

    sensor_init(&sgp30);

    long a;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    a = round(spec.tv_nsec / 1000000);
    unsigned int ms = a;

    while (1) {
        fsm_fire (sensor_fsm);
        fsm_fire (sensor_ack_fsm);

        clock_gettime(CLOCK_REALTIME, &spec);
        a = round(spec.tv_nsec / 1000000);
        unsigned int now = ms;

        ms += CLK_MS; // necesitamos constante CLK_MS
        delay_until (ms,now);
    }
}

void
user_init (void)
{
    xTaskHandle task;
    xTaskCreate (domotic_control, "sensor", 2048, NULL, 1, &task);
}

void
delay_until (unsigned int next, unsigned int now) {

	if (next > now) {
		usleep((next-now)*1000);
	}
}

void
socket_init (void){
  int socket_desc;
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);

	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}

	return 0;
}
void
socked_receive(void){
}
void
socket_send(void) {
}
