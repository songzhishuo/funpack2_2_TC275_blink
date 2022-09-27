/*
 * Pwm.h
 *
 *  Created on: 2022Äê9ÔÂ14ÈÕ
 *      Author: songz
 */

#ifndef PWM_H_
#define PWM_H_
#include "Ifx_Types.h"

#define LED1        &MODULE_P00,5        /* LED1 Port, Pin Definition */
#define MAX_DUTY    100
#define MAX_PERIOD  500000
/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void init_Iom(void);
void init_Gtm_Pwm(void);
void changeDutyCycle(float32 duty);
void init_Smu(void);
void init_IR(void);
void pwm_init();
void changePeriod(uint32 period);
#endif /* PWM_H_ */
