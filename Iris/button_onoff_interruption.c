/** File name   :button_onoff_interruption.c
  * Description :simulated interruption that works when space key is pressed
  */

#include "iris.h"

void *
button_onoff_interruption(){

  struct timeval next_activation;
  struct timeval now, timeout;

  gettimeofday (&next_activation, NULL);
  while (1) {
    struct timeval *period = task_get_period (pthread_self());
    timeval_add (&next_activation, &next_activation, period);
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;

    if(kbhit() && getchar() == 32){
      button_onoff_isr();
    }
  }
}
