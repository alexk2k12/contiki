/*
 * Copyright (c) 2013, CBL Electronics srl + Siralab srl.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of Colibri platform for the Contiki operating system .
 *
 * $Id: at25f512b.c,v 1.0 2013/04/09 $
 */

/**
 * \file
 *         at25f512b library file.
 *         This file contains all necessary functions to control at25f512b
 *         flash memory.
 *         Memory address space is 24 bit but 32 bit unsigned int is used to
 *         store address information for C compatibility.
 *         Byte interface has been implemented.
 * \author
 *         Andrea Cannavicci <andrea.cannavicci@cblelectronics.com>
 *         Stefano Pagnottelli <stefano.pagnottelli@siralab.com>
 */
#include "dev/at25f512b.h"
#include "stdio.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

/*---------------------------------------------------------------------------*/
/**
 * \brief      	     read data from flash memory
 * \param address    Memory address. Only 24 bit are used
 * \param *buff      uint8_t buffer that will contain read data
 * \param len        number of bytes to read
 * \return           void function
 *
 *             The function check flash memory status register to
 *             verify if flash is busy. After wait flash is not busy
 *             will be read len+2 bytes. Len+1 bytes will be copied
 *             in to buffer
 */

void at52f512_read(uint32_t address, uint8_t *buff, uint8_t len){

	uint8_t busy;
	AT25F512_WT_BUSY(busy);

	AT25F512_SPI_START((uint8_t)at25f512b_ReadPage);
	AT25F512_SPI_BYTE_WRITE((uint8_t)(address>>16)&0xff);
	AT25F512_SPI_BYTE_WRITE((uint8_t)(address>>8 )&0xff);
	AT25F512_SPI_BYTE_WRITE((uint8_t)(address    )&0xff);

	uint8_t i;
	for(i=0;i<len;i++){
		AT25F512_SPI_BYTE_READ(&buff[i]);
	}
	AT25F512_SPI_STOP(0);
}

void at52f512_write(uint32_t address, uint8_t *buff, uint8_t len){
	uint8_t busy;
	AT25F512_WT_BUSY(busy);

	AT25F512_SPI_BYTE_CMD((uint8_t)at25f512b_Wren);

	AT25F512_SPI_START((uint8_t)at25f512b_WritePage);
	AT25F512_SPI_BYTE_WRITE((address>>16)&0xff);
	AT25F512_SPI_BYTE_WRITE((address>>8 )&0xff);
	AT25F512_SPI_BYTE_WRITE((address    )&0xff);

	uint8_t i;
	for(i=0;i<len;i++){
		if(i!=len-1)
			AT25F512_SPI_BYTE_WRITE(buff[i]);
		else
			AT25F512_SPI_STOP(buff[i]);
	}

}

void at25f512_erase(at25f512b_instr blockSize, uint32_t address){
	uint8_t busy;
	AT25F512_WT_BUSY(busy);

	AT25F512_SPI_BYTE_CMD((uint8_t)at25f512b_Wren);

	if (blockSize == (uint8_t)at25f512b_EraseBulk){
		AT25F512_SPI_BYTE_CMD((uint8_t)at25f512b_EraseBulk);
	}
	else{
		AT25F512_SPI_START(blockSize);
		AT25F512_SPI_BYTE_WRITE((address>>16)&0xff);
		AT25F512_SPI_BYTE_WRITE((address>>8 )&0xff);
		AT25F512_SPI_STOP((address    )&0xff);
	}
}


void at25f512_wrOTP(uint32_t address, uint8_t *buff, uint8_t len){
	uint8_t busy;
	AT25F512_WT_BUSY(busy);

	AT25F512_SPI_BYTE_CMD((uint8_t)at25f512b_Wren);

	AT25F512_SPI_START((uint8_t)at25f512b_WrOtp);
	AT25F512_SPI_BYTE_WRITE((address>>16)&0xff);
	AT25F512_SPI_BYTE_WRITE((address>>8 )&0xff);
	AT25F512_SPI_BYTE_WRITE((address    )&0xff);

	uint8_t i;
	for(i=0;i<len;i++){
		if (i!=(len-1))
			AT25F512_SPI_BYTE_WRITE(buff[i]);
		else
			AT25F512_SPI_STOP(buff[i]);
	}
}

void at25f512_rdOTP(uint32_t address, uint8_t *buff, uint8_t len){
	uint8_t busy;
	AT25F512_WT_BUSY(busy);

	AT25F512_SPI_START((uint8_t)at25f512b_ReadOtp);
	AT25F512_SPI_BYTE_WRITE((address>>16)&0xff);
	AT25F512_SPI_BYTE_WRITE((address>>8 )&0xff);
	AT25F512_SPI_BYTE_WRITE((address    )&0xff);

	//two dummy bytes to start shift out otp bytes
	AT25F512_SPI_BYTE_WRITE(0);
	AT25F512_SPI_BYTE_WRITE(0);

	uint8_t i;
	for(i=0;i<len;i++){
		AT25F512_SPI_BYTE_READ(&buff[i]);
	}
	AT25F512_SPI_STOP(0);
}

uint8_t at25f512_Rdsr(){
	uint8_t sr;
	AT25F512_SPI_START((uint8_t)at25f512b_Rdsr);
	AT25F512_SPI_BYTE_READ(&sr);
	AT25F512B_SPI_DISABLE();
	return sr;
}

uint16_t at25f512_RdID(){
	uint16_t id;
	uint8_t i;

	uint8_t busy;
	AT25F512_WT_BUSY(busy);

	AT25F512_SPI_START((uint8_t)at25f512b_RdID);
	for(i=0;i<2;i++)
		AT25F512_SPI_BYTE_READ((((uint8_t*)&id)+i));
	AT25F512B_SPI_DISABLE();
	//printf("id: %.4x\n",id);
	return id;
}

void at25f512_DP(at25f512b_dpMode mode){
	//FIXME: aggiungere un wait??
	if (mode == at25f512b_DPon)
		AT25F512_SPI_BYTE_CMD((uint8_t)at25f512b_DP); //entering dp:3us
	else
		AT25F512_SPI_BYTE_CMD((uint8_t)at25f512b_ResumeDP); //exiting dp: 8us
}


//NB len espresso in byte esatti (1 vale 1 byte!!)
uint16_t at25f512b_crc(uint32_t address, uint32_t len){

	uint8_t buff[CRC_BLOCK];
	//memset(buff,0xff,sizeof(buff)); //riempio a ff perchè il CRC usa 16bit!
	uint16_t i=0,j;
	//len +=1;

	uint16_t blocks = (uint16_t)(len)/CRC_BLOCK;
	PRINTF("blocks: %d\n",blocks);
	PRINTF("bytes:  %d\n",len%CRC_BLOCK);


	PRINTF("  i\tbyte\tCRC\n");
	CRCINIRES = 0xFFFF;
	if(blocks){
		for(i=0;i<blocks;i++){
			PRINTF("\n");
				at52f512_read(address+i*CRC_BLOCK,buff,CRC_BLOCK-1);
				for (j=0;j<CRC_BLOCK;j++){
					CRCDIRB = buff[j];
					//PRINTF("0x%.4X\t0x%.4X\t0x%.4X\n",i*CRC_BLOCK+j,buff[j],CRCINIRES);
					PRINTF("%.2X",buff[j]);
					if (j==15) PRINTF("\n");
				}
			}
	}

	PRINTF("\n");
	if(len%CRC_BLOCK ){
		at52f512_read(address+i*CRC_BLOCK,buff,(len)%CRC_BLOCK);
		for (j=0;j<len%CRC_BLOCK;j++){
			CRCDIRB = buff[j];
			//PRINTF("0x%.4X\t0x%.4X\t0x%.4X\n",i*CRC_BLOCK+j,buff[j],CRCINIRES);
			PRINTF("%.2X",buff[j]);
			if (j==15) PRINTF("\n");
		}
	}

	//printf("blocchi %d\n",(uint16_t)(len/CRC_BLOCK));
	//printf("bytes %d\n",(uint16_t)((len+1)%CRC_BLOCK));
	PRINTF("\nCRC=0x%.4X\n",CRCINIRES);
	return CRCINIRES;


}
