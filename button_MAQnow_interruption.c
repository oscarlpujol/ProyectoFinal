// Button MAQ_now Interruption

#include "iris.h"

void
delay_until (unsigned int next, unsigned int now);
int
kbhit();

void
button_onoff_interruption(void){
  long a;
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	a = round(spec.tv_nsec / 1000000);
	unsigned int ms = a;

	while (1) {
    if(kbhit() && getchar() == 40){ //hay que ver que tecla queremos que sea
  		button_MAQnow_isr();
  	}

		clock_gettime(CLOCK_REALTIME, &spec);
		a = round(spec.tv_nsec / 1000000);
		unsigned int now = ms;

		ms += CLK_MS;
		delay_until (ms,now);
	}
}

void
delay_until (unsigned int next, unsigned int now) {

	if (next > now) {
		usleep((next-now)*1000);
	}
}

int kbhit(void){
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
