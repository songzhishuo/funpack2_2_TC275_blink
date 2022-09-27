/*
 * Can.c
 *
 *  Created on: 2022年9月15日
 *      Author: songz
 */

#include "Can.h"

#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"

#include "SysSe/Bsp/Bsp.h"
#include "Port/Io/IfxPort_Io.h"
#include "Multican/Can/IfxMultican_Can.h"   //包含CAN控制API



//定义CAN模块、节点、MessageObject：
// CAN handle
IfxMultican_Can can;
// Nodes handles
IfxMultican_Can_Node canSrcNode;
// Message Object handles
IfxMultican_Can_MsgObj canSrcMsgObj;

//定义中断等级和中断函数，报文发送完成后进入中断，翻转LED电平
#define ISR_PRIORITY_CAN_TX         2                           /* Define the CAN TX interrupt priority              */
#define TX_INTERRUPT_SRC_ID         IfxMultican_SrcId_0         /* Transmit interrupt service request ID             */
IFX_INTERRUPT(canIsrTxHandler, 0, ISR_PRIORITY_CAN_TX);
void canIsrTxHandler(void)
{
    IfxPort_togglePin(&MODULE_P14, 9);
}
//CAN初始化函数：
void CanApp_init(void)
{
    // create configuration
    IfxMultican_Can_Config canConfig;
    IfxMultican_Can_initModuleConfig(&canConfig, &MODULE_CAN);
    // initialize interrupt priority
    canConfig.nodePointer[TX_INTERRUPT_SRC_ID].priority = ISR_PRIORITY_CAN_TX;
    // initialize module
    IfxMultican_Can_initModule(&can, &canConfig);

    // create CAN node config
    IfxMultican_Can_NodeConfig canNodeConfig;
    IfxMultican_Can_Node_initConfig(&canNodeConfig, &can);
    canNodeConfig.baudrate = 500000; // 1 MBaud
    canNodeConfig.nodeId = IfxMultican_NodeId_0;
    canNodeConfig.rxPin = &IfxMultican_RXD0A_P02_1_IN;
    canNodeConfig.txPin = &IfxMultican_TXD0_P02_0_OUT;
    IfxMultican_Can_Node_init(&canSrcNode, &canNodeConfig);

    // create message object config
    IfxMultican_Can_MsgObjConfig canMsgObjConfig;
    IfxMultican_Can_MsgObj_initConfig(&canMsgObjConfig, &canSrcNode);
    // assigned message object:
    canMsgObjConfig.msgObjId = 0;
    canMsgObjConfig.frame = IfxMultican_Frame_transmit;
    canMsgObjConfig.control.messageLen = IfxMultican_DataLengthCode_8;
    canMsgObjConfig.control.extendedFrame = FALSE;
    canMsgObjConfig.txInterrupt.enabled = TRUE;
    canMsgObjConfig.txInterrupt.srcId = TX_INTERRUPT_SRC_ID;

    // initialize receive message object
    IfxMultican_Can_MsgObj_init(&canSrcMsgObj, &canMsgObjConfig);
}
//CAN发送函数
void CAN_SendSingle(uint32 id, uint32 high, uint32 low)
{
    // Initialise the message strcture
    IfxMultican_Message txMsg;
    IfxMultican_Message_init(&txMsg, id, low, high, IfxMultican_DataLengthCode_8);

    // Transmit Data
    while( IfxMultican_Can_MsgObj_sendMessage(&canSrcMsgObj, &txMsg) == IfxMultican_Status_notSentBusy );

}

