#include "contiki.h" 
#include "random.h" 
#include "powertrace.h" 
#include "dev/leds.h"
#include <stdio.h> 

PROCESS(power, "powertrace example"); 
AUTOSTART_PROCESSES(&power); 

int d1(float f) // Integer part
{
  return((int)f);
}
unsigned int d2(float f) // Fractional part
{
  if (f > 0)
    return(1000 * (f - d1(f)));
  else
    return(1000 * (d1(f) - f));
}

PROCESS_THREAD(power, ev, data) 
{ 
  static struct etimer et; 
  static float t;

  PROCESS_BEGIN(); 

  /* Start powertracing */ 
  int n = 1; // 1 second reporting cycle 
  powertrace_start(CLOCK_SECOND * n); 
  printf("Ticks per second: %u\n", RTIMER_SECOND); 

  while (1) 
  {
    /* Delay 2-4 seconds to perform tasks more frequently */ 
    t = 2 * ((float)random_rand() / RANDOM_RAND_MAX) + 2; 
    etimer_set(&et, CLOCK_SECOND * t); 
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); 

    // Task 1: Switching LED
    static int ledStatus = 0;
    switch (ledStatus) {
      case 0: 
        leds_on(LEDS_RED);
        break;
      case 1: 
        leds_on(LEDS_GREEN);
        break;
      case 2: 
        leds_on(LEDS_BLUE);
        break;
      case 3: 
        leds_on(LEDS_ALL); // Maintain all LEDs on longer
        break;
      default:
        leds_off(LEDS_ALL); 
        ledStatus = -1;
    }
    ledStatus++;

    int i;
    // Task 2
    float sq = t * t;
    for (i = 0; i < 50; i++) {
      sq += t * t;
    }
    printf("floor=%d\n", (int)sq);

    // Task 3 
    float calc = (t + 3.14) * (t - 1.23) / (t * 2.71);
    printf("Initial calculation result: %d.%03u\n", d1(calc), d2(calc));

    for (i = 0; i < 500; i++) {
      calc += (i * 0.01);
      calc = (calc + i) / (i + 1);
    }

    printf("Final calculation result: %d.%03u\n", d1(calc), d2(calc));

    // Task 4
    for (i = 0; i < 1000; i++) {
      float temp = (calc + i) / (i + 1);
    }

 
  } 

  PROCESS_END(); 
}