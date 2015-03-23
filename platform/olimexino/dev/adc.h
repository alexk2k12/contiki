/*
 * Copyright (c) 2010, Andrea Cannavicci + Stefano Pagnottelli
 * CBL Electronics srl + Siralab srl
 *
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
 *
 *
 */
#ifndef __ADC_H__
#define __ADC_H__

#define DIRTY               0x10
#define HIGH_PRIORITY_LOCK  0x01
#define LOW_PRIORITY_LOCK   0x02

uint8_t getAdcStatus();

#define MSP430_ADC_CH_BIT 0
typedef enum _MSP430_ADC_CH{
	ADC_CH0 = 0 << MSP430_ADC_CH_BIT,
	ADC_CH1 = 1 << MSP430_ADC_CH_BIT,
	ADC_CH2 = 2 << MSP430_ADC_CH_BIT,
	ADC_CH3 = 3 << MSP430_ADC_CH_BIT,
	ADC_CH4 = 4 << MSP430_ADC_CH_BIT,
	ADC_CH5 = 5 << MSP430_ADC_CH_BIT,
	ADC_CH6 = 6 << MSP430_ADC_CH_BIT,
	ADC_CH7 = 7 << MSP430_ADC_CH_BIT,
	ADC_CH8 = 8 << MSP430_ADC_CH_BIT,
	ADC_CH9 = 9 << MSP430_ADC_CH_BIT,
	ADC_CH10 = 10 << MSP430_ADC_CH_BIT,
	ADC_CH11 = 11 << MSP430_ADC_CH_BIT,
	ADC_CH12 = 12 << MSP430_ADC_CH_BIT,
	ADC_CH13 = 13 << MSP430_ADC_CH_BIT,
	ADC_CH14 = 14 << MSP430_ADC_CH_BIT,
	ADC_CH15 = 15 << MSP430_ADC_CH_BIT,
} ADC_CH;


void adc_init();
uint8_t  start_adc(ADC_CH channel); // start  adc a bassa priorità
uint8_t checkAdcBusy();     // verifica dello stato dell'ADC
uint16_t getAdcSample();     // lettura del dato campionato

uint16_t get_adc(ADC_CH channel); //Misure singola dell'adc ad alta priorità
void     adcOff();

#endif /* __ADC_H__ */
