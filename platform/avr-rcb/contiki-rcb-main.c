/*
 * Copyright (c) 2006, Technical University of Munich
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
 * This file is part of the Contiki operating system.
 *
 * @(#)$$
 */

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <eeprom.h>
#include <stdio.h>

#include "linkaddr.h"
#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"

#if RF230BB           //radio driver using contiki core mac
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/ipv6/sicslowpan.h"
#else                 //radio driver using Atmel/Cisco 802.15.4'ish MAC
#include <stdbool.h>
#include "mac.h"
#include "sicslowmac.h"
#include "sicslowpan.h"
#include "ieee-15-4-manager.h"
#endif /*RF230BB*/

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

//#include "dev/rs232.h"
#include "dev/serial-line.h"
#include "dev/slip.h"

#include "sicslowmac.h"
#include "contiki-rcb.h"

//FUSES =
//	{
//		.low = 0xe2,
//		.high = 0x99,
//		.extended = 0xff,
//	};
	
PROCESS(rcb_leds, "RCB leds process");

#if RF230BB
#if NETSTACK_CONF_WITH_IPV6 || NETSTACK_CONF_WITH_IPV4
PROCINIT(&etimer_process, &tcpip_process, &rcb_leds);
#else
PROCINIT(&etimer_process, &rcb_leds);
#endif
#else
#if NETSTACK_CONF_WITH_IPV6 || NETSTACK_CONF_WITH_IPV4
PROCINIT(&etimer_process, &mac_process, &tcpip_process, &rcb_leds);
#else
PROCINIT(&etimer_process, &mac_process, &rcb_leds);
#endif
#endif

/* Put default MAC address in EEPROM */
uint8_t mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
FILE ftdi_io = FDEV_SETUP_STREAM(putchar_ftdi, NULL, _FDEV_SETUP_WRITE); 

#define LED1 (1<<PE2)
#define LED2 (1<<PE3)
#define LED3 (1<<PE4)

#define LEDOff(x) (PORTE |= (x))
#define LEDOn(x) (PORTE &= ~(x))

void
init_lowlevel(void)
{
//  /* Second rs232 port for debugging */
//  rs232_init(RS232_PORT_1, USART_BAUD_57600,
//             USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);
//
//  /* Redirect stdout to second port */
//  rs232_redirect_stdout(RS232_PORT_1);

  DDRE |= LED1 | LED2 | LED3;
  LEDOff(LED1|LED2|LED2);
  LEDOn(LED1);

  //set up ftdi serial.
  XMCRA |= 0x80;
  stdout = &ftdi_io;

  //set up an interrupt
  EICRB &= ~(0x80); //ISC71 = 0
  EICRB |= 0x40;    //ISC70 = 1
  EIMSK |= 0x80;    //Enable this interrupt mask
  DDRE &= ~0x80;     //Set this port as an input
  sei();

  //set up transmit enable port
  DDRE &= ~0x40;
}


static struct etimer et;
PROCESS_THREAD(rcb_leds, ev, data)
{

  PROCESS_BEGIN();
  
    while(1) {
      PROCESS_YIELD();
      
#if NETSTACK_CONF_WITH_IPV6	  
	  if (ev == ICMP6_ECHO_REQUEST) {
#else
 		if (1) {
#endif        
//		etimer_set(&et, CLOCK_SECOND/10);
	  } else {
//		LEDOff(LED2);
	  }
    }
  PROCESS_END();
}

/* 
 * Try to read an address from EEPROM, otherwise use default short address: 0x1234
 * Set the PAN ID in the radio to 0xABCD, and the short address.
 * Set the rime address (if rime is being used) to the short address
*/
#ifndef EEPROM_ADDR_LOC
#define EEPROM_ADDR_LOC 0x0000
#endif
int init_addresses() {
  short int addr;
  printf("reading address from location in eeprom: 0x%x\n", EEPROM_ADDR_LOC);
  eeprom_read((eeprom_addr_t)0, (unsigned char*)&addr, sizeof(addr));
  
  if(addr == 0){
    printf("No address in EEPROM, using 0x1234 and PAN ID 0xabcd\n");
    addr = 0x1234;
  }
  else {
    printf("Address found in EEPROM: 0x%x\n", addr);
  }
  
  //set it in the radio..
  rf230_set_pan_addr(0xABCD, (unsigned short) addr, NULL);
 
  //set it in software..
  linkaddr_set_node_addr((linkaddr_t*)&addr);

  return 0;
}

int
main(void)
{
  /* Initialize hardware */
  init_lowlevel();  

  /* Process subsystem */
  printf("\n********BOOTING CONTIKI*********\n");
  printf("System online.\n");
  LEDOn(LED2);

  process_init();
  /* Clock */

  //initializing the rtimer module.
  rtimer_init();

  /* Be sure the process subsystem is initialized apriori */
  serial_line_init();
  clock_init();
  ctimer_init();
 
  /* Register initial processes */
  procinit_init();
  
  /* It is very important to do the NETSTACK_* initializations right here
   * to enable the PROCESS_YIELD** functionality.
   * The receive process is an single protothread which handles the 
   * received packets. This process needs PROCESS_YIELD_UNTIL().
   **/
  /* Start radio and radio receive process */
  NETSTACK_RADIO.init();
  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();
  printf("RDC: %s. MAC: %s.\n", NETSTACK_RDC.name, NETSTACK_MAC.name);

  init_addresses();

  /* Autostart processes */
  autostart_start(autostart_processes);

  LEDOn(LED3);

  /* Main scheduler loop */
  while(1) {
    process_run();    
  }

  return 0;
}

int putchar_ftdi(char c) {
  char* p = (char*)0x2200;
  //if((PINE & 0x80) != 0) *p = c;
  *p = c;
  return 0;
}

ISR(INT7_vect, ISR_BLOCK) {
  char *p = (char*)0x2200;
  while((PINE & 0x80) == 0) serial_line_input_byte(*p);
  //while((PINE & 0x80) == 0) printf("%c", *p);
  //printf("\n");
}

