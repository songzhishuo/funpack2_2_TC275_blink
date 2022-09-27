/*
 * Pwm.c
 *
 *  Created on: 2022��9��14��
 *      Author: songz
 */


#include "Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxIom_Driver.h"
#include "IfxIom.h"
#include "IfxGtm_Atom_Pwm.h"
#include "IfxGtm_regdef.h"
#include "IfxScuWdt.h"
#include "IfxSmu.h"
#include "Ifx_Assert.h"
#include "Ifx_reg.h"


#define P00_0               IfxGtm_ATOM0_5_TOUT15_P00_6_OUT//IfxGtm_ATOM0_0_TOUT9_P00_0_OUT  /* LED identifying the SMU alarm triggered on IOM event */
#define PWM_PERIOD          50000                            /* PWM period for the ATOM, in us                       */
#define CLK_FREQ            1000000.0f                      /* CMU clock frequency, in Hertz                        */
#define SMU_INTERRUPT       10                              /* SMU Interrupt Priority Level                         */
#define DUTY_THRESHOLD      0.00185                         /* IOM duty cycle threshold of 37% = 1,85ms             */


IfxGtm_Atom_Pwm_Config g_atomConfig;                        /* Timer configuration structure                        */
IfxGtm_Atom_Pwm_Driver g_atomDriver;                        /* Timer Driver structure                               */


IFX_INTERRUPT(smu_handler, 0, SMU_INTERRUPT);

void smu_handler(void)
{
    IfxPort_setPinLow(LED1); /* Switch ON the LED1 */

    /* Clear the SMU flag related to the IOM */
    IfxSmu_enableClearAlarmStatus(&MODULE_SMU);
    IfxSmu_clearAlarm(&MODULE_SMU, IfxSmu_Alarm_IomPinMismatchIndication);
}

void init_Smu(void)
{
    IfxSmu_start(&MODULE_SMU);  /* Start the SMU module */
    IfxSmu_enableClearAlarmStatus(&MODULE_SMU);

    IfxSmu_unlock(&MODULE_SMU); /* Unlock the SMU module to allow its configuration */

    IfxScuWdt_clearSafetyEndinitInline(IfxScuWdt_getSafetyWatchdogPasswordInline()); /* To allow bit configuration  */

    /* Enable the 'Interrupt Generation Configuration Set 0' of the SMU */
    MODULE_SMU.AGC.U = 0x1;                                 /* This register set will take care of the interrupt    */
    IfxScuWdt_setSafetyEndinitInline(IfxScuWdt_getSafetyWatchdogPasswordInline());  /* To disable bit configuration */

    /* Selection of the 'Interrupt Generation Configuration Set 0' of the SMU */
    IfxSmu_setAlarmConfig(&MODULE_SMU, IfxSmu_Alarm_IomPinMismatchIndication, IfxSmu_AlarmConfig_interruptSet0);

    IfxSmu_lock(&MODULE_SMU);                               /* Re-lock the SMU after configuration                  */
}

void init_IR(void)
{
    volatile Ifx_SRC_SRCR *src;
    src = &SRC_SMU0; /* Fetch the proper Service Request Register, here the one related to the SMU */
    IfxSrc_init(src, IfxSrc_Tos_cpu0, SMU_INTERRUPT);       /* Configuration of the interrupt level and its ToS     */
    IfxSrc_enable(src);
}


void init_Iom(void)
{
    /* Initialization of the LED used in this example */
    IfxPort_setPinModeOutput(LED1, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinHigh(LED1); /* Switched OFF (active low) */

    /* Initialization of the port pin used in this example */
    IfxPort_setPinMode(&MODULE_P00, 0, IfxPort_Mode_outputPushPullGeneral);     /* PWM output                       */
    IfxPort_setPinMode(&MODULE_P33, 1, IfxPort_Mode_inputPullDown);             /* IOM monitor input                */

    /* Disable interrupts */
    boolean interruptState = IfxCpu_disableInterrupts();

    IfxIom_Driver driver;
    IfxIom_Driver_Lam lam;
    IfxIom_Driver_Config configDriver;
    IfxIom_Driver_LamConfig configLam;

    IfxIom_enableModule(&MODULE_IOM, 1);                            /* Initialization of the IOM module             */
    IfxIom_Driver_initConfig(&configDriver, &MODULE_IOM);           /* Creation of default configuration            */
    IfxIom_Driver_init(&driver, &configDriver);                     /* Applying of the configuration                */
    IfxIom_Driver_initLamConfig(&configLam, &driver);               /* Creation of a default LAM configuration      */

    configLam.channel = IfxIom_LamId_0; /* Selection of LAM #0 */

    /* Configuration of the Monitor signal */
    configLam.mon.input = IfxIom_MonInput_p33_1;                    /* IOM - Monitor signal input                   */
    configLam.mon.filter.mode = IfxIom_LamFilterMode_noFilter;      /* No filter applied on the signal              */
    configLam.mon.inverted = FALSE;                                 /* Input signal NOT inverted                    */

    /* Configuration of the Event Window */
    configLam.eventWindow.controlSource = IfxIom_LamEventWindowControlSource_mon;  /* Event window generation determined from the monitor signal */
    configLam.eventWindow.run = IfxIom_LamEventWindowRunControl_freeRunning;       /* Even generation window is not gated                        */
    configLam.eventWindow.clearEvent = IfxIom_LamEventWindowClearEvent_risingEdge; /* Window cleared on the rising edge                          */
    configLam.eventWindow.threshold = DUTY_THRESHOLD;                              /* Setting a threshold of 1,85ms (37% of PWM_PERIOD)          */
    configLam.eventWindow.inverted = TRUE;                                         /* Window INVERTED: events are BEFORE the threshold           */

    /* Configuration of the IOM event */
    configLam.event.source = IfxIom_LamEventSource_mon;             /* Monitor signal is routed directly            */
    configLam.event.trigger = IfxIom_LamEventTrigger_fallingEdge;   /* Falling edge used to gate event generation   */
    configLam.systemEventTriggerThreshold = 1;                      /* Alarm generated after 1 event                */

    IfxIom_Driver_initLam(&lam, &configLam);                        /* Apply the custom configuration               */
    lam.accumulatedCounterIndex = 1;                                /* Accumulated counter used                     */

    IfxIom_Driver_enableLamEvent(&lam);                             /* Enable LAM events                            */

    IfxCpu_restoreInterrupts(interruptState);
}
void init_Gtm_Pwm(void)
{
    /* Enable the GTM module */
    IfxGtm_enable(&MODULE_GTM);

    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, CLK_FREQ);    /* Set the CMU clock 0 frequency        */
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_CLK0);            /* Enable the CMU clock 0               */

    IfxGtm_Atom_Pwm_initConfig(&g_atomConfig, &MODULE_GTM);                 /* Initialize default parameters        */

    /* Configuration of the PWM signal used as Monitor signal by the IOM */
    g_atomConfig.atom = IfxGtm_Atom_0;                                      /* Select the ATOM 0                    */
    g_atomConfig.atomChannel = IfxGtm_Atom_Ch_5;                            /* Select the channel 0                 */
    g_atomConfig.period = PWM_PERIOD;                                       /* Set timer period                     */
    g_atomConfig.pin.outputPin = &P00_0;                                    /* Set P00_0 as output port pin         */
    g_atomConfig.synchronousUpdateEnabled = TRUE;                           /* Enable synchronous update            */
    g_atomConfig.dutyCycle = PWM_PERIOD / 2;                                /* Set duty cycle of 50%                */

    IfxGtm_Atom_Pwm_init(&g_atomDriver, &g_atomConfig);                     /* Initialize the PWM                   */
    IfxGtm_Atom_Pwm_start(&g_atomDriver, TRUE);                             /* Start the PWM                        */
}

void pwm_init()
{
    init_Smu();                         /* Initialization of SMU module                                             */
    init_IR();                          /* Initialization of the Interrupt Router module                            */
    init_Iom();                         /* Initialization of the IOM module                                         */
    init_Gtm_Pwm();                     /* Initialization of the GTM module and creation of the PWM signal          */
}

void changeDutyCycle(float32 duty)
{
    g_atomConfig.dutyCycle = PWM_PERIOD * (duty / 100);                     /* Set duty cycle with new value, in ms */
    IfxGtm_Atom_Pwm_init(&g_atomDriver, &g_atomConfig);                     /* Re-initialize the PWM                */
}

void changePeriod(uint32 period)
{
    IfxGtm_Atom_Pwm_initConfig(&g_atomConfig, &MODULE_GTM);                 /* Initialize default parameters        */

    /* Configuration of the PWM signal used as Monitor signal by the IOM */
    g_atomConfig.atom = IfxGtm_Atom_0;                                      /* Select the ATOM 0                    */
    g_atomConfig.atomChannel = IfxGtm_Atom_Ch_5;                            /* Select the channel 0                 */
    g_atomConfig.period = period;                                       /* Set timer period                     */
    g_atomConfig.pin.outputPin = &P00_0;                                    /* Set P00_0 as output port pin         */
    g_atomConfig.synchronousUpdateEnabled = TRUE;                           /* Enable synchronous update            */
    g_atomConfig.dutyCycle = period / 2;                                /* Set duty cycle of 50% period         */

    IfxGtm_Atom_Pwm_init(&g_atomDriver, &g_atomConfig);                     /* Initialize the PWM                   */
    IfxGtm_Atom_Pwm_start(&g_atomDriver, TRUE);                             /* Start the PWM                        */

}
