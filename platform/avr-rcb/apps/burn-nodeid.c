#include "contiki.h"
#include "serial-line.h"
#include "dev/eeprom.h"

#ifndef NODEID
#error Must compile with -DNODEID=[nodeid], where node id is 0-0xFFFF
#endif

#ifndef EEPROM_ADDR_LOC
#define EEPROM_ADDR_LOC 0x0000
#endif

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Burn Node ID process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();

  printf("burning node id: 0x%x\n", NODEID);
  
  unsigned short addr = NODEID;
  eeprom_write((eeprom_addr_t)EEPROM_ADDR_LOC, (unsigned char*)&addr, sizeof(addr));
   
  printf("done...\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
