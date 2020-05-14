// Button MAQ_now Interruption

#include "iris.h"

void *
button_MAQnow_interruption(){

  struct timeval next_activation;
  struct timeval now, timeout;

  gettimeofday (&next_activation, NULL);
  while (1) {
    struct timeval *period = task_get_period (pthread_self());
    timeval_add (&next_activation, &next_activation, period);
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;

    if(kbhit() && getchar() == 'm'){
      button_MAQnow_isr();
    }
  }
}
