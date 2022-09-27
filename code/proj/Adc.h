/*
 * Adc.h
 *
 *  Created on: 2022Äê9ÔÂ14ÈÕ
 *      Author: songz
 */

#ifndef ADC_H_
#define ADC_H_

#include "IfxVadc_Adc.h"
#include "stdint.h"
typedef struct
{
    IfxVadc_Adc               vadc;               /* VADC configuration     */
    IfxVadc_Adc_Group         adcGroup;           /* Group configuration    */
    IfxVadc_Adc_ChannelConfig adcChannelConfig;   /* Channel configuration  */
    IfxVadc_Adc_Channel       adcChannel;         /* Channel                */
} ApplicationVadcBackgroundScan;

extern void adc_init();
extern uint32  adc_get_val();

#endif /* ADC_H_ */
