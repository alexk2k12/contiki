#ifndef AT25F512B_H
#define AT25F512B_H

#include "contiki.h"
#include "dev/spi.h"

#define RDY_BUSY 0x01
#define CRC_BLOCK 32

typedef enum _at25f512b_instr {
	at25f512b_ReadPage  = 0x03,
	at25f512b_Erase4k   = 0x20,
	at25f512b_Erase32k  = 0xd8,
	at25f512b_EraseBulk = 0x60,
	at25f512b_WritePage = 0x02,
	at25f512b_Wren      = 0x06,
	at25f512b_Wrdi      = 0x04,
	at25f512b_WrOtp     = 0x9b,
	at25f512b_ReadOtp   = 0x77,
	at25f512b_Rdsr      = 0x05,
	at25f512b_Wrsr      = 0x01,
	at25f512b_RdID      = 0x15,
	at25f512b_DP        = 0xb9,
	at25f512b_ResumeDP  = 0xAB,
	at25f512b_getCrc    = 0x00

} at25f512b_instr ;

typedef enum _at25f512b_dpMode{
	at25f512b_DPon        = 0x01,
	at25f512b_DPoff       = 0x00,
} at25f512b_dpMode;


//***********************************************************************/
// Additional SPI Macros for the MRF49XA */
//************************************************************************/
// Write to a register in the MRF49XA

//write a single byte to spi asserting and deasserting CS
#define AT25F512_SPI_BYTE_CMD(data) \
  do {  \
	SPI_WAITFORTx_BEFORE(); \
    AT25F512B_SPI_ENABLE(); \
    SPI_WRITE(data); \
    AT25F512B_SPI_DISABLE(); \
  } while(0)

//start spi write asserting CS and leave asserted
#define AT25F512_SPI_START(data) \
  do {  \
	SPI_WAITFORTx_BEFORE(); \
    AT25F512B_SPI_ENABLE(); \
    SPI_WRITE(data); \
  } while(0)

//byte to spi leaving cs asserted
#define AT25F512_SPI_BYTE_WRITE(data) \
  do { \
	SPI_WAITFORTx_BEFORE(); \
    SPI_WRITE(data); \
  } while(0)

/* Read a rbyte from spi*/
#define ECHO_SDI 0
#if ECHO_SDI
/* Read a rbyte from spi*/
#define AT25F512_SPI_BYTE_READ(data) \
  do { \
	SPI_WAITFORTx_BEFORE(); \
    SPI_WRITE(*data); \
    *data = (SPI_RXBUF); \
  } while(0)
#else
#define AT25F512_SPI_BYTE_READ(data) \
  do { \
	SPI_WAITFORTx_BEFORE(); \
    SPI_WRITE(0); \
    *data = (SPI_RXBUF); \
  } while(0)
#endif



//stop spi after byte write deasserting CS
#define AT25F512_SPI_STOP(data) \
  do { \
	SPI_WAITFORTx_BEFORE(); \
    SPI_WRITE(data); \
    AT25F512B_SPI_DISABLE(); \
  } while(0)

#define AT25F512_WT_BUSY(busy) \
do{ \
	busy = at25f512_Rdsr() & RDY_BUSY; \
}while(busy) \

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

void at52f512_read(uint32_t address, uint8_t *buff, uint8_t len);
void at52f512_write(uint32_t address, uint8_t *buff, uint8_t len);
void at25f512_erase(at25f512b_instr blockSize, uint32_t address);
void at25f512_rdOTP(uint32_t address, uint8_t *buff, uint8_t len);
void at25f512_wrOTP(uint32_t address, uint8_t *buff, uint8_t len);
uint8_t at25f512_Rdsr();
uint16_t at25f512_RdID();
void at25f512_DP(at25f512b_dpMode mode);
uint16_t at25f512b_crc(uint32_t address, uint32_t len);

#endif /* AT25F512B_H */
