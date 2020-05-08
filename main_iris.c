#include "sensor.h"

static TipoIris iris;
static TipoFlags flags;

fsm_t* fsm_new_iris ();
void delay_until(unsigned int next, unsigned int now);

static void
total_iris_control (void* ignore)
{
    fsm_t* iris_fsm = fsm_new_iris ();

    socket_init();
    sensor_ack_init(&iris, &flags);

    long a;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    a = round(spec.tv_nsec / 1000000);
    unsigned int ms = a;

    while (1) {
        fsm_fire (iris_fsm);

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
    xTaskCreate (iris_control, "iris", 2048, NULL, 1, &task);
}

void
delay_until (unsigned int next, unsigned int now) {

	if (next > now) {
		usleep((next-now)*1000);
	}
}

void
socket_init (void){
  int socket_desc , new_socket , c;
	struct sockaddr_in server , client;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("bind failed");
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 3);

	//Accept and incoming connection
  puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*) &c)) )
	{
		puts("Connection accepted");
    sgp30.socket_desc = new_socket;

	}

	if (new_socket < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

void
socket_receive(char* receiver, void* socket_desc){

  int sock = *(int*)socket_desc;
	int read_size;
	char *message , client_message[2000];

  while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		*receiver = client_message;
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
socket_send(char* sender, int* socket_desc) {

  int sock = *(int*)socket_desc;

  message = *sender;
	write(sock , message , strlen(message));

}
