/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: cc2420-arch.c,v 1.10 2010/12/16 22:49:33 adamdunkels Exp $
 */

#include "contiki.h"
#include "contiki-net.h"

#include "dev/spi.h"
#include "mrf49xa.h"
#include "isr_compat.h"
#include <dev/leds.h>

#include "dev/leds.h"

#define DEBUG 0
#if DEBUG
//#include <stdio.h>
#define PRINTF(...) PRINTF(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

/*
 *
#ifdef CC2420_CONF_SFD_TIMESTAMPS
#define CONF_SFD_TIMESTAMPS CC2420_CONF_SFD_TIMESTAMPS
#endif // CC2420_CONF_SFD_TIMESTAMPS

#ifndef CONF_SFD_TIMESTAMPS
#define CONF_SFD_TIMESTAMPS 0
#endif // CONF_SFD_TIMESTAMPS

#ifdef CONF_SFD_TIMESTAMPS
#include "cc2420-arch-sfd.h"
#endif*/

static uint8_t isrCnt = 0;
static uint8_t dioCnt = 0;
#define MAX_DIO_CNT 5
/*---------------------------------------------------------------------------*/
ISR(MRF49XA_IRQ, mrf49xa_port2_interrupt)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  //PRINTF("interrupt!!\n");
  MRF49XA_DISABLE_FIFOP_INT();
  MRF49XA_DISABLE_DIO_INT();

  if(P2IFG & BV(MRF49XA_IRQ_PIN)){
	  isrCnt++;
	  if(mrf49xa_interrupt()) {
	    LPM4_EXIT;
	  }
	  MRF49XA_CLEAR_FIFOP_INT();

  }
  if(P2IFG & BV(MRF49XA_DIO_PIN)){
	  if(isrCnt){
		  isrCnt = 0;
		  dioCnt = 0;
	  }
	  else
		  dioCnt++; // numero di impulsi su DIO senza dati validi
	  if(dioCnt == MAX_DIO_CNT){
		  setReg(MRF49XA_FIFORSTREG,   0); //RegisterSet(FIFORSTREG);
		  setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch
		  uint16_t reg;
		  readSR(&reg);                     //serve a far tornare alto IRQ*/
		  isrCnt = 0;
		  dioCnt = 0;
	  }
	  MRF49XA_CLEAR_DIO_INT();
  }

  MRF49XA_ENABLE_DIO_INT();
  MRF49XA_ENABLE_FIFOP_INT();
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*
 * Initialize SPI bus.
 */
void
spi_init(void)
{
/*  // Initialize ports for communication with SPI units.
	PRINTF("SCK: %d\n",SCK);

  UCA0CTL1 |=  UCSWRST;                //reset usci
  UCA0CTL1 |=  UCSSEL_2;               //smclk while usci is reset
  UCA0CTL0 = ( UCMSB | UCMST | UCSYNC | UCCKPL); // MSB-first 8-bit, Master, Synchronous, 3 pin SPI master, no ste, watch-out for clock-phase UCCKPH

  UCA0BR1 = 0x00;
  UCA0BR0 = 0xaa;

  UCA0MCTL = 0;                       // Dont need modulation control.

  P3SEL |= BV(MISO) | BV(MOSI); // Select Peripheral functionality
  P3DIR |= BV(MISO);            // Configure as outputs(SIMO).

  P2SEL |= BV(SCK);
  P2DIR |= BV(SCK);

  //ME1   |= USPIE0;            // Module enable ME1 --> U0ME? xxx/bg

  // Clear pending interrupts before enable!!!
  UCA0IE &= ~UCRXIFG;
  UCB0IE &= ~UCTXIFG;
  UCB0CTL1 &= ~UCSWRST;         // Remove RESET before enabling interrupts

  while (!(UCA0IFG&UCTXIFG));               // USCI_A0 TX buffer ready?
  UCA0TXBUF = 0xaa;                     // Transmit first character

  //Enable UCB0 Interrupts
  //IE2 |= UCB0TXIE;              // Enable USCI_B0 TX Interrupts
  //IE2 |= UCB0RXIE;              // Enable USCI_B0 RX Interrupts*/

	  //WDTCTL = WDTPW|WDTHOLD;                   // Stop watchdog timer

	  //P3DIR |= BIT0;                            // Set P3.0 output (RESET_RF)
	  //P3OUT |= BIT0;                            // Set P3.0 high   (RESET_RF)

	  P3SEL |= BIT3|BIT4;                       // P3.3,4 option select
	  P2SEL |= BIT7;                            // P2.7 option select

	  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	  UCA0CTL0 |= UCMST|UCSYNC| UCMSB |UCCKPH;  // 3-pin, 8-bit SPI master
	                                            // Clock polarity low, MSB first, strobe rising edge
	  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	  UCA0BR0 = 0x04;                           // /2
	  UCA0BR1 = 0;                              //
	  UCA0MCTL = 0;                             // No modulation
	  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	  //UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt


	  //for(i=50;i>0;i--);                        // Wait for slave to initialize

	  while (!(UCA0IFG&UCTXIFG));               // USCI_A0 TX buffer ready?
	  UCA0TXBUF = 0xaa;                     // Transmit first character

	  //WDTCTL &= ~(WDTPW|WDTHOLD);           //restart watch dog timer

}

/*---------------------------------------------------------------------------*/
void
mrf49xa_arch_init(void)
{
  spi_init();

  // all input by default, set these as output
  MRF49XA_CSN_PORT(DIR) |= BV(MRF49XA_CSN_PIN);
  MRF49XA_IRQ_PORT(DIR) &= ~BV(MRF49XA_IRQ_PIN);
  MRF49XA_DIO_PORT(DIR) &= ~BV(MRF49XA_DIO_PIN);

  MRF49XA_FSELN_PORT(DIR) |= BV(MRF49XA_FSELN_PIN);
  MRF49XA_FSELN_PORT(OUT) |= BV(MRF49XA_FSELN_PIN);

  AT25F512B_PORT(DIR) |=  BV(AT25F512B_CS) ;
  AT25F512B_PORT(OUT) |=  BV(AT25F512B_CS) ;

  //interrupt is input pin
  //MRF49XA_INT_PORT(DIR) &= ~BV(MRF49XA_IRQ_PIN);

  MRF49XA_DISABLE_FIFOP_INT();
  MRF49XA_EDGE_FALL_INT();
  MRF49XA_CLEAR_FIFOP_INT();
  MRF49XA_ENABLE_FIFOP_INT();

  MRF49XA_DISABLE_DIO_INT();
  MRF49XA_EDGE_FALL_DIO_INT();
  MRF49XA_CLEAR_DIO_INT();
  MRF49XA_ENABLE_DIO_INT();


  /*CC2420_VREG_PORT(DIR) |= BV(CC2420_VREG_PIN);
  CC2420_RESET_PORT(DIR) |= BV(CC2420_RESET_PIN);

#if CONF_SFD_TIMESTAMPS
  cc2420_arch_sfd_init();
#endif*/

  MRF49XA_SPI_DISABLE();                // Unselect radio. */

  AT25F512B_PORT(OUT) &=  ~BV(AT25F512B_CS) ;
  SPI_WRITE(0xAB);
  AT25F512B_PORT(OUT) |=  BV(AT25F512B_CS) ;
}
/*---------------------------------------------------------------------------*/


