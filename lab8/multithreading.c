#include "contiki.h"
#include <stdio.h> /* For printf() */

#include "dev/sht11-sensor.h"
#include "dev/light-sensor.h"

#define MAX_VALUES  4 // Number of values used in averaging process

static process_event_t event_data_ready; // Application specific event value


int d1(float f) // Integer part
{
    return ((int)f);
}
unsigned int d2(float f) // Fractional part
{
    if (f > 0)
        return (1000 * (f - d1(f)));
    else
        return (1000 * (d1(f) - f));
}

void printFloat(const char *prefix, float number)
{
    printf("%s%d.%03u\n", prefix, d1(number), d2(number));
}

float getTemperature(void)
{
    int tempData;

    // NOTE: You only need to use one of the following
    // If you run the code in Cooja Simulator, please remove the second one
    tempData = sht11_sensor.value(SHT11_SENSOR_TEMP_SKYSIM); // For Cooja Sim
    // tempData = sht11_sensor.value(SHT11_SENSOR_TEMP); // For XM1000 mote

    float temp = 0.04 * tempData - 39.6; // you need to implement the transfer function here
    return temp;
}

/*---------------------------------------------------------------------------*/
/* We declare the two processes */
PROCESS(temp_process, "Temperature process");
PROCESS(print_process, "Print process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&temp_process, &print_process);
/*---------------------------------------------------------------------------*/
/* Implementation of the first process */
PROCESS_THREAD(temp_process, ev, data)
{
    /* Variables are declared static to ensure their values are kept */
    /* between kernel calls. */
    static struct etimer timer;
    static int count = 0;

    // Any process must start with this.
    PROCESS_BEGIN();

    // Allocate the required event
    event_data_ready = process_alloc_event();

    // Initialise the temperature sensor
    SENSORS_ACTIVATE(light_sensor);  // need this for sky-mote emulation
    SENSORS_ACTIVATE(sht11_sensor);

    // Initialise variables
    count = 0;

    // Set the etimer module to generate a periodic event
    etimer_set(&timer, CLOCK_CONF_SECOND);

    static float myData;
    static float sum;
    while (1)
    {
        // Wait here for the timer to expire
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

        // this is temperature process
       	count++;
        //int tempVal = sht11_sensor.value(SHT11_SENSOR_TEMP_SKYSIM); // For Cooja Sim
        float tempVal = getTemperature();
	printFloat("[temperature process] temp=",tempVal);
	sum+=tempVal;

        // Check if enough samples are collected
       	if (count==MAX_VALUES)
       	{
       	  // Transfer the last read raw temperature reading for passing
       	  myData = sum/MAX_VALUES;

       	  // Reset variables
       	  count = 0;

       	  // Post an event to the print process
       	  // and pass a pointer to the last measure as data
	  printFloat("[temperature process] passing value ", myData);
          process_post(&print_process, event_data_ready, &myData);
	  sum=0;
        }

        // Reset the timer so it will generate another event
        etimer_reset(&timer);
     }

     // Any process must end with this, even if it is never reached.
     PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Implementation of the second process */
PROCESS_THREAD(print_process, ev, data)
{
    PROCESS_BEGIN();

    while (1)
    {
      // Wait until we get a data_ready event
      PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);

      // Use 'data' variable to retrieve data and then display it
      float myData = *(float *)data; // cast to an integer pointer
      printFloat("[print process] data ready, received value = ", myData);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/