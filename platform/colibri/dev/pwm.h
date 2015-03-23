/*
Copyright 2013, CBL Electronics srl + Siralab srl. All rights reserved.

These sources were developed from CBL Electronics srl + Siralab srl,
Mote automation system development

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of CBL Electronics srl + Siralab srl nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the Andrea Cannavicci and Stefano
Pagnottelli at the CBL Electronics srl + Siralab srl.

For documentation and questions please use the web site
http://www.cblelectronics.com.
Todi, 2013
*/

/**
 * @file		pwm.h
 * @addtogroup  peripherial
 * @brief		MSP430 pwm generation
 *
 * @author		Andrea Cannavicci <andrea.cannavicci@cblelectronics.com>
 */
#ifndef PWM_H
#define PWM_H

#define PWM_DEF_PERIOD 64


#define PWM_CHANNELS 3
#define PWM_OFF 0
#define PWM_CH0 1
#define PWM_CH1 2
#define PWM_CH2 4

#define PWM_CH0_PORT(type) P1##type
#define PWM_CH0_BIT        BIT2

#define PWM_CH1_PORT(type) P1##type
#define PWM_CH1_BIT        BIT3

#define PWM_CH2_PORT(type) P1##type
#define PWM_CH2_BIT        BIT4

#define PWM_ON  1
#define PWM_OFF 0

uint8_t isPwmInit(void);
void pwm_init(uint8_t status);
void pwm_disablePort(uint8_t ports);
void pwm_enablePort(uint8_t ports);
void pwm_set(uint16_t pwmPeriod, uint16_t pwmDuty, uint8_t port);
void pwm_setPort(uint8_t port);

#endif // !PWM_H
