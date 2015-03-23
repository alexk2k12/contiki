#include "sys/node-id.h"
#include "dev/at25f512b.h"
#include "platform-conf.h"

uint32_t node_id_colibri = 0;

void node_id_restore(void) {
	node_id_colibri = 0;
	at25f512_rdOTP(XMEM_NODEID_BASEADDR, (uint8_t *)&node_id_colibri, 4);
}
