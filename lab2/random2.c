#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../core/sys/clock.h"

#define MAX 2.5
#define RANDOM_ELEMENT_COUNT 10

char *convertIntToChar(unsigned short number)
{
    unsigned short hundreds, tens, ones;
    hundreds = number / 100;
    tens = (number % 100) / 10;
    ones = number % 10;

    char *numChar = (char *)malloc(sizeof(char) * 5);
    sprintf(numChar, "%u.%u%u", hundreds, tens, ones);

    return numChar;
}

PROCESS(random_print_process, "Random print process");
AUTOSTART_PROCESSES(&random_print_process);
PROCESS_THREAD(random_print_process, ev, data)
{
    PROCESS_BEGIN();
    // Could not make it work with clock_time() as seed
    /*     clock_init();
        unsigned long seed=(unsigned long) clock_time();
        printf("%lu\n", seed); */
    random_init(7777);

    int i;
    int sum;
    for (i = 0; i < RANDOM_ELEMENT_COUNT; i++)
    {
        unsigned short elementShort = (random_rand() % (unsigned short)(MAX * 100));
        sum += elementShort;
        char *elementChar = convertIntToChar(elementShort);
        printf("%s\n", elementChar);
    }

    char *sumChar = convertIntToChar(sum);
    printf("Sum: %s\n", sumChar);
    PROCESS_END();
}