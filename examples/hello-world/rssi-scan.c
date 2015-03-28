#include "contiki.h"
#include "clock.h"
#include "rime.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>
#include "rf230bb.h"
#include "watchdog.h"
#include "hal.h"

#define BUFFER_SIZE 10000

signed char rssi[BUFFER_SIZE/2];
unsigned char count[BUFFER_SIZE/2];
unsigned long curr = 0;

#include <avr/io.h>
#include <avr/interrupt.h>

#define HAL_SPI_TRANSFER_OPEN() { \
  HAL_ENTER_CRITICAL_REGION();	  \
  HAL_SS_LOW(); /* Start the SPI transaction by pulling the Slave Select low. */
#define HAL_SPI_TRANSFER_WRITE(to_write) (SPDR = (to_write))
#define HAL_SPI_TRANSFER_WAIT() ({while ((SPSR & (1 << SPIF)) == 0) {;}}) /* gcc extension, alternative inline function */
#define HAL_SPI_TRANSFER_READ() (SPDR)
#define HAL_SPI_TRANSFER_CLOSE() \
    HAL_SS_HIGH(); /* End the transaction by pulling the Slave Select High. */ \
    HAL_LEAVE_CRITICAL_REGION(); \
    }

#define HAL_SPI_TRANSFER(to_write) ( \
 HAL_SPI_TRANSFER_WRITE(to_write),\
 HAL_SPI_TRANSFER_WAIT(),\
 HAL_SPI_TRANSFER_READ())

#define R_REG (0x80 | RG_PHY_RSSI)
#define QUICKGET_RSSI(val) do{ \
  HAL_SS_LOW();\
  HAL_SPI_TRANSFER(R_REG);\
  val=(0x1F & HAL_SPI_TRANSFER(0));\
  HAL_SS_HIGH();\
}while(0)

#define GPIO_INIT() (DDRB |= 0x40)
#define GPIO_TOGGLE() do{\
  if(PORTB & 0x40) PORTB &= ~0x40;\
  else PORTB |= 0x40;\
}while(0)

/* Configures how many readings to average against. In the Tmote sky this is 8 symbols. At 64Khz, this should be 4 symbols to match it here */
#define RSSI_AVG 4
#define BASE -91
#if RSSI_AVG > 0
#if ((RSSI_AVG) & (RSSI_AVG-1))!=0
#error RSSI_AVG must be a power of 2, e.g. 2,4,8,16
#endif

signed char rssi_averages[RSSI_AVG];
int running_total=0;
int av_index = 0;
#define UPDATE_AVERAGE(value) do {\
  running_total-=rssi_averages[av_index];\
  running_total+=value;\
  rssi_averages[av_index++]=value;\
  if(av_index== RSSI_AVG)av_index=0;\
} while(0);
//  rssi_averages[av_index++]=value;
//  av_index = av_index%RSSI_AVG;
#define GET_AVERAGE() ((running_total/RSSI_AVG)+BASE)
#define CALC_RSSI(in) ((in-1)*3)
#endif


int add(signed char val) {
  if(curr == (BUFFER_SIZE/2)) {
    return 0;
  }
  if(val == rssi[curr] && count[curr] < 255) {
    count[curr]++;
  }
  else {
    curr++;
    rssi[curr] = val;
    count[curr]++;
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
PROCESS(rssi_process, "RSSI scanner");
AUTOSTART_PROCESSES(&rssi_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rssi_process, ev, data)
{
  PROCESS_BEGIN();
  
  printf("Starting scanner...\n");
  curr = 0;
  memset(rssi, 0, BUFFER_SIZE/2);
  memset(count, 0, BUFFER_SIZE/2);

  signed char rssi_p;
  int rssi_c,j;
  signed char last_rssi;
  GPIO_INIT();

  /* Init */
  for(j=0;j<RSSI_AVG;j++) {
    QUICKGET_RSSI(rssi_p);
    rssi_p=CALC_RSSI(rssi_p);
    rssi_averages[j]=rssi_p;
    running_total+=rssi_p;
    printf("%d. %d\n", j, rssi_averages[j]);
  }
  
  QUICKGET_RSSI(last_rssi); 
  cli();
  while(curr != (BUFFER_SIZE/2)) {
  //  HAL_SS_LOW();
  //  /* Send SPi command to read RSSI register */
  //  HAL_SPI_TRANSFER_WRITE(R_REG);

  //  /* Add the previous rssi sample to the rssi_averages array */
  //  UPDATE_AVERAGE(CALC_RSSI(last_rssi));

  //  /* Wait for the SPI to finish*/
  //  HAL_SPI_TRANSFER_WAIT();

  //  /* Start reading in the value sent back... */
  //  HAL_SPI_TRANSFER_WRITE(0x00);
 
  //  /* Calculate the updated average and add it to the big results */
  //  add(GET_AVERAGE());

  //  /* Wait for the latest... */
  //  HAL_SPI_TRANSFER_WAIT();
  //  last_rssi = (0x1F & HAL_SPI_TRANSFER_READ());

  //  HAL_SS_HIGH();

    QUICKGET_RSSI(rssi_p);
    UPDATE_AVERAGE(CALC_RSSI(rssi_p));
    add(GET_AVERAGE());
    GPIO_TOGGLE();
  }

  sei();
  
  printf("done..\n");
  int i;
  for(i=0;i< BUFFER_SIZE/2;i++) {
    int cnt=count[i];
    while(cnt--) {
      printf("%d\n", rssi[i]);
      clock_delay_usec(6);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
