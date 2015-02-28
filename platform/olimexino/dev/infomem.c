/*
Copyright 2007, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universit�t Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2007
*/

/**
 * @file		infomem.c
 * @addtogroup	storage
 * @brief		MSP430 Infomemory Storage
 * @author		Michael Baar	<baar@inf.fu-berlin.de>
 *
 * Functions to store and read data from the two infomemories (2 x 128 Bytes).
 * Offset addresses start at zero, size has a maximum of 128, write operations
 * across both blocks are not allowed.
 */
#include <string.h>
#include <stdarg.h>
#include "stdio.h"
#include "contiki-conf.h"
#include "infomem.h"

void
infomem_read(void *buffer, unsigned int offset, unsigned char size) {
	//printf("a\n");
  uint8_t *address = (uint8_t *)INFOMEM_START + offset;
  memcpy(buffer, address, size);
}

int
infomem_write(void *data, unsigned int offset, unsigned char size )
{
  char backup[INFOMEM_BLOCK_SIZE];	
  uint16_t i;
  uint32_t *buffer;
  uint32_t *flash;
  uint8_t *bufdata;


  int s;

  if(offset%INFOMEM_BLOCK_SIZE + size >= 128) return 0;
  flash = (uint32_t *)(INFOMEM_START + offset/INFOMEM_BLOCK_SIZE);

  s = splhigh();

  /* backup into RAM */
  memcpy(backup, flash, INFOMEM_BLOCK_SIZE);

  bufdata = ((uint8_t *)backup) + offset;
  memcpy(bufdata, data, size);


  /* init flash access */
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY|ERASE;                      // Set Erase bit
  *flash = 0;								// Dummy write to erase Flash seg

  /* write flash */
  FCTL1 = FWKEY|BLKWRT;                     // Enable long-word write
  buffer = (uint32_t *)backup;
  for(i = 0; i < INFOMEM_BLOCK_SIZE; i+=4) {
    *flash++ = *buffer++;
  }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY|LOCK;                       // Set LOCK bit

  splx(s);

  return 1;
}
