#include "contiki.h"
#include "net/rime.h"
#include <stdio.h>
#include <string.h>
#include "dev/light-sensor.h"

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Client 2 - Light Sensor");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/

float getLight(void)
{
  int lightData = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
  float light = lightData;
  float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 4096;
  float I = V_sensor / 100000;
  float light_lx = 0.625 * 1e6 * I * 1000;
  return light_lx;
}

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

static int MAX_NUM = 0;
static float average = 0;
static const int TIMEOUT = 7/5;

static int is_max_num_received = 0;
static int is_ack_received = 0;
static int is_ready = 0;
static int is_outcome_received = 0;

static int try_count = 0;
static const int MAX_RETRY = 2;

struct ReportData
{
  char header[5]; // header signature: "DATA"
  float data;
};

struct InformMaxNum
{
  char header[9]; // header signature: "MAX_DATA"
  int data;
};

struct ReportOutcome
{
  char header[8]; // header signature: "OUTCOME"
  float data;
};

struct ReportData reportData;

static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  // Gelen mesajı yazdırma
  char *dataReceived = (char *)packetbuf_dataptr();

  if (strncmp(dataReceived, "MAX_DATA", 8) == 0)
  {
    struct InformMaxNum *maxData = (struct InformMaxNum *)dataReceived;
    MAX_NUM = maxData->data;
    is_max_num_received = 1;
    printf("Received MAX_NUM: %d\n", MAX_NUM);
  }
  else if (strncmp(dataReceived, "ACK", 3) == 0)
  {
    printf("Received ACK\n");
    is_ack_received = 1;
  }
  else if (strncmp(dataReceived, "READY", 5) == 0)
  {
    printf("Received READY\n");
    is_ready = 1;
  }
  else if (strncmp(dataReceived, "OUTCOME", 7) == 0)
  {
    struct ReportOutcome *reportOutcome = (struct ReportOutcome *)dataReceived;
    average = reportOutcome->data;
    is_outcome_received = 1;
    printf("Outcome: %d,%d\n", d1(average), d2(average));
  }
  else
  {
    printf("Unknown data format received. %s \n", dataReceived);
  }
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(light_sensor);

  unicast_open(&uc, 146, &unicast_callbacks);

  static rimeaddr_t addr;
  addr.u8[0] = 1;
  addr.u8[1] = 0;

  // Sending HELLO for the first time
  printf("Sending HELLO\n");
  char hello[] = "HELLO";
  packetbuf_copyfrom(hello, strlen(hello));
  unicast_send(&uc, &addr);

  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND * TIMEOUT);
  PROCESS_WAIT_EVENT_UNTIL(is_max_num_received || etimer_expired(&et));
  is_max_num_received = 0;

  static int i = 0;
  strcpy(reportData.header, "DATA");

  while (i < MAX_NUM)
  {

    reportData.data = getLight();

    packetbuf_copyfrom(&reportData, sizeof(reportData));
    unicast_send(&uc, &addr);

    printf("A packet is sent with data: %d.%d\n", d1(reportData.data), d2(reportData.data));

    etimer_set(&et, CLOCK_SECOND * TIMEOUT);
    PROCESS_WAIT_EVENT_UNTIL(is_ack_received || etimer_expired(&et));

    if (is_ack_received)
    {
      i = i + 1;
      is_ack_received = 0;
      continue;
    }
    else
    {
      if (try_count < MAX_RETRY)
      {
        try_count = try_count + 1;
        continue;
      }
      else
      {
        i = i + 1;
        try_count = 0;
        continue;
      }
    }
  }

  char done[] = "DONE";
  printf("All packets are sent\n");
  packetbuf_copyfrom(done, strlen(done));
  unicast_send(&uc, &addr);

  etimer_set(&et, CLOCK_SECOND * 2);
  PROCESS_WAIT_EVENT_UNTIL(is_ready || etimer_expired(&et));

  if (is_ready)
  {
    printf("Average is requested\n");
    char average[] = "AVERAGE";
    packetbuf_copyfrom(average, strlen(average));
    unicast_send(&uc, &addr);

    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(is_outcome_received || etimer_expired(&et));

    if (is_outcome_received)
    {
      printf("Closing connection\n");
      char end[] = "END";
      packetbuf_copyfrom(end, strlen(end));
      unicast_send(&uc, &addr);
    }
  }

  PROCESS_END();
}
