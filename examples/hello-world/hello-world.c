#include "contiki.h"
#include "rime.h"
#include "serial-line.h"
#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  printf("unicast message received from %d.%d\n", from->u8[0], from->u8[1]);
}
static void sent_uc(struct unicast_conn *c, int status, int num) {
  printf("unicast send status: %d\n", status);
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  unicast_open(&uc, 146, &unicast_callbacks);
 
  PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && !strcmp(data,"tx"));

  while(1) {
    static struct etimer et;
    linkaddr_t addr;
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    static char buffer[120];
    packetbuf_copyfrom(buffer, 90);
    addr.u8[0] = 5; //(linkaddr_node_addr.u8[0]==1?2:1);
    addr.u8[1] = 0;
    if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
      unicast_send(&uc, &addr);
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
