/** File name   :main_iris.c
  * Description :intializes and starts the iris system and the interruption
  */

// Includes
#include "iris.h"

// Private variables
static TipoIris iris;
static TipoFlags flags;

static char lastMsg[20];
static char ack[20] = "ACK";
static char xck[20] = "XCK";

// Private function prototypes
fsm_t* fsm_new_iris ();
int observer();
int socket_init();
int iris_init(TipoIris* iris, TipoFlags* flags);
int ready (int fd);

/**
* @brief function that will run forever checking the iris fsm and observer
*/
void *
total_iris_control (void* ignore)
{
    fsm_t* iris_fsm = fsm_new_iris ();

    socket_init();
    iris_init(&iris, &flags);

    struct timeval next_activation;
    struct timeval now, timeout;

    gettimeofday (&next_activation, NULL);
    while (1) {
      struct timeval *period = task_get_period (pthread_self());
      timeval_add (&next_activation, &next_activation, period);
      gettimeofday (&now, NULL);
      timeval_sub (&timeout, &next_activation, &now);
      select (0, NULL, NULL, NULL, &timeout);

      if(ready(iris.socket_desc)){//wont block if nothing happens
          observer();
      }

      fsm_fire (iris_fsm);
    }
}

/**
* @brief starts every thread needed. One for each simulated interruption and other for the Iris system
*/
int
main ()
{
    pthread_t tid1 = task_new ("iris", total_iris_control, CLK_MS, CLK_MS, 2, 2048);
    pthread_t tid2 = task_new ("button_onoff", button_onoff_interruption, CLK_MS, CLK_MS, 1, 2048);
    pthread_t tid3 = task_new ("button_MAQnow", button_MAQnow_interruption, CLK_MS, CLK_MS, 1, 2048);
    pthread_t tid4 = task_new ("button_MRSnow", button_MRSnow_interruption, CLK_MS, CLK_MS, 1, 2048);
    pthread_join (tid1, NULL);
    pthread_join (tid2, NULL);
    pthread_join (tid3, NULL);
    pthread_join (tid4, NULL);
    return 0;
}
/**
* @brief initialization of the sockets needed (as server for sensor control, as client for GW)
* @return socket descriptors will be saved in the iris structure
*/
int
socket_init (){
  int socket_desc , new_socket, socket_desc_GW , new_socket_GW, c;
	struct sockaddr_in server , client, server_GW , client_GW;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  socket_desc_GW = socket(AF_INET , SOCK_STREAM , 0);

	if (socket_desc == -1 || socket_desc_GW == -1)
	{
		printf("Could not create socket");
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( SOCKETNUMBER );

  server_GW.sin_family = AF_INET;
	server_GW.sin_addr.s_addr = INADDR_ANY;
	server_GW.sin_port = htons( SOCKETNUMBERGW );

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
		puts("Connection to sensor accepted");
	}

	if (new_socket < 0)
	{
		perror("accept failed");
		return 1;
	}

  if( connect(socket_desc_GW, (struct sockaddr *)&server_GW, sizeof(server_GW)) < 0)
	{
		puts("Error conecting to GW");

	}else{
    puts("Connected to GW");
  }

	return 0;
}

/**
* @brief reads from the sensor socket and calls interrupton functions
*/
int
observer() {
  memset(lastMsg, 0, sizeof(lastMsg));
  read(iris.socket_desc, &lastMsg, 20);

  if (!strcmp(lastMsg,ack)){
    ack_isr();
  }else if (!strcmp(lastMsg,xck)){
    xck_isr();
  }else{
    new_msg(lastMsg);
    bits_isr();
  }

  return 0;

}

/**
* @brief reacts to a key being hit
* @return 1 if a key from keyboard has hitted, 0 if not
*/
int kbhit(){
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

/**
* @brief checks if something is happening in the socket descriptor so read or accept wont block
* @param file descriptor (socket descriptor is a file descriptor)
* @return >0 if file descriptor wont block
*/
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
