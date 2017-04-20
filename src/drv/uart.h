#ifndef UART_H_
#define UART_H_

#include "stm32f1xx_hal_uart.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void uart1_Config(u32 Baudrate);
void uart2_Config(u32 Baudrate);
void uart_XmitBuf(UART_HandleTypeDef *huart,u08 *pBuf, int numBytes);
void uart_PrintSz(UART_HandleTypeDef *huart,char *szBuf);
void uart_Printf(UART_HandleTypeDef *huart,char* format, ...);
void uart_PrintChar(UART_HandleTypeDef *huart,char b);
void uart_ClearFifo(UART_HandleTypeDef *huart);
int uart_IsCharReady(UART_HandleTypeDef *huart,int waitTicks);
u08 uart_GetChar(UART_HandleTypeDef *huart);
void uart_PutByte(UART_HandleTypeDef *huart,u08 b);
void uart_PutChar(UART_HandleTypeDef *huart,char b);
void uart_PutArray(UART_HandleTypeDef *huart,void * varr, int arrayLength, int typeBytes);
void uart_PutVar(UART_HandleTypeDef *huart,void * val, int typeBytes, int lowByteFirst);


#endif // UART_H

