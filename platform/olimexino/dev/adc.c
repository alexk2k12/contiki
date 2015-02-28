/*
 * Copyright (c) 2012, Andrea Cannavicci + Stefano Pagnottelli
 * CBL Electronics srl + Siralab srl
 * */
#include "contiki.h"
#include "adc.h"
#include "utils.h"

static uint8_t adcStatus;

uint8_t getAdcStatus(){
	return adcStatus;
}

void adc_init(void)
{
#ifdef __MSP430_HAS_ADC10_A__
	  //WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

	  // ADCCLK è il clock interno dell'ADC pari a circa 4.8MHZ (200ns)
	  // Configure ADC10 - Pulse sample mode; ADC10SC trigger
	  ADC10CTL0 = ADC10SHT_2 | ADC10ON;         // 16 ADC10CLKs; ADC ON
	  ADC10CTL1 = ADC10SHP | ADC10CONSEQ_0;     // s/w trig, single ch/conv
	  ADC10CTL2 = ADC10RES;                     // 10-bit conversion results
	  ADC10MCTL0 = ADC10SREF_1 | ADC10INCH_10;  // AVcc/2
#endif

#ifdef __MSP430_HAS_ADC12_PLUS__
	  // ADCCLK è il clock interno dell'ADC pari a circa 4.8MHZ (200ns)
	  // Configure ADC10 - Pulse sample mode; ADC10SC trigger
	  ADC12CTL0 = ADC12SHT01 | ADC12ON;         // 16 ADC10CLKs; ADC ON
	  ADC12CTL1 = ADC12SHP | ADC12CONSEQ_0;     // s/w trig, single ch/conv
	  ADC12CTL2 = ADC12RES_1;                   // 10-bit conversion results (for compatibility with ADC10)
	  ADC12MCTL0 = ADC12SREF_1 | ADC12INCH_10;  // AVcc/2
#endif


	  // Configure internal reference
	  while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
	  REFCTL0 |= REFVSEL_2|REFON;               // Select internal ref = 2.5V
	                                            // Internal Reference ON
	  clock_delay(600);                         // Delay (~75us) for Ref to settle
	  adcStatus = 0;
}

// start  adc a bassa priorità
uint8_t start_adc(ADC_CH channel){
	if (adcStatus & HIGH_PRIORITY_LOCK)
		return 0;       //l'adc è impegnato in una conversione ad alta priorità quindi esco senza fare nulla
#ifdef __MSP430_HAS_ADC10_A__
	if ((ADC10MCTL0 &  0xf ) != channel){
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 &=  0xfff0;
		ADC10MCTL0 |= channel;  // imposto il canale
	    clock_delay(100);                         // Delay (~3us) for channel to settle
	}

	ADC10CTL0 |= ADC10ENC | ADC10SC;        // Sampling and conversion start
#endif

#ifdef __MSP430_HAS_ADC12_PLUS__
	if ((ADC12MCTL0 &  0xf ) != channel){
		ADC12CTL0 &= ~ADC12ENC;
		ADC12MCTL0 &=  0xfff0;
		ADC12MCTL0 |= channel;  // imposto il canale
	    clock_delay(100);                         // Delay (~3us) for channel to settle
	}

	ADC12CTL0 |= (ADC12ENC | ADC12SC);        // Sampling and conversion start
#endif
	adcStatus = LOW_PRIORITY_LOCK;          // indico che è in corso una conversione a bassa priorita
	return 1;

}
// verifica dello stato dell'ADC
// ritorna vero se l'ADC è busy
uint8_t checkAdcBusy(){
#ifdef __MSP430_HAS_ADC10_A__
    return (ADC10CTL1 & ADC10BUSY);
#endif
#ifdef __MSP430_HAS_ADC12_PLUS__
    return (ADC12CTL1 & ADC12BUSY);
#endif
}

// lettura del dato campionato
uint16_t getAdcSample(){
	adcStatus = 0;
#ifdef __MSP430_HAS_ADC10_A__
	return ADC10MEM0;
#endif
#ifdef __MSP430_HAS_ADC12_PLUS__
	return ADC12MEM0;
#endif
}

//Misure singola dell'adc ad alta priorità
uint16_t get_adc(ADC_CH channel){
	adcStatus = DIRTY | HIGH_PRIORITY_LOCK;
#ifdef __MSP430_HAS_ADC10_A__
    while (ADC10CTL1 & ADC10BUSY);          // ADC10BUSY? attendo la fine di una precedente conversione avviata

	if ((ADC10MCTL0 &  0xf ) != channel){
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 &=  0xfff0;
		ADC10MCTL0 |= channel;  // imposto il canale
	    clock_delay(300);       // Delay (~3us) for channel to settle
	}
    ADC10CTL0 |= (ADC10ENC | ADC10SC);        // Sampling and conversion start
    while (ADC10CTL1 & ADC10BUSY);          // ADC10BUSY?
    adcStatus = DIRTY;
    return ADC10MEM0;
#endif

#ifdef __MSP430_HAS_ADC12_PLUS__
    while (ADC12CTL1 & ADC12BUSY);          // ADC10BUSY? attendo la fine di una precedente conversione avviata

	if ((ADC12MCTL0 &  0xf ) != channel){
		ADC12CTL0 &= ~ADC12ENC;
		ADC12MCTL0 &=  0xfff0;
		ADC12MCTL0 |= channel;  // imposto il canale
	    clock_delay(300);       // Delay (~3us) for channel to settle
	}
    ADC12CTL0 |= (ADC12ENC | ADC12SC);        // Sampling and conversion start
    while (ADC12CTL1 & ADC12BUSY);          // ADC10BUSY?
    adcStatus = DIRTY;
    return ADC12MEM0;
#endif
}

void adcOff(){
#ifdef __MSP430_HAS_ADC10_A__
	  // ADCCLK è il clock interno dell'ADC pari a circa 4.8MHZ (200ns)
	  // Configure ADC10 - Pulse sample mode; ADC10SC trigger
	  ADC10CTL0 &= ~ADC10ON;                    // ADC OFF
	  //ADC10MCTL0 = ADC10SREF_1 | ADC10INCH_10;  // AVcc/2
#endif
#ifdef __MSP430_HAS_ADC12_PLUS__
	  // ADCCLK è il clock interno dell'ADC pari a circa 4.8MHZ (200ns)
	  // Configure ADC12 - Pulse sample mode; ADC10SC trigger
	  ADC12CTL0 &= ~ADC12ON;                    // ADC OFF
	  //ADC10MCTL0 = ADC10SREF_1 | ADC10INCH_10;  // AVcc/2
#endif
	  // Configure internal reference
	  while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
	  REFCTL0 &= ~REFON;                       // Select internal ref = 2.5V
												// Internal Reference ON
}

/*void adcOn(){
	  // ADCCLK è il clock interno dell'ADC pari a circa 4.8MHZ (200ns)
	  // Configure ADC10 - Pulse sample mode; ADC10SC trigger
	  ADC10CTL0 |= ADC10ON;         // 16 ADC10CLKs; ADC ON

	  // Configure internal reference
	  while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
	  REFCTL0 |= REFON;                         // Internal Reference ON
	  clock_delay(600);                         // Delay (~75us) for Ref to settle
}*/
