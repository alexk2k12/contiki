/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup mrf24j40 MRF24J40 Driver
 *
 * @{
 */

/** 
 * \file   mrf24j40_arch.h
 * \brief  MRF24J40 Specific Arch Conf
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#ifndef __MRF24J40_ARCH_H__
#define __MRF24J40_ARCH_H__

#include "p32xxxx.h"

#include <stdint.h>

#include "dev/radio.h"

// Pins:
// - Reset: port 4, pin 7
// - CSn: port 4, pin 0
// - Wake: port 4, pin 4
// - Interrupt: port 4, pin 5.
// - SPI clock: port 4, pin 3, PM_UCB1CLK
// - SDI/SDI: PM_UCB1 SOMI/SIMO

/* Pin reset macros */
#define MRF24J40_RESETn_INIT()		    (P4DIR |= (1 << 6))
#define MRF24J40_CSn_INIT()		    (P4DIR |= (1 << 0))
#define MRF24J40_WAKE_INIT()		    (P4DIR |= (1 << 3))

/* Pin low/high */
#define MRF24J40_HARDRESET_LOW()            (P4OUT &= ~(1 << 6))
#define MRF24J40_HARDRESET_HIGH()           (P4OUT |= (1 << 6))
#define MRF24J40_CSn_LOW()                  (P4OUT &= ~(1 << 0))
#define MRF24J40_CSn_HIGH()                 (P4OUT |= (1 << 0))

/* IRC Configuration */
#define MRF24J40_INT_INIT()		    (P4IE |= (1 << 5))
#define MRF24J40_INTERRUPT_FLAT_CLR()	    (P4IFG &= ~(1 << 5))
#define MRF24J40_INTERRUPT_CHECK()	    (P4IFG & (1 <<5))

// Needed in spi.h - wait for transmission to stop. Use status word.
#define SPI_WAITFOREOFTx() do{} while((UCB0STAT_ & UCBUSY) == 1))
#define MRF24J40_SPI_PORT_INIT()	spi_init()
#define MRF24J40_SPI_PORT_WRITE(data)	SPI_WRITE(data)
#define MRF24J40_SPI_PORT_READ(data)	SPI_READ(data)

#define MRF24J40_PINDIRECTION_INIT()        \
do {                                        \
  MRF24J40_RESETn_INIT();		\
  MRF24J40_CSn_INIT();			\
  MRF24J40_WAKE_INIT();			\
} while(0)

#define MRF24J40_INTERRUPT_INIT(p, s)                                       \
do {                                                                        \
  P4DIR &= ~(1 << 5)			\
  MRF24J40_INT_INIT()			\
} while(0)

#endif /* __MRF24J40_ARCH_H__ */

/** @} */
