/*
 * Adc.c
 *
 *  Created on: 2022Äê9ÔÂ14ÈÕ
 *      Author: songz
 */
#include "IfxVadc_Adc.h"
#include "Adc.h"
#include "Ifx_Types.h"

#define VADC_GROUP                  IfxVadc_GroupId_0           /* Use the ADC group 0                              */
#define CHANNEL_ID                  0                           /* Use the Channel 0                                */
#define CHANNEL_RESULT_REGISTER     5                           /* Use the Result Register 5                        */

ApplicationVadcBackgroundScan g_vadcBackgroundScan;

void adc_init()
{
    /* VADC module configuration */
    /* Create VADC configuration */
    IfxVadc_Adc_Config adcConfig;

    /* Initialize the VADC configuration with default values */
    IfxVadc_Adc_initModuleConfig(&adcConfig, &MODULE_VADC);

    /* Initialize the VADC module using the VADC configuration */
    IfxVadc_Adc_initModule(&g_vadcBackgroundScan.vadc, &adcConfig);

    /* VADC group configuration */
    /* Create group configuration */
    IfxVadc_Adc_GroupConfig adcGroupConfig;

    /* Initialize the group configuration with default values */
    IfxVadc_Adc_initGroupConfig(&adcGroupConfig, &g_vadcBackgroundScan.vadc);

    /* Define which ADC group is going to be used */
    adcGroupConfig.groupId = VADC_GROUP;
    adcGroupConfig.master = VADC_GROUP;

    /* Enable background scan source */
    adcGroupConfig.arbiter.requestSlotBackgroundScanEnabled = TRUE;

    /* Enable background auto scan mode */
    adcGroupConfig.backgroundScanRequest.autoBackgroundScanEnabled = TRUE;

    /* Enable the gate in "always" mode (no edge detection) */
    adcGroupConfig.backgroundScanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

    /* Initialize the group using the group configuration */
    IfxVadc_Adc_initGroup(&g_vadcBackgroundScan.adcGroup, &adcGroupConfig);



    /* Initialize the channel configuration of application handle g_vadcBackgroundScan with default values */
    IfxVadc_Adc_initChannelConfig(&g_vadcBackgroundScan.adcChannelConfig, &g_vadcBackgroundScan.adcGroup);

    g_vadcBackgroundScan.adcChannelConfig.channelId = (IfxVadc_ChannelId)CHANNEL_ID;
    g_vadcBackgroundScan.adcChannelConfig.resultRegister = (IfxVadc_ChannelResult)CHANNEL_RESULT_REGISTER;
    g_vadcBackgroundScan.adcChannelConfig.backgroundChannel = TRUE;

    /* Initialize the channel of application handle g_VadcBackgroundScan using the channel configuration */
    IfxVadc_Adc_initChannel(&g_vadcBackgroundScan.adcChannel, &g_vadcBackgroundScan.adcChannelConfig);

    /* Enable background scan for the channel */
    IfxVadc_Adc_setBackgroundScan(&g_vadcBackgroundScan.vadc,
                                  &g_vadcBackgroundScan.adcGroup,
                                  (1 << (IfxVadc_ChannelId)CHANNEL_ID),
                                  (1 << (IfxVadc_ChannelId)CHANNEL_ID));

    /* Start background scan conversion */
    IfxVadc_Adc_startBackgroundScan(&g_vadcBackgroundScan.vadc);
}

#define RETRY_MAX 0xf000

uint32  adc_get_val()
{
    Ifx_VADC_RES conversionResult;
    uint16 retry = 0;
    do {
        conversionResult = IfxVadc_Adc_getResult(&g_vadcBackgroundScan.adcChannel);
        if (retry++ > RETRY_MAX) {
            return 0xffffffff;
        }
    } while (!conversionResult.B.VF);

    return conversionResult.B.RESULT;
}
