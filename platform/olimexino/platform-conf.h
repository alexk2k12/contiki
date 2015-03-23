#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#define TMOTE_MYMOTE 1

#define PROCESS_CONF_NO_PROCESS_NAMES 1

#define CONTIKI_TARGET_OLIMEXINO 1

#define PLATFORM_HAS_LEDS   1
#define PLATFORM_HAS_BUTTON 0

/* CPU target speed in Hz */
#define F_CPU  7372800uL
//#define RTIMER_ARCH_SECOND 14000uL
//#define F_CPU 16000000uL

/* Our clock resolution, this is the same as Unix HZ. */
#define ACLK_FROM_REFO 1
#define CLOCK_CONF_SECOND 128UL
#define RTIMER_CONF_SECOND (4096U*8)

#define BAUD2UBR(baud) (baud)

#define CCIF
#define CLIF

#define HAVE_STDINT_H
#include "msp430def.h"

// --------------------
// --- XMEM MEMORY ----
// --------------------
#define XMEM_NODEID_BASEADDR		0x10

// --------------------
// --- INFO MEMORY ----
// --------------------

#define INFOMEM_START 				0x1800
#define INFOMEM_BLOCK_SIZE 			0x80

#define INFOMEM_NODE_ID				0x0000 /* - 0x0004 */
#define INFOMEM_NODE_ID_SIZE		4
#define INFOMEM_MAC_ADDR			0x0004 /* - 0x000C */
#define INFOMEM_MAC_ADDR_SIZE		8
#define INFOMEM_RIME_ADDR			0x000C /* - 0x000F */
#define INFOMEM_RIME_ADDR_SIZE		4

/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
typedef unsigned long off_t;

/* the low-level radio driver */
//#define NETSTACK_CONF_RADIO   cc2520_driver

#define ROM_ERASE_UNIT_SIZE  512
#define XMEM_ERASE_UNIT_SIZE (64*1024L)


//#define CFS_CONF_OFFSET_TYPE    long


/* Use the first 64k of external flash for node configuration */
//#define NODE_ID_XMEM_OFFSET     (0 * XMEM_ERASE_UNIT_SIZE)

/* Use the second 64k of external flash for codeprop. */
//#define EEPROMFS_ADDR_CODEPROP  (1 * XMEM_ERASE_UNIT_SIZE)


#define MMEM_CONF_SIZE 256

//#define CFS_XMEM_CONF_OFFSET    (2 * XMEM_ERASE_UNIT_SIZE)
//#define CFS_XMEM_CONF_SIZE      (1 * XMEM_ERASE_UNIT_SIZE)

//#define CFS_RAM_CONF_SIZE 4096




/*
 * SPI bus - M25P80 external flash configuration.
 */

#define FLASH_PWR       //3       /* P4.3 Output */
#define FLASH_CS        //4       /* P4.4 Output */
#define FLASH_HOLD      //7       /* P4.7 Output */

/* Enable/disable flash access to the SPI bus (active low). */

#define SPI_FLASH_ENABLE()  //( P4OUT &= ~BV(FLASH_CS) )
#define SPI_FLASH_DISABLE() //( P4OUT |=  BV(FLASH_CS) )

#define SPI_FLASH_HOLD()               // ( P4OUT &= ~BV(FLASH_HOLD) )
#define SPI_FLASH_UNHOLD()              //( P4OUT |=  BV(FLASH_HOLD) )

// at25f512b flash
// flash spi port is shared with mrf49xa port.
//

#define AT25F512B_CS         1       /* P3.1 Output */
#define AT25F512B_PORT(type) P3##type /* P3.1 Output */

#define AT25F512B_SPI_ENABLE()  ( AT25F512B_PORT(OUT) &= ~BV(AT25F512B_CS) )
#define AT25F512B_SPI_DISABLE() ( AT25F512B_PORT(OUT) |=  BV(AT25F512B_CS) )


//
//  MRF49XA configuration and macros
//

// P4.2 - Output: SPI Chip Select (CS_N)
#define MRF49XA_CSN_PORT(type)      P3##type
#define MRF49XA_CSN_PIN             2

// P2.5 - Output: FSK/DATA/FSEL_N (CS_N)
#define MRF49XA_FSELN_PORT(type)      P2##type
#define MRF49XA_FSELN_PIN             5

#define MRF49XA_SPI_ENABLE()     (MRF49XA_CSN_PORT(OUT) &= ~BV(MRF49XA_CSN_PIN)) // ENABLE CSn (active low)
#define MRF49XA_SPI_DISABLE()    (MRF49XA_CSN_PORT(OUT) |=  BV(MRF49XA_CSN_PIN))  // DISABLE CSn (active low)
#define MRF49XA_SPI_IS_ENABLED() ((MRF49XA_CSN_PORT(OUT) & BV(MRF49XA_CSN_PIN)) != BV(MRF49XA_CSN_PIN))


// SPI bus configuration
// SPI input/output registers
#define SPI_TXBUF UCA0TXBUF
#define SPI_RXBUF UCA0RXBUF

#define SPI_WAITFOREOTx()    while ((UCA0STAT & UCBUSY) != 0)  // USART0 Tx ready?
#define SPI_WAITFOREORx()    while ((UCA0IFG & UCRXIFG) == 0)  // USART0 Rx ready?
#define SPI_WAITFORTxREADY() while ((UCA0IFG & UCTXIFG) == 0)  // USART0 Tx buffer ready?

#define MOSI           3 // P3.3 - Output: SPI Master out - slave in (MOSI)
#define MISO           4 // P3.4 - Input:  SPI Master in - slave out (MISO)
#define SCK            7 // P2.7 - Output: SPI Serial Clock (SCLK)

#define MRF49XA_DIO_PIN 4
#define MRF49XA_DIO_PORT(type)      P2##type

// P2.6 - Input: IRQ pin
#define MRF49XA_IRQ_PORT(type)      P2##type
#define MRF49XA_IRQ_PIN             6
#define MRF49XA_IRQ_VECTOR PORT2_VECTOR


// P2.6 - Input: INT pin
/*#define MRF49XA_INT_PORT(type)      P2##type
#define MRF49XA_INT_PIN             5
#define MRF49XA_FIFOP_PIN 5*/

/* FIFOP on external interrupt 0. */
#define MRF49XA_ENABLE_FIFOP_INT()          do { P2IE  |=  BV(MRF49XA_IRQ_PIN); } while (0)
#define MRF49XA_DISABLE_FIFOP_INT()         do { P2IE  &= ~BV(MRF49XA_IRQ_PIN); } while (0)
#define MRF49XA_CLEAR_FIFOP_INT()           do { P2IFG &= ~BV(MRF49XA_IRQ_PIN); } while (0)
#define MRF49XA_EDGE_FALL_INT()             do { P2IES |=  BV(MRF49XA_IRQ_PIN); } while (0)

#define MRF49XA_ENABLE_DIO_INT()            do { P2IE  |=  BV(MRF49XA_DIO_PIN); } while (0)
#define MRF49XA_DISABLE_DIO_INT()           do { P2IE  &= ~BV(MRF49XA_DIO_PIN); } while (0)
#define MRF49XA_CLEAR_DIO_INT()             do { P2IFG &= ~BV(MRF49XA_DIO_PIN); } while (0)
#define MRF49XA_EDGE_FALL_DIO_INT()         do { P2IES |=  BV(MRF49XA_DIO_PIN); } while (0)

//posizione del bit dello switch nella variabile
#define BC_BIT       0x01
#define WU_BIT       0x02
#define BC_EVENT_BIT (BC_BIT<<8)
#define WU_EVENT_BIT (WU_BIT<<8)

/*// Colibri module general purpose pin definition
#define COLIBRI_GPIO_PINS 2

#define PIN0SEL   P1SEL
#define PIN0DIR   P1DIR
#define PIN0BIT   BIT1

#define PIN1SEL   P1SEL
#define PIN1DIR   P1DIR
#define PIN1BIT   BIT2*/

/* LED ports */
#define LEDS_PxDIR P1DIR // port direction register
#define LEDS_PxOUT P1OUT // port register
#define LEDS_CONF_GREEN  BIT5 //red led
#define LEDS_CONF_RED    BIT6 // green led
#define LEDS_CONF_YELLOW BIT7 // yellow led

#endif /* __PLATFORM_CONF_H__ */
