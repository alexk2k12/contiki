#include <contiki.h>
#include <random.h>
#include <dev/serial-line.h>
#include <leds.h>
#include <net/rime.h>
#include <msp430f5510.h>
#include <watchdog.h>
#include <msp430.h>
#include <stdint.h>

//#include "usb_printf.h"
//#include "dev/mrf49xa.h"
#include "dev/infomem.h"
//#include "dev/adc.h"
//#include "olimexino-lpm.h"
#include "node-id-olimexino.h"


#ifdef USE_MRF49XA
#undef USB_MRF49XA
#endif

#define SERIAL_LINE_USB 1
#ifdef SERIAL_LINE_USB
#include "USB/USB_API/USB_Common/types.h"               //Basic Type declarations
#include "USB/USB_API/USB_Common/usb.h"                 //USB-specific functions

#include "F5xx_F6xx_Core_Lib/HAL_UCS.h"
#include "F5xx_F6xx_Core_Lib/HAL_PMM.h"

#include <intrinsics.h>
#include <string.h>
#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "usbConstructs.h"
#else
#include <dev/uart1.h>
#endif

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define USE_BUTTON_SENSOR=1
#ifdef USE_BUTTON_SENSOR
#include "dev/button-sensor.h"
SENSORS(&button_sensor);
#endif

#define NODEID_RESTORE_RETRY 8

//Indicates data has been received without an open rcv operation
volatile BYTE bCDCDataReceived_event = FALSE;

static void set_rime_addr(void) {
	int i; rimeaddr_t n_addr;

	memcpy(&n_addr, INFOMEM_STRUCT_A->addresses.rimeAddr, sizeof(rimeaddr_t));
	rimeaddr_set_node_addr(&n_addr);

	PRINTF("Rime started with address ");
	for(i = 0; i < sizeof(n_addr.u8) - 1; i++) PRINTF("%d.",n_addr.u8[i]);
	PRINTF("%d\n", n_addr.u8[i]);
}

#ifdef SERIAL_LINE_USB
void init_clock(void) {
    //enable XT2 pins for F5529
//    P5DIR |= 0x0C;
//
//    //use REFO for FLL and ACLK
//    //UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
//    //UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);
//
//    //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
//    //Start the FLL, at the freq indicated by the config
//    //constant USB_MCLK_FREQ
//    //Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);
//    //Start the "USB crystal"
//    XT2_Start(XT2DRIVE_3);

// Taken from the olimexino example code.
//{
    if (USB_PLL_XT == 2){
	
	//	#if defined (__MSP430F552x) || defined (__MSP430F550x )
			P5SEL |= 0x0C;                                        //enable XT2 pins for F5529
	//	#elif defined (__MSP430F563x_F663x)
	//		P7SEL |= 0x0C;
	//	#endif
                        
      int timeout = 5000;
//      P1DIR |= 0x01;                   //set P1.0 as output
//      P1SEL |= 0x01;                   //set P1.0 as ACLK function

      P5DIR &= ~(BIT5 + BIT4);
      P5SEL |= BIT5 + BIT4;           //enable XT1 pins
      
      UCSCTL6 |= XCAP0 + XCAP1;
      UCSCTL6 = UCSCTL6 & (~XT1OFF);            //set XT1OFF=0 (bit0 = 0) --> enable XT1
                        
                        
      //use REFO for FLL 
      UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
      //use XT1CLK for ACLK
      UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__XT1CLK);      
      
      do
      {
        if (!(UCSCTL7 & XT1LFOFFG)) break;
        UCSCTL7 = UCSCTL7 & ~(0x0F);            // Clear XT2,XT1,DCO fault flags
        SFRIFG1 = SFRIFG1 & ~(0x02);            // Clear fault flags (SFRIFG1.OFIFG = 0)
        timeout--;
      }
      while ((SFRIFG1 & OFIFG) && timeout);      // Test oscillator fault flag

      UCSCTL6 = (UCSCTL6 & ~(1<<6)) & ~(1<<7);  //XT1Drive = 00; --> oscilator range 4-8 MHz      
      
      //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
      Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);   //Start the FLL, at the freq indicated by the config
                                                                        //constant USB_MCLK_FREQ
     
      XT2_Start(XT2DRIVE_0);                                          //Start the "USB crystal"
            
    } 
    else {
            #if defined (__MSP430F552x) || defined (__MSP430F550x)
                    P5SEL |= 0x10;                                    //enable XT1 pins
            #endif
    //Use the REFO oscillator to source the FLL and ACLK
    UCSCTL3 = SELREF__REFOCLK;
    UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);

//}
}
}

void usb_printf_init(void) {
    SetVCore(3);
    init_clock();
    //Init USB
    USB_init();
    //Enable various USB event handling routines
    USB_setEnabledEvents(kUSB_VbusOnEvent +
                         kUSB_VbusOffEvent +
                         kUSB_receiveCompletedEvent +
                         kUSB_dataReceivedEvent +
                         kUSB_UsbSuspendEvent +
                         kUSB_UsbResumeEvent +
                         kUSB_UsbResetEvent);
    // See if we're already attached physically to USB, and if so,
    // connect to it. Normally applications don't invoke the event
    // handlers, but this is an exception.

    if(USB_connectionInfo() & kUSB_vbusPresent) {
        //usb_printf_state = USB_ENABLED;
            USB_handleVbusOnEvent();
	PJDIR=0xff;
    	PJOUT=0xff;

    }

}

/** Initializes modules and pins */
void msp_init(void) {
	//P1DIR |= BIT5|BIT6|BIT7;

	//*taken from example code.
    //Initialization of ports (all unused pins as outputs with low-level
    P1OUT = 0x00;
    P1DIR = 0x00;
    P2REN |= BIT0;                //Enable BUT2 pullup
    P2OUT = 0x01;
    P2DIR = 0x00;      
    P4OUT = 0x00;
    P4DIR = 0x00;
    P5OUT = 0x00;
    P5DIR = 0x00;
    P6OUT = 0x00;
    P6DIR = 0x00;
    PJDIR = BIT3;
    PJOUT |= BIT1;
    PJOUT &= ~BIT3;


    SFRIE1 &= ~WDTIE;

    // Initialize USB interface
    usb_printf_init();
    // Enable global interrupts
    __enable_interrupt();
}
#endif

//static void lpm_uart_enter(void) {
//	/* RS232 */
//	UCA1CTL1 |= UCSWRST;            /* Hold peripheral in reset state */
//	P4SEL &= ~(BIT4|BIT5);			// P4.5 4.6 = USCI_A1 TXD/RXD
//	/* Clear pending interrupts*/
//	UCA1IE &= ~UCRXIFG;
//	UCA1IE &= ~UCTXIFG;
//}
//
//static void lpm_msp430_enter(void) {
//	UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | SELA_1 ; 		// Set ACLK = VLO
//
//#if OLIMEXINO_USE_DISPLAY //if display leave BTN1 as input
//	P1DIR &= ~ (BIT0);
//#else
//	P1DIR |= BIT0;  // if not display BTN1 as output
//#endif
//#if OLIMEXINO_USE_BATTERY_CHARGER //if battery charger leave VEXT_PRES as input
//	P1DIR &= ~ (BIT3);
//#else
//	P1DIR |= BIT3;// if not battery charger leave VEXT_PRES as output
//#endif
//	P1DIR |= 0xF6;           //all other pin as output
//
//	P2DIR = ~ (BIT6|BIT4);  //IRQ, INT as input
//	P3DIR = 0xFF;           //all output
//#if OLIMEXINO_USE_BUTTON_SENSOR
//	P4DIR |= (BIT5 | BIT4); //uart line as output
//#else
//	P4DIR = 0xFF;           //all output changed if SENSORS_ENABLED
//#endif
//	P5DIR = 0xFF;           //all output
//	//P6DIR = ~ (BIT0);       //LBI input. RSSIO as output. Pin low when radio is sleeping
//	P6DIR = 0xFF;
//	PJDIR = 0xFF;           //all output
//
//
///*
//#if OLIMEXINO_USE_BUTTON_SENSOR
//	P1DIR &= ~(BIT3); //VEXT_PRES input
//	P1DIR &= ~(BIT1|BIT0); //BTN1 + BTN0 input
//	P4DIR &= ~(BIT2|BIT1); //BTN3 + BTN2 input
//#endif
//*/
//	P1OUT = 0x00;
//	P2OUT = BIT5;
//	P3OUT = BIT2|BIT1;
//	P4OUT = 0x00;
//	P5OUT = 0x00;
//	P6OUT = 0x00;
//	PJOUT = 0x00;
//
//}
//
//static void lpm_uart_exit() {
//	P4SEL |= BIT4|BIT5;  	// P4.5 4.6 = USCI_A1 TXD/RXD
//
//	UCA1IE &= ~UCRXIFG;  	/* Clear pending interrupts before enable */
//	UCA1IE &= ~UCTXIFG;
//
//	UCA1CTL1 &= ~UCSWRST;	/* Initialize USCI state machine **before** enabling interrupts */
//	UCA1IE |= UCRXIE;		/* Enable UCA1 RX interrupt */
//}
//
//static void lpm_msp430_exit(void) {
//	UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | SELA_2 ; // Set ACLK = REFO
//
//	P6DIR &= ~ (BIT7|BIT0);       //RSSIO as input + LBI as input
//#ifdef SENSORS_ENABLED
//#if BATTERY_CHARGER
//	P6DIR &= ~ (BIT1|BIT2);   //VSENSE, ISENSE as input
//#endif
//#endif
//}
//
//static void lpm_enter(void) {
//	//leds_on(LEDS_BLUE);
//	//leds_off(LEDS_BLUE);
//	CLEAR_LPM_REQUEST(LPM_IS_ENABLED);
//	lpm_uart_enter();
//	lpm_msp430_enter();
//}
//
//static void lpm_exit(void) {
//	CLEAR_LPM_REQUEST(LPM_IS_DISABLED);
//	lpm_msp430_exit();
//	lpm_uart_exit();
//}

uint16_t getRandomIntegerFromVLO(void)
{
  uint8_t i;
  uint16_t result = 0;
  UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | SELA_1 ; // Set ACLK = VLO

  // setup Timer_A
  // on msp430f53xx ACLK is connected to CCI2B
  TA0CCTL2 = CM_1 + CCIS_1 + CAP;
  TA0CTL |= TASSEL__SMCLK + MC__CONTINOUS;
  leds_on(LEDS_RED);
  for(i=0 ; i<16 ; i++)
  {
    // shift left result
    result <<= 1;
    // wait until Capture flag is set
    while(!(TA0CCTL2 & CCIFG));

    // clear flag
    TA0CCTL2 &= ~CCIFG;

    // check LSB
    if(TA0CCR2 & 0x01)
    {
      result |= 0x01;
    }

    // change the divison of timer input clock
    //TA0CTL = (TA0CTL & 0xFCFF) | ((TA0CCR2 & 0x03) << 8);
  }

  UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | SELA_2 ; // Set ACLK = REFO
  //disable timer A
  TA0CCTL2 = 0;
  TA0CTL   = 0;
  leds_off(LEDS_RED);

  return result;
}

int main(void) {
	msp430_cpu_init();
	leds_init();

#ifdef SERIAL_LINE_USB
    msp_init();
#else
#ifdef SERIAL_LINE_OUTPUT_ENABLED
    uart1_init(115200);
#endif
#endif

    clock_init();
    rtimer_init();
    process_init();
    process_start(&etimer_process, NULL);
    ctimer_init();
    process_start(&sensors_process,NULL);

	uint8_t i;
	for(i=0;i<4;i++){
		leds_off(LEDS_ALL);
		switch(i){
		case 0:
			leds_on(LEDS_RED);
			break;
		case 1:
			leds_on(LEDS_GREEN);
			break;
		case 2:
			leds_on(LEDS_YELLOW);
			break;
		}
		clock_wait(20);
	}

//    adc_init();
    set_rime_addr();
#ifdef USE_MRF49XA
    {
		uint8_t baud = INFOMEM_STRUCT_A->radio.baudRate == 0xFF ? MRF49XA_57600 : INFOMEM_STRUCT_A->radio.baudRate;
		uint8_t txpwr = INFOMEM_STRUCT_A->radio.txPower == 0xFF ? MRF49XA_TXPWR_0DB : INFOMEM_STRUCT_A->radio.txPower;
		mrf49xa_init(baud,txpwr,INFOMEM_STRUCT_A->radio.band,INFOMEM_STRUCT_A->radio.channel);
    }
#endif
	//to avoid errata read from flash that sometimes occour at powerup (tapullo)
	//for(int i=NODEID_RESTORE_RETRY;i!=0;i--)
	//{
	node_id_restore();
	/*	if(node_id_olimexino)
			break;
	}*/

    //random_init((unsigned short)node_id_olimexino);
    uint16_t seed = getRandomIntegerFromVLO();
    random_init((unsigned short)seed);


#ifdef SERIAL_LINE_INPUT_ENABLED
#ifdef SERIAL_LINE_USB
    serial_line_init();
#else
    uart1_set_input(serial_line_input_byte);
    serial_line_init();
#endif
#endif
    /*uint8_t i;
    for(i=0;i<16;i++){
        print_hex_buff(&seed,2);
        putchar('\n');
        seed = getRandomIntegerFromVLO();
    }*/

    autostart_start(autostart_processes);
    watchdog_start();

    // Main loop
    while(1) {  
        int r;
        do {
          watchdog_periodic();
          r = process_run();
        } while(r > 0);
#ifdef SERIAL_LINE_USB
        if(process_nevents() != 0 /*|| uart1_active()*/) {
        } else {
            watchdog_stop();
            watchdog_start();
        }
#else
        int s = splhigh();		/* Disable interrupts. */
        if(process_nevents() != 0 || uart1_active()) {
        	splx(s);			/* Re-enable interrupts. */
        } else {
        	if(IS_LPM_REQUESTED(LPM_IS_DISABLED)) lpm_enter();
            watchdog_stop();
            (lpm_active()) ? __bis_SR_register(GIE | LPM3_bits) : __bis_SR_register(GIE | LPM0_bits) ;
            watchdog_start();
            if(IS_LPM_REQUESTED(LPM_IS_ENABLED)) lpm_exit();
        }
#endif
        /*if(events_available()) {
            process_events();
        }*/
    }
}

