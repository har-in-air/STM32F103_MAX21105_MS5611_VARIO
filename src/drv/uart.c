#include "common.h"
#include "config.h"
#include "board.h"
#include "stm32f1xx_hal_uart.h"
#include "uart.h"

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

void uart_ClearFifo(UART_HandleTypeDef *huart){
    int inx = sizeof(huart->rxFifo);
    huart->rxFifoHead = huart->rxFifoTail = 0;
    while (inx--) huart->rxFifo[inx] = 0;
    }

int uart_IsCharReady(UART_HandleTypeDef *huart, int waitTicks) {
    u32 tick = HAL_GetTick()+waitTicks;
    while((huart->rxFifoHead == huart->rxFifoTail) && (HAL_GetTick() < tick)) NOP();
    return ((huart->rxFifoHead != huart->rxFifoTail)? 1 : 0);
    }


u08 uart_GetChar(UART_HandleTypeDef *huart) {
    while (huart->rxFifoHead == huart->rxFifoTail) NOP();
    huart->rxFifoTail++;
    if (huart->rxFifoTail == UART_RX_FIFO_SIZE) huart->rxFifoTail=0; //wrap
    return huart->rxFifo[huart->rxFifoTail];
    }

void uart_ReportError(char* szMsg) {}

void uart_PutChar(UART_HandleTypeDef *huart, char c){
  UART_SendData(huart, c);
  }

void uart_PrintSz(UART_HandleTypeDef *huart, char *sz){
  while (*sz != '\0')  {
    uart_PutChar(huart, *sz);
    sz++;
    }
  }

void uart_PutByte(UART_HandleTypeDef *huart, u08 c){
  UART_SendData(huart, c);
  }

void uart_XmitBuf(UART_HandleTypeDef *huart, u08 * pBuf, int numBytes){
  while (numBytes--)  {
    UART_SendData(huart, *pBuf++);
    }
  }

void uart1_Config(u32 baudRate) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = baudRate;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    uart_ClearFifo(&huart1);
    HAL_UART_Init(&huart1);
    }

void uart2_Config(u32 baudRate) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = baudRate;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    uart_ClearFifo(&huart2);
    HAL_UART_Init(&huart2);
    }




void uart_Printf(UART_HandleTypeDef *huart, char* format, ...)    {
    char szbuf[200];
    va_list args;
    va_start(args,format);
    vsprintf(szbuf,format,args);
    va_end(args);
    uart_PrintSz(huart, szbuf);
    }


void uart_PutArray(UART_HandleTypeDef *huart, void * varr, int arrayLength, int typeBytes) {
    int inx;
    u08 * arr = (u08*) varr;
    for( inx = 0; inx < arrayLength; inx++) {
        uart_PutVar(huart, &arr[inx * typeBytes], typeBytes, 1);
        }
    }


// thanks to Francesco Ferrara and the Simplo project for the following code!
void uart_PutVar(UART_HandleTypeDef *huart, void * val, int typeBytes, int lowByteFirst) {
    int inx;
    u08 * addr = (u08 *)(val);
    if (lowByteFirst) {
        for (inx = 0; inx < typeBytes; inx++) {
            uart_PutByte(huart, addr[inx]);
            }
        }   
    else {
        inx = typeBytes;
        while (inx--) uart_PutByte(huart, addr[inx]);
        }
    }


void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart->Instance==USART1)
  {
    __USART1_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
  else if(huart->Instance==USART2)
  {
    __USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart){

  if(huart->Instance==USART1) {
    __USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART1_IRQn);

  }
  else if(huart->Instance==USART2)
  {
    __USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
} 
