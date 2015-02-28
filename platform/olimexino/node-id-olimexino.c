#include "sys/node-id.h"
#include "dev/at25f512b.h"
#include "platform-conf.h"

uint32_t node_id_olimexino = 0;

void node_id_restore(void) {
	node_id_olimexino = 0;
	infomem_read((uint8_t *)&node_id_olimexino,INFOMEM_NODE_ID, INFOMEM_NODE_ID_SIZE); 
}
