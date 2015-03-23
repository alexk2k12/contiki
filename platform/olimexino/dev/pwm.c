/*
 * Copyright (c) 2012, Andrea Cannavicci + Stefano Pagnottelli
 * CBL Electronics srl + Siralab srl
 * */
#include "contiki.h"
#include "pwm.h"

uint8_t init = 0;
uint8_t isPwmInit(void){
	return init;
}

void pwm_disablePort(uint8_t port){
	if (port & PWM_CH0 ){
		P6DIR  &= ~ BIT0;
		PWM_CH0_PORT(DIR) &= ~PWM_CH0_BIT;
		PWM_CH0_PORT(SEL) &= ~PWM_CH0_BIT;
	}
	if (port & PWM_CH1 ){
		P6DIR  &= ~ BIT1;
		PWM_CH1_PORT(DIR) &= ~PWM_CH1_BIT;
		PWM_CH1_PORT(SEL) &= ~PWM_CH1_BIT;
	}
	if (port & PWM_CH2 ){
	    P6DIR  &= ~ BIT2;
		PWM_CH2_PORT(DIR) &= ~PWM_CH2_BIT;
		PWM_CH2_PORT(SEL) &= ~PWM_CH2_BIT;
	}
}


void pwm_enablePort(uint8_t port){
	if (port & PWM_CH0 ){
		PWM_CH0_PORT(DIR) |= PWM_CH0_BIT;
		PWM_CH0_PORT(SEL) |= PWM_CH0_BIT;
	}
	if (port & PWM_CH1 ){
		PWM_CH1_PORT(DIR) |= PWM_CH1_BIT;
		PWM_CH1_PORT(SEL) |= PWM_CH1_BIT;
	}
	if (port & PWM_CH2 ){
		PWM_CH2_PORT(DIR) |= PWM_CH2_BIT;
		PWM_CH2_PORT(SEL) |= PWM_CH2_BIT;
	}

}


void pwm_init(uint8_t status){

	if(status){
	  init = 1;
	  //moved to ioInit
	  //P1SEL |= BIT4; // | BIT2 | BIT3;  // P1.2 TA0.1
	  //P1DIR |= BIT4;// | BIT2 | BIT3;  // P1.2 TA0.1

	  TA0CCR0 = PWM_DEF_PERIOD;                            // PWM Period/2

	  TA0CCTL1 = OUTMOD_7;                      // CCR1 toggle/set
	  TA0CCR1 = PWM_DEF_PERIOD/2;               // CCR1 PWM duty cycle

	  TA0CCTL2 = OUTMOD_7;                      // CCR2 toggle/set
	  TA0CCR2 = PWM_DEF_PERIOD/2;               // CCR2 PWM duty cycle

	  TA0CCTL3 = OUTMOD_7;                      // CCR3 toggle/set
	  TA0CCR3 = PWM_DEF_PERIOD/2;               // CCR3 PWM duty cycle


	  TA0CTL = TASSEL_2 | MC_1 | TACLR;         // SMCLK, up-down mode, clear TAR
	}
	else
	{
		init = 0;
		TA0CTL = 0;

	}

}

void pwm_set(uint16_t pwmPeriod, uint16_t pwmDuty, uint8_t port){
	  TA0CCR0 = pwmPeriod;                       // PWM Period
		if (port & PWM_CH0 )
			TA0CCR1 = pwmDuty;                      // CCR1 PWM duty cycle
		if (port & PWM_CH1)
			TA0CCR2 = pwmDuty;                      // CCR3 PWM duty cycle
		if (port & PWM_CH2)
			TA0CCR3 = pwmDuty;                      // CCR4 PWM duty cycle

}
