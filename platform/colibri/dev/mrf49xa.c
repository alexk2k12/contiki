#include "contiki.h"
#include "mrf49xa.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"

#include "sys/timetable.h"

#include <string.h>
#include "dev/adc.h"
#include "dev/leds.h"
#include "utils.h"

#define DEBUG 0
#if DEBUG
//#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

void mrf49xa_arch_init(void);

int mrf49xa_packets_seen;

/* The declarations below ensure that the packet buffer is aligned on
   an even 16-bit boundary. On some platforms (most notably the
   msp430), having apotentially misaligned packet buffer may lead to
   problems when accessing 16-bit values. */
static uint16_t mrf49xabuf_aligned[(PACKETBUF_SIZE + PACKETBUF_HDR_SIZE) / 2 + 3];
static uint8_t *mrf49xabuf = (uint8_t *)mrf49xabuf_aligned;
static uint8_t mrf49xa_pending;
static uint8_t mrf49xa_recvlen;
static uint8_t *mrf49xaptr;

uint16_t mrf49xa_start_time;
static uint16_t last_packet_timestamp;
static uint16_t last_packet_len;
static uint32_t rssi;
static uint16_t rssiSample;

static uint16_t rxcreg;
static uint16_t txcreg;
static uint16_t gencreg;
static clock_time_t timeout;
//static uint8_t POR = 1; //power on reset

//clock time is in 7ms tick.
//bytes are trasmitted at 56kbps (142us@byte).
//1 tick equal to 49 byte.
#define TIMEOUT_TICKS 6


/*---------------------------------------------------------------------------*/
PROCESS(mrf49xa_process, "Mrf49xa driver");
/*---------------------------------------------------------------------------*/

// specific defined functions
int getIRQstatus(void){
	uint8_t irq;
	irq =   P2IN;
	//PRINTF("p2: %x\n",irq & (1<<MRF49XA_IRQ_PIN) ? 1 : 0);
	return (irq & (1<<MRF49XA_IRQ_PIN) ? 1 : 0);
}

int getDioStatus(void)
{
	uint8_t dio;
	dio =   P2IN;
	//PRINTF("dio: %x\n",dio & (1<<MRF49XA_DIO_PIN) ? 1 : 0);
	return (dio & (1<<MRF49XA_DIO_PIN) ? 1 : 0);

}


static void startFifo(uint8_t data){
	MRF49XA_WRITE_FIRST_FIFO(data);
}

static void dataToFifo(uint8_t data){
	MRF49XA_WRITE_DATA_FIFO(data);
}

static void stopFifo(uint8_t data){
	MRF49XA_WRITE_LAST_FIFO(data);
}


void setReg(enum mrf49xa_register regname, unsigned value) {
  MRF49XA_WRITE_REG(regname, value);
  //while (!(UCA0IFG&UCTXIFG));               // USCI_A0 TX buffer ready?
  //UCA0TXBUF = 0xaa;                     // Transmit first character
}

void
readSR(uint16_t *value)
{
	MRF49XA_READ_STSREG(value);
	  //while (!(UCA0IFG&UCTXIFG));               // USCI_A0 TX buffer ready?
	  //UCA0TXBUF = 0xaa;                     // Transmit first character
}


void setBatteryTh(uint16_t mV){
	uint16_t th;

	//low and high threshold limit clamping
	if (mV < 2500) mV=2500;
	if (mV > 3800) mV=3800;

	th = (mV - 2250)/100;
	th &= 0xf;
	//PRINTF ("th: 0x%.2X\n",th);
	setReg(MRF49XA_BCSREG,th);

}

// mrf49xa_setDataRate(mrf49xa_baudRate dataRate)
//
// This function take as parameter desired dataRate and set
// RXCREG: RXBW bits used to select baseband receiver bandwidth
// TXCREG: MODBW bits to set trasmitter frequency deviation
// DRSREG: DRPV + DRPE to set trasmitter datarate
//



void mrf49xa_setDataRate(mrf49xa_baudRate dataRate){

	//setting data rate
	uint8_t drpe;
	uint8_t drpv;
	uint8_t rxbw;
	uint8_t modbw;


	/*if (dataRate < 2694)
		drpe = 1;
	else
		drpe = 0;

	dataRate = (29*(1+drpe * 7)*dataRate);
	dataRate = 10000000/dataRate;
	dataRate -= 1;

	uint8_t drpv = (uint8_t) dataRate;
	drpv &= 0x7f;

	//PRINTF("drpe @ 0x%x\n",drpe);
	//PRINTF("drpv @ 0x%x\n",drpv);
	drpv = drpv | drpe << 7;
	PRINTF("write MRF49XA_DRSREG @ 0x%x\n",drpv);
	setReg(MRF49XA_DRSREG,drpv);*/

	switch(dataRate){
	case MRF49XA_2400:
		drpe  = 1;
		drpv  = 0x11;
		rxbw  = MRF49XA_RXBW_270KHZ;
		modbw = MRF49XA_MODBW_120KHZ;
		break;
	case MRF49XA_4800:
		drpe  = 0;
		drpv  = 0x47;
		rxbw  = MRF49XA_RXBW_270KHZ;
		modbw = MRF49XA_MODBW_120KHZ;
		break;
	case MRF49XA_9600:
		drpe  = 0;
		drpv  = 0x23;
		rxbw  = MRF49XA_RXBW_270KHZ;
		modbw = MRF49XA_MODBW_120KHZ;
		break;
	case MRF49XA_19200:
		drpe  = 0;
		drpv  = 0x11;
		rxbw  = MRF49XA_RXBW_270KHZ;
		modbw = MRF49XA_MODBW_135KHZ;
		break;
	case MRF49XA_38400:
		drpe  = 0;
		drpv  = 0x08;
		rxbw  = MRF49XA_RXBW_340KHZ;
		modbw = MRF49XA_MODBW_150KHZ;
		break;
	case MRF49XA_57600:
		drpe  = 0;
		drpv  = 0x05;
		rxbw  = MRF49XA_RXBW_400KHZ;
		modbw = MRF49XA_MODBW_165KHZ;
		break;
	case MRF49XA_115200:
		drpe  = 0;
		drpv  = 0x02;
		rxbw  = MRF49XA_RXBW_400KHZ;
		modbw = MRF49XA_MODBW_225KHZ;
		break;
	case MRF49XA_172400:
		drpe  = 0;
		drpv  = 0x01;
		rxbw  = MRF49XA_RXBW_400KHZ;
		modbw = MRF49XA_MODBW_240KHZ;
		break;
	default: //9600
		drpe  = 0;
		drpv  = 0x23;
		rxbw  = MRF49XA_RXBW_270KHZ;
		modbw = MRF49XA_MODBW_120KHZ;
		break;
	}


	rxcreg &= 0x71F; //maschera dei bit per lasciare invariato FINTDIO, DIORT, RXLNA e reserttare RXBW
	rxcreg |= rxbw;
	//PRINTF("write MRF49XA_RXCREG @ 0x%x\n",rxcreg);
	setReg(MRF49XA_RXCREG,rxcreg);

	txcreg &= 0x7; //bit mask for leave otxpwr bits unchanged
	txcreg |= modbw;
	//PRINTF("write MRF49XA_TXCREG @ 0x%x\n",txcreg);
	setReg(MRF49XA_TXCREG,txcreg);

	drpv = drpv | drpe << 7;
	//PRINTF("write MRF49XA_DRSREG @ 0x%x\n",drpv);
	setReg(MRF49XA_DRSREG,drpv);

}

void mrf49xa_setTxPwr(mrf49xa_oTxPwr txPwr){
	txcreg &= 0xf0; //bit mask for leave MODBW bits unchanged
	txcreg |= txPwr <<  OTXPWR_BIT;
	//PRINTF("write MRF49XA_TXCREG @ 0x%x\n",txcreg);
	setReg(MRF49XA_TXCREG,txcreg);
}

void mrf49xa_setRxRssi(mrf49xa_rxRSSI rxRSSI, mrf49xa_gainLNA gainLNA){
	rxcreg &= 0x7e0; //bit mask for leave RXBW FINTDIO DIORT bits unchanged
	rxcreg |= rxRSSI;
	rxcreg |= gainLNA;
	//PRINTF("write MRF49XA_RXCREG @ 0x%x\n",rxcreg);
	setReg(MRF49XA_RXCREG,rxcreg);
}

// mrf49xa_setChannel(enum band, uint8_t ch)
// this function set band and channel
// Allowed band are 868MHz and 916MHz.
// Allowed channel are fro 0 to 23.
// channel separation: 0.828MHz @ 868MHz, 1.241MHz @ 916MHz

void mrf49xa_setChannel(mrf49xa_band band, uint8_t ch) {
	// Predefined values for band and channel
	if(band == 0xFF) band = MRF49XA_DEF_BAND;
	if(ch > MRF49XA_MAX_CHANNEL) ch = MRF49XA_DEF_CHANNEL;
	//banda
	gencreg &= 0xCF; // leave TSDEN, FIFOEN, LCS bit unchanged
	gencreg |= band << BAND_BIT;
	//PRINTF("write MRF49XA_GENCREG @ %x\n",gencreg);
	setReg(MRF49XA_GENCREG,gencreg);

	//canale
	uint16_t freqb = 96+ch*166+ch/2+ch%2;
	//PRINTF("write MRF49XA_CFSREG @ %d\n",freqb);
	setReg(MRF49XA_CFSREG,freqb);
	// Wait about 1ms for PLL to stabilize.
	// FIXME check if clock_delay(300) is 1ms wait
	clock_delay(300);
}

uint16_t mrf49xa_readRssi() {
	uint8_t i = 8;
	uint32_t timeout;
	uint32_t rssi_sum = 0;
	while(i--) {
		timeout = 5000;
		start_adc(ADC_CH7);
		while(checkAdcBusy() && timeout) {
			timeout--;
		}
		if(timeout) rssi_sum += getAdcSample();
	}
	rssi_sum = (rssi_sum >> 3);
	return (uint16_t)rssi_sum;
}

uint8_t mrf49xa_isReceiving(void){
	return mrf49xa_pending;
}

void resetRadioFlags(){
	mrf49xa_pending = 0;
	mrf49xa_recvlen = 0;
	rssi = 0;
	rssiSample = 0;
}

// driver mandatory functions

void testSpi(void){
	uint16_t reg;
    setBatteryTh(2500);
    readSR(&reg);
    reg &= LBTD_BIT;
    PRINTF("LBTD_BIT: %X\t",reg);

    setBatteryTh(3400);
    readSR(&reg);
    reg &= LBTD_BIT;
    PRINTF("LBTD_BIT: %X\n",reg);
}
#if DEBUG
void mrf49xa_print_hex_byte(uint8_t byte) {
	uint8_t nibble = (byte>>4) & 0x0F;
	putchar(nibble<10?'0'+nibble:'A'+nibble-10);
	nibble = byte & 0x0F;
	putchar(nibble<10?'0'+nibble:'A'+nibble-10);
}
#endif

uint8_t mrf49xa_get_byte(void)
{
  MRF49XA_FSELN_PORT(OUT) &= ~BV(MRF49XA_FSELN_PIN);
  uint16_t data = 0;
  MRF49XA_READ_RXFIFOREG(&data);   // read from fifo
  MRF49XA_FSELN_PORT(OUT) |= BV(MRF49XA_FSELN_PIN);

  return (uint8_t) (data & 0xff);
}

//! @brief
//! mrf49xa_interrupt manage incoming data form radio transceiver.
//! Radio packet is composed by 1 length byte followed by N data bytes with 2 CRC bytes at the
//! end (PAYLOAD + CRC = N). mrf49xa_pending hold pending data. if mrf49xa_pending is zero
//! incoming byte is stored as mrf49xa_pending.
//! When interrupt occurs a byte is read from the radio transceiver. If mrf49xa_pending is
//! not zero incoming byte will be stored in buffer.
//! After N bytes are received a poll request is posted to radio thread that verify
//! if CRC is correct. If CRC is incorrect packet is discarded otherwise a message is sent to
//! network stack that will manage incoming packet.
//! To be noted that if a radio error occurs during length transmission sync with
//! subsequent packet will be lost. To avoid this a timeout has been implemented.
//! If after timeout not all expected byte will be received packet has to be discharged and
//! communication status will be reset.
//! In other words if during broadcast transmissions a packet separation of TIMEOUT_TICKS * 7ms
//! is guaranteed no packet synchronization lost should occur; if packet with acknowledge is
//! transmitted a minimum timeout of TIMEOUT_TICKS * 7ms must be considered


int
mrf49xa_interrupt(void)
{
  uint16_t reg;
  uint8_t adcStatus;
  clock_time_t currTime;

  currTime = clock_time();

  if(!mrf49xa_pending) {
	 if (getDioStatus()){
		 //if there is no pending data first trasmitted byte is packet lenght.
		 mrf49xa_recvlen=mrf49xa_pending=mrf49xa_get_byte();

		 if (mrf49xa_pending > PACKETBUF_SIZE + PACKETBUF_HDR_SIZE){ //more data expected than buffer size.
			 PRINTF("%d\n",mrf49xa_pending);
			 mrf49xa_recvlen=mrf49xa_pending = 0;
	    	 setReg(MRF49XA_FIFORSTREG,   0); //RegisterSet(FIFORSTREG);
	    	 setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch
	    	 readSR(&reg);
		 }
		 else{//DIO status low indicates that there are valid data on radio fifo
			 mrf49xaptr=mrf49xabuf;
			 mrf49xa_start_time=RTIMER_NOW();
			 //reset delle medie dei campioni dell'RSSI
			 rssi = 0;
			 rssiSample = 0;
			 timeout = clock_time()+TIMEOUT_TICKS; //set packet timeout
			 leds_off(LEDS_RED);
		 }

	 }
	 else{ //should never occour. To be verified.
		 mrf49xa_recvlen=mrf49xa_pending = 0;
    	 setReg(MRF49XA_FIFORSTREG,   0); //RegisterSet(FIFORSTREG);
    	 setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch
    	 readSR(&reg);                     //RegisterSet(0x0000);				    // read status byte (read ITs)
	 }

  }
  else if(currTime > timeout){
	  //if byte has arrived after timeout reset all status and exit
	 PRINTF("timeout occurred %d bytes pending\n",mrf49xa_pending);
     mrf49xa_recvlen=mrf49xa_pending = 0;
 	 setReg(MRF49XA_FIFORSTREG,   0); //RegisterSet(FIFORSTREG);
 	 setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch
 	 readSR(&reg);
 	 leds_on(LEDS_RED);
  }
  else {
     *mrf49xaptr++ = mrf49xa_get_byte();
     //rssiSample++;
     adcStatus = getAdcStatus();
     if (!(adcStatus & HIGH_PRIORITY_LOCK)){
    	 if(adcStatus == LOW_PRIORITY_LOCK){
    		 if(!checkAdcBusy()){
    			 rssi+=getAdcSample();
    			 rssiSample++;
    		 }
    	 }
    	 if ((adcStatus & DIRTY) || (adcStatus == 0)){ //se l'adc è sporco o non ho precedenti conversioni in corso
    		 start_adc(ADC_CH7);                        // avvio una nuova conversione
    	 }
     }
     //PRINTF("%c\n",*(mrf49xaptr-1));
     if(!--mrf49xa_pending) {
    	 //mrf49xa_get_byte();
    	 last_packet_timestamp = mrf49xa_start_time;
    	 last_packet_len       = mrf49xa_recvlen;
    	 mrf49xa_packets_seen++;
    	 setReg(MRF49XA_FIFORSTREG,   0); //RegisterSet(FIFORSTREG);
    	 setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch
    	 readSR(&reg);                     //RegisterSet(0x0000);				    // read status byte (read ITs)
    	 /*for (i=0;i<mrf49xa_recvlen;i++)
    		 PRINTF("%c",*(mrf49xabuf+i));
    	 PRINTF("\n");*/
    	 process_poll(&mrf49xa_process);
     }
  }
  return 1;
}

uint8_t crcCheck(uint8_t *buff, uint8_t len){
	uint8_t i=0;
	uint16_t crcExp = (buff[len-2] << 8) | buff[len-1];
	CRCINIRES = 0xFFFF;
	CRCDI = len;
	for (i=0;i<len-2;i++)
		CRCDI = buff[i];

	return !(crcExp ^ CRCINIRES);


}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mrf49xa_process, ev, data) {
	//int i;
	PROCESS_BEGIN();
	PRINTF("mrf49xa_process: started\n");

	while(1) {
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		MRF49XA_DISABLE_FIFOP_INT();
		MRF49XA_CLEAR_FIFOP_INT();
		  /*putchar('R');
		  for(i=0;i<last_packet_len;i++)
			  mrf49xa_print_hex_byte(mrf49xabuf[i]);
		  putchar('\n');*/


		PRINTF("mrf49xa_process: EVENT_POLL\n");
		if (crcCheck(mrf49xabuf,last_packet_len)){

			packetbuf_clear();
			packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, last_packet_timestamp);
			memset(packetbuf_dataptr(),0,PACKETBUF_SIZE + PACKETBUF_HDR_SIZE);
			memcpy(packetbuf_dataptr(),mrf49xabuf,last_packet_len-2);

			packetbuf_set_datalen(last_packet_len-2);
			PRINTF("recv len: %d\n",last_packet_len-2);
#if 0
			int i;
			for(i=0;i<last_packet_len;i++){
			if (( ((char *)mrf49xabuf)[i] >= ' ') && (((char *)mrf49xabuf)[i] <= '~') )
				PRINTF ("%c",((char *)mrf49xabuf)[i]);
			else
				PRINTF (" 0x%02x",((uint8_t *)mrf49xabuf)[i]);
			}
			PRINTF ("\n");
#endif

			/*putchar('*');
			putchar('\n');
			for(i=0;i<rssiSample;i++){
				 print_hex_buff(&(samples[i]),2);
			     putchar('\n');
			}*/
			/*putchar('r');
			putchar(':');
			uint16_t rssiDbg = (rssi / rssiSample);
			print_hex_buff(&(rssiDbg),2);
		    putchar('\n');*/

			if (rssiSample){
				//PRINTF("sample: %d\n",rssiSample);
				rssi /= rssiSample;
				packetbuf_set_attr(PACKETBUF_ATTR_RSSI, (uint16_t)rssi);
			} else {
				packetbuf_set_attr(PACKETBUF_ATTR_RSSI, 0);
			}

			PRINTF("mrf49xa_process: input\n");
			NETSTACK_RDC.input();
		}
		mrf49xa_pending = 0;
		MRF49XA_ENABLE_FIFOP_INT();
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
mrf49xa_init(mrf49xa_baudRate baud, mrf49xa_oTxPwr pwr, mrf49xa_band band, uint8_t channel)
{
	resetRadioFlags();

    PRINTF("MRF49XA init...\n");
	MRF49XA_DISABLE_FIFOP_INT();
    rxcreg  = MRF49XA_LNA_20DB | 0x500; //valore del registro MRF49XA_RXCREG
    txcreg  = 0x0;              //valore del registro MRF49XA_TXCREG
    gencreg = 0x0;

	uint16_t reg;
    {
      int s = splhigh();
      mrf49xa_arch_init();		/* Initalize ports and SPI. */
      //CC2420_DISABLE_FIFOP_INT();
      //CC2420_FIFOP_INT_INIT();
      splx(s);
    }

    setReg(MRF49XA_FIFORSTREG,    0); // RegisterSet(FIFORSTREG);
    setReg(MRF49XA_FIFORSTREG, 0x02); //RegisterSet(FIFORSTREG | 0x0002);       // enable synchron latch
    setReg(MRF49XA_GENCREG, gencreg); //RegisterSet(GENCREG);
    setReg(MRF49XA_AFCCREG,    0xF7); //RegisterSet(AFCCREG);
    setReg(MRF49XA_CFSREG,        0); //RegisterSet(CFSREG);
    setReg(MRF49XA_DRSREG,     0x23); //RegisterSet(DRVSREG); @9578 bps
    setReg(MRF49XA_PMCREG,     0x04); //RegisterSet(PMCREG);
    setReg(MRF49XA_RXCREG,   rxcreg); //RegisterSet(RXCREG);
    setReg(MRF49XA_TXCREG,   txcreg); //RegisterSet(TXCREG);
    setReg(MRF49XA_BBFCREG,   0xaf);  // FTYPE digital

    // antenna tuning on startup
    setReg(MRF49XA_PMCREG,     0x21); //RegisterSet(PMCREG | 0x0020);           // turn on the transmitter
    clock_wait(3);                           // wait 10ms for oscillator to stablize
    // end of antenna tuning
    setReg(MRF49XA_PMCREG,     0xd9); //RegisterSet(PMCREG | 0x0080);           // turn off transmitter, turn on receiver
    //setReg(MRF49XA_FIFORSTREG, 0x40); //RegisterSet(GENCREG | 0x0040);          // enable the FIFO
    setReg(MRF49XA_FIFORSTREG,    0); //RegisterSet(FIFORSTREG);
    setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch
    readSR(&reg);                     //RegisterSet(0x0000);				    // read status byte (read ITs)
//    setReg(MRF49XA_BBFCREG,   0xaf);  // FTYPE digital
   //setBatteryTh(2500);               //set low battery threshold

    mrf49xa_setChannel(band,channel);
	mrf49xa_setRxRssi(MRF49XA_RSSI_79DB,MRF49XA_LNA_0DB);
    setReg(MRF49XA_GENCREG, gencreg |FIFOEN); //RegisterSet(GENCREG);
    //clock_wait(1);
    //mrf49xa_get_byte();
    //mrf49xa_get_byte();
    //PRINTF("sts reg: %x\n",reg);
    MRF49XA_CLEAR_FIFOP_INT();
	MRF49XA_ENABLE_FIFOP_INT();
	process_start(&mrf49xa_process, NULL);

	mrf49xa_setDataRate(baud);
	mrf49xa_setTxPwr(pwr);

    //PRINTF("Done!!!\n");

    return 0;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
	int i;
	/*putchar('T');
	for(i=0;i<payload_len;i++)
		mrf49xa_print_hex_byte(((uint8_t*)payload)[i]);
	putchar('\n');*/

	//MRF49XA_FSELN_PORT(OUT) |= BV(MRF49XA_FSELN_PIN);
	MRF49XA_DISABLE_FIFOP_INT();
#if DEBUG
	PRINTF("len: %d\n",payload_len);
	for (i=0;i<payload_len;i++){
		if (( ((char *)payload)[i] >= ' ') && (((char *)payload)[i] <= '~') )
			PRINTF ("%c",((char *)payload)[i]);
		else
			PRINTF (" 0x%02x",((uint8_t *)payload)[i]);
	}
	PRINTF ("\n");
#endif
    // Turn off receiver, enable the TX register
	setReg(MRF49XA_PMCREG,    0);                  // RegisterSet(PMCREG);
	gencreg &= 0x3F;                               // leave FBS+LCS bit unchanged
	setReg(MRF49XA_GENCREG,   gencreg | TXDEN);

	// enable transmitter
	setReg(MRF49XA_PMCREG,    0x20); //    RegisterSet(PMCREG | 0x0020);
	while(getIRQstatus());  //attendo inizio trasmissione
    startFifo(0xAA);                 // FIFO write to register 0xB8 (do not deassert CS)
	//while(!getIRQstatus()); //aspetto che sia stato trasmesso il primo byte

    CRCINIRES = 0xFFFF;

    i=-3;
    while (i<(int)payload_len){
    	while(getIRQstatus());
    	switch(i){
    	case -3: //0x2d
        	dataToFifo(0x2d);
    		break;
    	case -2: //0xd4
        	dataToFifo(0xd4);
    		break;
    	case -1: //payload len
        	dataToFifo(payload_len+2);
        	CRCDI = payload_len+2;
    		break;
    	default:
        	dataToFifo(((uint8_t *)payload)[i]);
        	CRCDI = ((uint8_t *)payload)[i];
        	break;
    	}
    	//PRINTF("sent byte %d\n",i);
    	i++;
    }

	while(getIRQstatus());
    dataToFifo((uint8_t)(CRCINIRES >> 8));  //CRC[15:8]

	while(getIRQstatus());
    dataToFifo((uint8_t)(CRCINIRES & 0xff));  //CRC[7:0]

	while(getIRQstatus());
    dataToFifo(0xAA);  //dummy byte to flush fifo

	while(getIRQstatus());
    stopFifo(0xAA);  //dummy byte to flush fifo

    //PRINTF("CRC: 0x%04X\n",CRCINIRES);

    // Turn off the transmitter, disable the Tx register
    setReg(MRF49XA_PMCREG,    0xd9);    //RegisterSet(PMCREG | 0x0080);
	gencreg &= 0x3F;                    // leave FBS+LCS bit unchanged
	gencreg |= 0x40;
    setReg(MRF49XA_GENCREG,   gencreg);    //RegisterSet(GENCREG | 0x0040 );
    setReg(MRF49XA_FIFORSTREG,    0); //RegisterSet(FIFORSTREG);
    setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch

    uint16_t reg;
    readSR(&reg);                     //serve a far tornare alto IRQ

    MRF49XA_IRQ_PORT(DIR) &= ~BV(MRF49XA_IRQ_PIN);
    MRF49XA_CLEAR_FIFOP_INT();
	MRF49XA_ENABLE_FIFOP_INT();


  //putchar('\n');
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
read(void *buf, unsigned short buf_len)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int on(void) {

	resetRadioFlags();
	//leds_on(LEDS_BLUE);
	//spi_init();
	adc_init();
	// all input by default, set these as output
	MRF49XA_CSN_PORT(DIR) |= BV(MRF49XA_CSN_PIN);
	MRF49XA_IRQ_PORT(DIR) &= ~BV(MRF49XA_IRQ_PIN);
	MRF49XA_DIO_PORT(DIR) &= ~BV(MRF49XA_DIO_PIN);

	MRF49XA_FSELN_PORT(DIR) |= BV(MRF49XA_FSELN_PIN);
	MRF49XA_FSELN_PORT(OUT) |= BV(MRF49XA_FSELN_PIN);

	//AT25F512B_PORT(DIR) |=  BV(AT25F512B_CS) ;
	//AT25F512B_PORT(OUT) |=  BV(AT25F512B_CS) ;
	//interrupt is input pin
	//MRF49XA_INT_PORT(DIR) &= ~BV(MRF49XA_IRQ_PIN);

	MRF49XA_DISABLE_FIFOP_INT();
	MRF49XA_EDGE_FALL_INT();
	MRF49XA_CLEAR_FIFOP_INT();
	MRF49XA_ENABLE_FIFOP_INT();

	P3SEL |= BIT3|BIT4;                       // P3.3,4 option select
	P2SEL |= BIT7;                            // P2.7 option select
	P2IFG = 0;

	/* XXX Clear pending interrupts before enable */
	UCA0IE &= ~UCRXIFG;
	UCA0IE &= ~UCTXIFG;
	UCA0CTL1 &= ~UCSWRST;                      // **Put state machine in reset**

	/*if(POR){
		clock_wait(30);
		POR = 0;
	}
	// antenna tuning on startup
	setReg(MRF49XA_PMCREG,     0x21);	// turn on the transmitter
	clock_wait(3);						// wait 10ms for oscillator to stablize*/
	setReg(MRF49XA_PMCREG,     0xd9);	// turn off transmitter, turn on receiver

	setReg(MRF49XA_FIFORSTREG,   0); //RegisterSet(FIFORSTREG);
	setReg(MRF49XA_FIFORSTREG,0x82);  //RegisterSet(FIFORSTREG | 0x0082);       // enable synchron latch
	uint16_t reg;
	readSR(&reg);                     //serve a far tornare alto IRQ*/
	clock_delay(150);                 //TODO verificare attesa di 250us

	AT25F512B_PORT(OUT) &=  ~BV(AT25F512B_CS) ;
	SPI_WRITE(0xAB);
	AT25F512B_PORT(OUT) |=  BV(AT25F512B_CS) ;

	return 0;
}
/*---------------------------------------------------------------------------*/
static int off(void) {
	//leds_off(LEDS_BLUE);

	adcOff();

	AT25F512B_PORT(OUT) &=  ~BV(AT25F512B_CS) ;
	SPI_WRITE(0xB9);
	AT25F512B_PORT(OUT) |=  BV(AT25F512B_CS) ;

    setReg(MRF49XA_GENCREG,   gencreg);    //RegisterSet(GENCREG | 0x0040 );
	setReg(MRF49XA_PMCREG,     0x0);
	//clock_wait(30);
	UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	P3SEL &= ~(BIT3|BIT4);                    // P3.3,4 option select
	P2SEL &= ~BIT7;
	// P2.7 option select
	P3DIR |= BIT3; P3OUT &= ~BIT3;
	P2DIR |= BIT7; P2OUT &= ~BIT7;
	//P3DIR &= ~BIT4;
	return 0;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver mrf49xa_driver =
  {
    mrf49xa_init,
    prepare,
    transmit,
    send,
    read,
    channel_clear,
    receiving_packet,
    pending_packet,
    on,
    off,
  };
/*---------------------------------------------------------------------------*/



