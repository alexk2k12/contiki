#ifndef MRF49XA_H
#define MRF49XA_H

#include "contiki.h"
#include "dev/spi.h"
#include "dev/radio.h"

#define OTXPWR_BIT 0
// Power is -xdB so MRF49XA_0DB is the maximum output power
typedef enum _mrf49xa_oTxPwr {
	MRF49XA_TXPWR_17DB  = 7 ,
	MRF49XA_TXPWR_15DB  = 6 ,
	MRF49XA_TXPWR_12DB  = 5 ,
	MRF49XA_TXPWR_10DB  = 4 ,
	MRF49XA_TXPWR_7DB   = 3 ,
	MRF49XA_TXPWR_5DB   = 2 ,
	MRF49XA_TXPWR_2DB   = 1 ,
	MRF49XA_TXPWR_0DB   = 0 ,
}mrf49xa_oTxPwr;

typedef enum _mrf49xa_baudRate {
	MRF49XA_2400,
	MRF49XA_4800,
	MRF49XA_9600,
	MRF49XA_19200,
	MRF49XA_38400,
	MRF49XA_57600,
	MRF49XA_115200,
	MRF49XA_172400
} mrf49xa_baudRate;

#define RXBW_BIT 5
enum mrf49xa_rxBw{
	MRF49XA_RXBW_67KHZ  = (6 << RXBW_BIT),
	MRF49XA_RXBW_134KHZ = (5 << RXBW_BIT),
	MRF49XA_RXBW_200KHZ = (4 << RXBW_BIT),
	MRF49XA_RXBW_270KHZ = (3 << RXBW_BIT),
	MRF49XA_RXBW_340KHZ = (2 << RXBW_BIT),
	MRF49XA_RXBW_400KHZ = (1 << RXBW_BIT)

};

#define MODBW_BIT 4
enum mrf49xa_modBw{
	MRF49XA_MODBW_240KHZ = (15 << MODBW_BIT),
	MRF49XA_MODBW_225KHZ = (14 << MODBW_BIT),
	MRF49XA_MODBW_210KHZ = (13 << MODBW_BIT),
	MRF49XA_MODBW_195KHZ = (12 << MODBW_BIT),
	MRF49XA_MODBW_180KHZ = (11 << MODBW_BIT),
	MRF49XA_MODBW_165KHZ = (10 << MODBW_BIT),
	MRF49XA_MODBW_150KHZ = (9 << MODBW_BIT),
	MRF49XA_MODBW_135KHZ = (8 << MODBW_BIT),
	MRF49XA_MODBW_120KHZ = (7 << MODBW_BIT),
	MRF49XA_MODBW_105KHZ = (6 << MODBW_BIT),
	MRF49XA_MODBW_90KHZ  = (5 << MODBW_BIT),
	MRF49XA_MODBW_75KHZ  = (4 << MODBW_BIT),
	MRF49XA_MODBW_60KHZ  = (3 << MODBW_BIT),
	MRF49XA_MODBW_45KHZ  = (2 << MODBW_BIT),
	MRF49XA_MODBW_30KHZ  = (1 << MODBW_BIT),
	MRF49XA_MODBW_15KHZ  = (0 << MODBW_BIT)

};
#define RX_RSSI_BIT 0
// RSSI is -xdB so MRF49XA_103DB is the maximum sensibility allowed
typedef enum _mrf49xa_rxRSSI {
	MRF49XA_RSSI_73DB  = (5 << RX_RSSI_BIT),
	MRF49XA_RSSI_79DB  = (4 << RX_RSSI_BIT),
	MRF49XA_RSSI_85DB  = (3 << RX_RSSI_BIT),
	MRF49XA_RSSI_91DB  = (2 << RX_RSSI_BIT),
	MRF49XA_RSSI_97DB  = (1 << RX_RSSI_BIT),
	MRF49XA_RSSI_103DB = (0 << RX_RSSI_BIT),
} mrf49xa_rxRSSI;

#define GAIN_LNA_BIT 3
// RSSI is -xdB so MRF49XA_103DB is the maximum sensibility allowed
typedef enum _mrf49xa_gainLNA {
	MRF49XA_LNA_20DB  = (3 << GAIN_LNA_BIT),
	MRF49XA_LNA_14DB  = (2 << GAIN_LNA_BIT),
	MRF49XA_LNA_6DB   = (1 << GAIN_LNA_BIT),
	MRF49XA_LNA_0DB   = (0 << GAIN_LNA_BIT),
} mrf49xa_gainLNA;

#define BAND_BIT 4
// Band allowed are 868/915
typedef enum _mrf49xa_band {
	MRF49XA_BAND_433  = 1 ,
	MRF49XA_BAND_868  = 2 ,
	MRF49XA_BAND_915  = 3 ,
} mrf49xa_band;

#define MRF49XA_MAX_CHANNEL 23

int mrf49xa_interrupt(void);
int mrf49xa_init(mrf49xa_baudRate baud, mrf49xa_oTxPwr pwr, mrf49xa_band band, uint8_t channel);
void testSpi(void);


void mrf49xa_setDataRate(mrf49xa_baudRate dataRate);
void mrf49xa_setTxPwr(mrf49xa_oTxPwr txPwr);
void mrf49xa_setRxRssi(mrf49xa_rxRSSI rxRSSI, mrf49xa_gainLNA gainLNA);
void mrf49xa_setChannel(mrf49xa_band band, uint8_t ch);
//void mrf49xa_setFifoRstReg(uint8_t value);
uint16_t mrf49xa_readRssi();
uint8_t mrf49xa_isReceiving(void);

#define TXDEN  0x80
#define FIFOEN 0x40



extern const struct radio_driver mrf49xa_driver;

#define LBTD_BIT  0x400


enum mrf49xa_register {
         MRF49XA_STSREG      =  0x00 ,
         MRF49XA_GENCREG     =  0x80 ,
         MRF49XA_AFCCREG     =  0xC4 ,
         MRF49XA_TXCREG      =  0x98 ,
         MRF49XA_TXBREG      =  0xb8 ,
         MRF49XA_CFSREG      =  0xa6 ,
         MRF49XA_RXCREG      =  0x90 ,
         MRF49XA_BBFCREG     =  0xc2 ,
         MRF49XA_RXFIFOREG   =  0xb0 ,
         MRF49XA_FIFORSTREG  =  0xca ,
         MRF49XA_SYNBREG     =  0xce ,
         MRF49XA_DRSREG      =  0xc6 ,
         MRF49XA_PMCREG      =  0x82 ,
         MRF49XA_WTSREG      =  0xe1 ,
         MRF49XA_DCSREG      =  0xc8 ,
         MRF49XA_BCSREG      =  0xc0 ,
         MRF49XA_PLLCREG     =  0xcc };

void setReg(enum mrf49xa_register regname, unsigned value);
void readSR(uint16_t *value);

//***********************************************************************/
// Additional SPI Macros for the MRF49XA */
//************************************************************************/
// Write to a register in the MRF49XA

#define MRF49XA_WRITE_REG(adr,data)                           \
  do {                                                       \
	PRINTF("write reg: 0x%x\t@: 0x%x\n",adr,data);\
    adr = adr | (data >>8);                                \
	SPI_WAITFORTx_BEFORE();                                \
    MRF49XA_SPI_ENABLE();                                    \
    SPI_WRITE(adr);                                     \
    SPI_WRITE((uint8_t)(data & 0xff));                  \
    MRF49XA_SPI_DISABLE();                                   \
  } while(0)

// start Write FIFO (assert cs)
#define MRF49XA_WRITE_FIRST_FIFO(data)                     \
  do {                                                     \
	  SPI_WAITFORTx_BEFORE();                              \
      MRF49XA_SPI_ENABLE();                                \
	  SPI_WRITE(0xB8);                                      \
      SPI_WRITE((uint8_t)(data & 0xff));                   \
  } while(0)

// Write FIFO byte
#define MRF49XA_WRITE_DATA_FIFO(data)                           \
  do {                                                     \
	  SPI_WAITFORTx_BEFORE();                                \
      SPI_WRITE((uint8_t)(data & 0xff));                  \
  } while(0)

//end Write FIFO (deassert CS)
#define MRF49XA_WRITE_LAST_FIFO(data)                           \
  do {                                                     \
	  SPI_WAITFORTx_BEFORE();                                \
      SPI_WRITE((uint8_t)(data & 0xff));                  \
      MRF49XA_SPI_DISABLE();                                   \
  } while(0)

/* Read a register in the MRF49XA */
#define MRF49XA_READ_STSREG(data)                       \
  do {                                                  \
	  SPI_WAITFORTx_BEFORE();                           \
    MRF49XA_SPI_ENABLE();                               \
    SPI_WRITE(MRF49XA_STSREG);                          \
    *data = ((uint16_t)SPI_RXBUF << 8);                  \
    SPI_WRITE(0);                                       \
    *data |= (uint16_t)SPI_RXBUF;                        \
    MRF49XA_SPI_DISABLE();                              \
  } while(0)

/* Read a register in the MRF49XA */
#define MRF49XA_READ_RXFIFOREG(data)                       \
  do {                                                  \
	  SPI_WAITFORTx_BEFORE();                           \
    MRF49XA_SPI_ENABLE();                               \
    SPI_WRITE(0);                                       \
    *data |= (uint16_t)SPI_RXBUF;                        \
    MRF49XA_SPI_DISABLE();                              \
  } while(0)

/* Read a register in the MRF49XA
//#define MRF49XA_READ_RXFIFOREG(data)                       \
//  do {                                                  \
//	  SPI_WAITFORTx_BEFORE();                           \
//    MRF49XA_SPI_ENABLE();                               \
//    SPI_WRITE(MRF49XA_RXFIFOREG);                          \
//    *data = ((uint16_t)SPI_RXBUF << 8);                  \
//    SPI_WRITE(0);                                       \
//    *data |= (uint16_t)SPI_RXBUF;                        \
//    MRF49XA_SPI_DISABLE();                              \
//  } while(0)
//PRINTF("0x%x\n",(uint16_t)*data); */

/*
//#define getIRQstatus(irq) \
//	do{
//	irq =   P2IN; 	    \
//	irq = (irq & 1<<MRF49XA_IRQ_PIN) ? 1 : 0; \
//} while(0)*/

#endif /* MRF49XA_H */
