/*
 * Can.h
 *
 *  Created on: 2022Äê9ÔÂ15ÈÕ
 *      Author: songz
 */

#ifndef CAN_H_
#define CAN_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Ifx_Types.h"
//#include "IfxCan_Can.h"
//#include "IfxCan.h"
//#include "IfxCpu_Irq.h"
//#include "IfxPort.h"                                        /* For GPIO Port Pin Control                            */

void CanApp_init(void);
void CAN_SendSingle(uint32 id, uint32 high, uint32 low);
#endif /* CAN_H_ */
