/*
 * Uart.h
 *
 *  Created on: 2022Äê9ÔÂ14ÈÕ
 *      Author: songz
 */

#ifndef UART_H_
#define UART_H_


#define UART_PIN_RX         IfxAsclin0_RXA_P14_1_IN                 /* UART receive port pin                        */
#define UART_PIN_TX         IfxAsclin0_TX_P14_0_OUT                 /* UART transmit port pin                       */

void uart_Init(void);

void uart_SendStr(char* str);

#endif /* UART_H_ */
