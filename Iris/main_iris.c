#include "iris.h"

static TipoIris iris;
static TipoFlags flags;

fsm_t* fsm_new_iris ();

static char lastMsg[20];
static char ack[20] = "ACK";
static char xck[20] = "XCK";

int observer();
int socket_init();
int iris_init(TipoIris* iris, TipoFlags* flags);

void *
total_iris_control (void* ignore)
{
    fsm_t* iris_fsm = fsm_new_iris ();

    socket_init();
    iris_init(&iris, &flags);

    /*char* message = "hola\n";
    int result;
    result = write(iris.socket_desc, message,strlen(message)); //no consigo comunicacion
    printf("%d\n", result);
    fflush(stdout);
    printf("He pasado\n");
    fflush(stdout);*/

    struct timeval next_activation;
    struct timeval now, timeout;

    gettimeofday (&next_activation, NULL);
    while (1) {
      struct timeval *period = task_get_period (pthread_self());
      timeval_add (&next_activation, &next_activation, period);
      gettimeofday (&now, NULL);
      timeval_sub (&timeout, &next_activation, &now);
      select (0, NULL, NULL, NULL, &timeout);

      fsm_fire (iris_fsm);
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

    observer();
  }
}

int
main ()
{
    pthread_t tid1 = task_new ("iris", total_iris_control, CLK_MS, CLK_MS, 2, 2048);
    pthread_t tid2 = task_new ("button_onoff", button_onoff_interruption, CLK_MS, CLK_MS, 1, 2048);
    //pthread_t tid3 = task_new ("button_MAQnow", button_MAQnow_interruption, CLK_MS, CLK_MS, 1, 2048);
    pthread_t tid4 = task_new ("observer", socket_receive_observer, CLK_MS, CLK_MS, 2, 2048);
    pthread_join (tid1, NULL);
    pthread_join (tid2, NULL);
    //pthread_join (tid3, NULL);
    pthread_join (tid4, NULL);
    return 0;
}

int
socket_init (){
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
	server.sin_port = htons( SOCKETNUMBER );

	//Bind
	if( (bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0))
	{
		puts("bind failed");
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 1);

	//Accept and incoming connection
  puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	if( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*) &c)))
	{
    iris.socket_desc= new_socket;
		puts("Connection accepted");
	}

	if (new_socket < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

int
observer() {
  read(iris.socket_desc, &lastMsg, 20);

  if (!strcmp(lastMsg,ack)){
    ack_isr();
  }else if (!strcmp(lastMsg,xck)){
    xck_isr();
  }else{
    new_msg(&lastMsg);
    bits_isr();
  }

  return 0;

}
