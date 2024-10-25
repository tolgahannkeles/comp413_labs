#include "contiki.h"
#include "dev/light-sensor.h"
#include "dev/sht11-sensor.h"
#include <stdio.h> /* For printf() */

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

float getLight(void)
{
    int lightData = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
    float light = lightData; // you need to implement the transfer function here
    float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 4096;
    float I = V_sensor / 100000;             // <- Ohm’s law
    float light_lx = 0.625 * 1e6 * I * 1000; // <- based on data sheet
    return light_lx;
}

/*---------------------------------------------------------------------------*/
PROCESS(sensor_reading_process, "Sensor reading process");
AUTOSTART_PROCESSES(&sensor_reading_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_reading_process, ev, data)
{
    static struct etimer timer;
    PROCESS_BEGIN();
    etimer_set(&timer, CLOCK_CONF_SECOND / 4); // you need to put the correct
                                               // timer setting here
    SENSORS_ACTIVATE(light_sensor);
    SENSORS_ACTIVATE(sht11_sensor);

    static int counter = 0;
    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev = PROCESS_EVENT_TIMER);

        float temp = getTemperature();
        float light_lx = getLight();

        printf("%d\n", ++counter); // you should also print the temperature
                                   // and light intensity here
        printFloat("Temperature in C: ", temp);
        printFloat("Light in Lux: ", light_lx);
        printf("-------------\n");

        etimer_reset(&timer);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

