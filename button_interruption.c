// Button Interruption

#include "iris.h"

void
delay_until (unsigned int next, unsigned int now);

void
button_interruption(void){
  long a;
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	a = round(spec.tv_nsec / 1000000);
	unsigned int ms = a;

	while (1) {
    if(kbhit() && getchar() == 32){
  		button_isr();
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
