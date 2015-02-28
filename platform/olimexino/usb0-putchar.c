#include <stdio.h>
//#include "usb_printf.h"
#include "USB/USB_API/USB_Common/types.h"
#include "USB/usbConstructs.h"
#include <descriptors.h>

#ifdef SERIAL_LINE_CONF_BUFSIZE
#define BUFSIZE SERIAL_LINE_CONF_BUFSIZE
#else /* SERIAL_LINE_CONF_BUFSIZE */
#define BUFSIZE 128
#endif /* SERIAL_LINE_CONF_BUFSIZE */

#if (BUFSIZE & (BUFSIZE - 1)) != 0
#error SERIAL_LINE_CONF_BUFSIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change SERIAL_LINE_CONF_BUFSIZE in contiki-conf.h.
#endif

//extern uint8_t usb_printf_state;
uint8_t outBuff[BUFSIZE];
uint8_t outBuffPos=0;

int putchar(int c) {
	//uint8_t ch = (uint8_t)c;
    //if(usb_printf_state == USB_DISABLED) return -1;
    //if(usb_printf_state == USB_LOCKED) return -1;
    //usb_printf_state = USB_LOCKED;
	outBuff[outBuffPos++] = c;
	if(c=='\n'||outBuffPos==BUFSIZE) {
		cdcSendDataWaitTilDone(outBuff,outBuffPos, CDC0_INTFNUM, 5000);
		outBuffPos=0;
	}
	//usb_printf_state = USB_ENABLED;
	return c;
}
