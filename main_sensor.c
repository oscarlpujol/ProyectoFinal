
#include "sensor.h"

static pthread_mutex_t mutex;

static TipoSensor sgp30;
static TipoFlags flags_sensor;
static TipoFlags flags_sensor_ack;

static char lastMsg;

fsm_t* fsm_new_sensor ();
fsm_t* fsm_new_sensor_ack ();
void delay_until(unsigned int next, unsigned int now);
void observer();
int socket_init();
int sensor_init(TipoSensor* sensor, TipoFlags* flags);
int sensor_ack_init(TipoSensor* sensor, TipoFlags* flags, TipoFlags* flags2);

static void
total_sensor_control (void* ignore)
{

  pthread_mutex_init(&mutex, NULL);

  fsm_t* sensor_fsm = fsm_new_sensor ();
  fsm_t* sensor_ack_fsm = fsm_new_sensor_ack ();

  srand(SEED);

  socket_init();
  sensor_init(&sgp30, &flags_sensor);
  sensor_ack_init(&sgp30, &flags_sensor_ack,&flags_sensor);

  long a;
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  a = round(spec.tv_nsec / 1000000);
  unsigned int ms = a;

  while (1) {
    fsm_fire (sensor_ack_fsm);
    fsm_fire (sensor_fsm); // Se puede hacer así? o con dos procesos diferentes?

    clock_gettime(CLOCK_REALTIME, &spec);
    a = round(spec.tv_nsec / 1000000);
    unsigned int now = ms;

    ms += CLK_MS; // necesitamos constante CLK_MS
    delay_until (ms,now);
    }
}

static void
socket_receive_observer(void) { //funcion que se encarga de activar los flags en funcion de lo que haya llegado
  long a;
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  a = round(spec.tv_nsec / 1000000);
  unsigned int ms = a;

  while (1) {
      observer();

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
    xTaskCreate (total_sensor_control, "sensor", 2048, NULL, 1, &task);
    xTaskCreate (socket_receive_observer, "observer", 2048, NULL, 1, &task);
}

void
delay_until (unsigned int next, unsigned int now) {

	if (next > now) {
		usleep((next-now)*1000);
	}
}

int
socket_init (){
  int socket_desc , new_socket , c;
	struct sockaddr_in server , client;

  int socket_desc2 , new_socket2 , c2;
	struct sockaddr_in server2 , client2;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  socket_desc2 = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1 || socket_desc2 ==-1)
	{
		printf("Could not create socket");
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( SOCKETNUMBERSEND );

  server2.sin_family = AF_INET;
	server2.sin_addr.s_addr = INADDR_ANY;
	server2.sin_port = htons( SOCKETNUMBERRECEIVE );

	//Bind
	if( (bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) || (bind(socket_desc2,(struct sockaddr *)&server2 , sizeof(server2)) < 0))
	{
		puts("bind failed");
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 3);
  listen(socket_desc2 , 3);

	//Accept and incoming connection
  puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
  c2 = sizeof(struct sockaddr_in);
	if( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*) &c)) || (new_socket2 = accept(socket_desc2, (struct sockaddr *)&client2, (socklen_t*) &c2)) )
	{
		puts("Connection accepted");
    sgp30.socket_desc_send = new_socket;
    sgp30.socket_desc_receive = new_socket2;

	}

	if ((new_socket < 0) || (new_socket2 < 0))
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

void
socket_receive(char* receiver){

  int sock = *(int*)sgp30.socket_desc_receive;
	int read_size;
	char *message , client_message[2000];

  while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		receiver = client_message;
	}

	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}

}

void
socket_send(char* sender) {

  int sock = *(int*)sgp30.socket_desc_send;

  char message;
  message = *sender;
	write(sock , message , strlen(message));

}

void
observer(void) {
  socket_receive(&lastMsg);

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
