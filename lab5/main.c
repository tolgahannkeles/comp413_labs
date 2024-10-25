#include "contiki.h"
#include "dev/light-sensor.h"
#include "dev/sht11-sensor.h"
#include "dev/button-sensor.h" // For button
#include "dev/leds.h"          // For LED
#include <stdio.h>             /* For printf() */

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
/*---------------------------------------------------------------------------*/
PROCESS(fire_alarm_process, "Firealarm process");
AUTOSTART_PROCESSES(&fire_alarm_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(fire_alarm_process, ev, data)
{
    static struct etimer timer;
    PROCESS_BEGIN();
    etimer_set(&timer, CLOCK_CONF_SECOND / 2);

    SENSORS_ACTIVATE(light_sensor); // need this for temperature sensor in Simulation
    SENSORS_ACTIVATE(sht11_sensor);
    SENSORS_ACTIVATE(button_sensor); // activate button too
    leds_off(LEDS_ALL);              // turn off all LEDs

    static float temp;
    static float threshold = 30.0;
    static float thresholdStep = 0.5;
    static int isAlarmActive = 0;

    static int counter = 0;
    while (1)
    {
        PROCESS_WAIT_EVENT(); // wait for an event

        if (ev == PROCESS_EVENT_TIMER)
        {
            // timer event
            printf("Time: %d\n", ++counter);

            temp = sht11_sensor.value(SHT11_SENSOR_TEMP_SKYSIM) * 0.04 - 39.6;
            printFloat("Temperature: ", temp);

            if (temp >= threshold)
            {
                printf("Alarm!\n");
                leds_on(LEDS_ALL);
                isAlarmActive = 1;
            }
            etimer_reset(&timer);
        }
        else if (ev == sensors_event && data == &button_sensor)
        {
            // button event

            if (isAlarmActive == 1)
            {
                printf("--------------------\n");
                printFloat("Old threshold: ", threshold);
                threshold = temp + thresholdStep;
                printFloat("New threshold: ", threshold);
                printf("--------------------\n");

                leds_off(LEDS_ALL);
                isAlarmActive = 0;
            }
            counter = 0;
            printf("%d\n", counter);
        }
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
