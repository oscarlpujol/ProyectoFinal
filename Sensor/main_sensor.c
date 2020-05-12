
#include "sensor.h"

static pthread_mutex_t mutex;

static TipoSensor sgp30;
static TipoFlags flags_sensor;
static TipoFlags flags_sensor_ack;

static char lastMsg;

fsm_t* fsm_new_sensor ();
fsm_t* fsm_new_sensor_ack ();

int observer();
int socket_init();
int sensor_init(TipoSensor* sensor, TipoFlags* flags);
int sensor_ack_init(TipoSensor* sensor, TipoFlags* flags, TipoFlags* flags2);

void *
total_sensor_control (void* ignore)
{
  fsm_t* sensor_fsm = fsm_new_sensor ();
  fsm_t* sensor_ack_fsm = fsm_new_sensor_ack ();

  srand(SEED);

  socket_init();
  sensor_init(&sgp30, &flags_sensor);
  sensor_ack_init(&sgp30, &flags_sensor_ack,&flags_sensor);

  char message[1024];
  read(sgp30.socket_desc, &message,1024); // no lee nada, se queda esperando
  printf("%s\n", message);
  fflush(stdout);

  struct timeval next_activation;
  struct timeval now, timeout;

  gettimeofday (&next_activation, NULL);
  while (1) {
    struct timeval *period = task_get_period (pthread_self());
    timeval_add (&next_activation, &next_activation, period);
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;

    fsm_fire (sensor_ack_fsm);
    fsm_fire (sensor_fsm); // Se puede hacer así? o con dos procesos diferentes?
  }
}

void *
socket_receive_observer(void* ignore) { //funcion que se encarga de activar los flags en funcion de lo que haya llegado

  struct timeval next_activation;
  struct timeval now, timeout;

  gettimeofday (&next_activation, NULL);
  while (1) {
    struct timeval *period = task_get_period (pthread_self());
    timeval_add (&next_activation, &next_activation, period);
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;
  }
}

int
main (void)
{
  pthread_mutex_init(&mutex, NULL);

  pthread_t tid1 = task_new ("sensor", total_sensor_control, CLK_MS, CLK_MS, 1, 2048);
  pthread_t tid2 = task_new ("observer", socket_receive_observer, CLK_MS, CLK_MS, 1, 2048);
  pthread_join (tid1, NULL);
  pthread_join (tid2, NULL);
  return 0;
}

int
socket_init (){
  int socket_desc , new_socket , c;
	struct sockaddr_in server , client;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  sgp30.socket_desc = new_socket;

	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}

  //Prepare server to be connected to
  server.sin_addr.s_addr = INADDR_ANY;//fill with my own ip address
  server.sin_family = AF_INET;
  server.sin_port = htons(SOCKETNUMBER);

	//Connect client to server
	if( connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("Connect error");

	}else{
    puts("Connected");
  }

  return 0;

}

int
observer() {
  read(&sgp30.socket_desc, &lastMsg, 20);

  if(lastMsg == sgp30.receiver) return;

  sgp30.receiver = lastMsg;

  if (lastMsg == "StartCond"){
    pthread_mutex_lock (&mutex);
  	flags_sensor.start_cond = 1;
  	flags_sensor_ack.start_cond = 1;
  	pthread_mutex_unlock (&mutex);
  }else if (lastMsg == "StopCond"){
    pthread_mutex_lock (&mutex);
  	flags_sensor.stop_cond = 1;
  	flags_sensor_ack.stop_cond = 1;
  	pthread_mutex_unlock (&mutex);
  }else if (lastMsg == "ACK"){
    pthread_mutex_lock (&mutex);
  	flags_sensor.ack = 1;
  	flags_sensor_ack.ack = 1;
  	pthread_mutex_unlock (&mutex);
  }else if (lastMsg == "XCK"){
    pthread_mutex_lock (&mutex);
  	flags_sensor.xck = 1;
  	flags_sensor_ack.xck = 1;
  	pthread_mutex_unlock (&mutex);
  }else{
    flags_sensor.bits_received = 1;
    flags_sensor_ack.bits_received = 1;
  }

}
