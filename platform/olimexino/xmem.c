#include "contiki.h"
#include "dev/xmem.h"
#include "at25f512b.h"

void xmem_init(void) {
}

int xmem_pread(void *_p, int size, unsigned long offset) {
	//uint8_t t,i;
	/*printf("add:0x");
	for(i=0;i<sizeof(unsigned long);i++){
		t = (offset >> (sizeof(unsigned long)-1-i)*8)&0xff;
		printf("%.2x",t);
	}
	printf("\n");*/
	//printf("size rd:0x%.2x\n",size);
	/*for(i=0;i<128;i+=16){
		at25f512_rdOTP(i, (uint8_t *)_p, 16);
		printf("add 0x%.2x:  ",i);
		for(t=0;t<16;t++)
			printf("%.2x ",((uint8_t *)(_p))[t]);
		printf("\n");
	}*/

	/*at25f512_rdOTP(0, _p, 20);
	print_hex_buff(_p,20);
	putchar('\n');*/

	if(offset >= 0xFFFF00) at25f512_rdOTP(offset & 0xFF, _p, size);
	else at52f512_read(offset,(uint8_t *)_p,size);
	return size;
}

int xmem_pwrite(const void *_buf, int size, unsigned long addr) {
	//printf("size wr:0x%.2x\n",size);
	if(addr >= 0xFFFF00) at25f512_wrOTP(addr & 0xFF,(uint8_t *)_buf,size);
	else at52f512_write(addr,(uint8_t *)_buf,size);
	//at25f512_RdID(); //wait until write end
	return size;
}

int xmem_erase(long size, unsigned long addr) {
	at25f512b_instr blockSize;
	switch (size)
	{
	case 0x00:
		blockSize=at25f512b_Erase4k;
		break;
	case 0x01:
		blockSize=at25f512b_Erase32k;
		break;
	case 0x02:
		blockSize=at25f512b_EraseBulk;
		break;
	default:
		return 0;
	}
	at25f512_erase(blockSize,addr);
	return size;
}
