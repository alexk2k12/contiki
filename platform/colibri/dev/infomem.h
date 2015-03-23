/*
Copyright 2006, Freie Universitaet Berlin. All rights reserved.

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
Berlin, 2006
*/

/**
 * @file		infomem.h
 * @addtogroup		storage
 * @brief		MSP430 Infomemory Storage
 * 
 * @author		Michael Baar	<baar@inf.fu-berlin.de>
 */

#ifndef INFOMEM_H
#define INFOMEM_H

#if !defined(INFOMEM_START) || !defined(INFOMEM_BLOCK_SIZE)
  #error "infomem position (INFOMEM_START) and block size (INFOMEM_BLOCK_SIZE) need to be defined for the platform"
#endif

#define OPTIONS_BACKBONE_NODE	0x8000
#define OPTIONS_NET_REPEATER    0x4000
// Start at 0x0000
typedef struct infomem_addresses_t {
	uint32_t nodeId;					// @0x00
	uint8_t macAddr[6];					// @0x04
	uint16_t options;					// @0x0A
	uint8_t rimeAddr[4];				// @0x0C
}infomem_addresses; 					// Size: 0x10

/**
 * baudarte:
 *   0 = 2400
 *   1 = 4800
 *   2 = 9600
 *   3 = 19200
 *   4 = 38400
 *(D)5 = 57600
 *   6 = 115200
 *   7 = 172400
 * txPower:
 *(D)0 = 0DB
 *   1 = 2DB
 *   2 = 5DB
 *   3 = 7DB
 *   4 = 10DB
 *   5 = 12DB
 *   6 = 15DB
 *   7 = 17DB
 * minRssi:
 *   Unused
 * channel:
 *   [0-23] (D)==12
 * band:
 *   1 = 433
 *   2 = 868
 *   3 = 915
 **/

// Start at 0x0010
typedef struct infomem_radio_t {
	uint8_t baudRate;					// @0x10
	uint8_t txPower;					// @0x11
	uint8_t minRssi;					// @0x12
	uint8_t channel;					// @0x13
	uint8_t band;						// @0x14
	uint8_t padding;					// @0x15
}infomem_radio;							// Size: 0x06

typedef enum pinMode_t{
	pwmOut     = 0,
	analogIn   = 1,
	digitalOut = 2,
	digitalIn  = 3
}pinMode;

//this structure can define up to 8 gpio colibri pins
typedef struct infomem_ioMode_t{
	uint16_t pinsMode;
}infomem_ioMode;

typedef struct infomem_pwm_t{
	uint16_t pwmPeriod;
	uint16_t pwmDuty[2];
}infomem_pwm;

typedef struct infomem_a_t {
	infomem_addresses addresses;
	infomem_radio radio;
	infomem_ioMode GpioMode;
	infomem_pwm pwmSets;

}infomem_a;

#define INFOMEM_STRUCT_A ((infomem_a *)INFOMEM_START)
#define IM_OPTIONS INFOMEM_STRUCT_A->addresses.options

/**
 * @brief	Read bytes from infomemory
 * @param[out]		buffer		Pointer to buffer for read data
 * @param[in]		offset		Offset in infomemory (0-254)
 * @param[in]		size		Number of bytes to read
 */
void infomem_read(void *buffer, unsigned int offset, unsigned char size);

/**
 * @brief	Write bytes to infomemory
 * @param[in]		offset		Offset in infomemory (0-254)
 * @param[in]		count		Number of items following
 *					each item is a pair pointer, length
 *
 * Example: Infomem_write( 0, 2, &a,3, &b,1 );
 *
 * \note: The MSP430 has two consecutive blocks of infomemory.
 *        Each is 128 bytes large. The offset is the relative address 
 *        starting at the beginning of the first block. You can write an 
 *        arbitrary number of bytes at any offset, but this function 
 *        cannot write across the two blocks of infomemory.
 */
int infomem_write(void *data, unsigned int offset, unsigned char size );

#endif // !INFOMEM_H
