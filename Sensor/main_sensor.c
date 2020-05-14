
#include "sensor.h"

static TipoSensor sgp30;
static TipoFlags flags_sensor;
static TipoFlags flags_sensor_ack;

static char lastMsg[20];
static char startcond[20] = "StartCond";
static char stopcond[20] = "StopCond";
static char ack[20] = "ACK";
static char xck[20] = "XCK";

fsm_t* fsm_new_sensor ();
fsm_t* fsm_new_sensor_ack ();

int observer();
int socket_init();
int sensor_init(TipoSensor* sensor, TipoFlags* flags);
int sensor_ack_init(TipoSensor* sensor, TipoFlags* flags);
int ready (int fd);

void *
total_sensor_control (void* ignore)
{
  fsm_t* sensor_fsm = fsm_new_sensor ();
  fsm_t* sensor_ack_fsm = fsm_new_sensor_ack ();

  srand(SEED);

  socket_init();
  sensor_init(&sgp30, &flags_sensor);
  sensor_ack_init(&sgp30, &flags_sensor_ack);

  struct timeval next_activation;
  struct timeval now, timeout;

  gettimeofday (&next_activation, NULL);
  while (1) {
    struct timeval *period = task_get_period (pthread_self());
    timeval_add (&next_activation, &next_activation, period);
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;

    if(ready(sgp30.socket_desc)){
        observer();
    }
    fsm_fire (sensor_ack_fsm);
    fsm_fire (sensor_fsm);
  }
}

int
main (void)
{
  pthread_mutex_init(&mutex_socket, NULL);

  pthread_t tid1 = task_new ("sensor", total_sensor_control, CLK_MS, CLK_MS, 1, 2048);
  pthread_join (tid1, NULL);
  return 0;
}

int
socket_init (){
  int socket_desc , new_socket , c;
	struct sockaddr_in server , client;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  sgp30.socket_desc = socket_desc;

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
  pthread_mutex_lock(&mutex_socket);
  read(sgp30.socket_desc, &lastMsg, 20);
  pthread_mutex_unlock(&mutex_socket);

  if (!strcmp(lastMsg,startcond)){
    start_ack_isr();
    start_isr();
  }else if (!strcmp(lastMsg,stopcond)){
    stop_ack_isr();
    stop_isr();
  }else if (!strcmp(lastMsg,ack)){
    ack_ack_isr();
    ack_isr();
  }else if (!strcmp(lastMsg,xck)){
    xck_ack_isr();
    xck_isr();
  }else{
    new_msg(&lastMsg);
    bits_ack_isr();
    bits_isr();
  }

  return 0;

}

int ready (int fd){
  struct timeval timeout = {1,0};
  fd_set rdset;
  int res;

  if ( fd < 0 ) return 0;
  FD_ZERO(&rdset);
  FD_SET(fd, &rdset);
  res = select(fd+1, &rdset, NULL, NULL, &timeout);
  return res > 0;
}
