#include "gpio.h"


/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through 
        * the Code Generation settings)
*/
void MX_GPIO_Init(void){
  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO Ports Clock Enable 
  //__GPIOC_CLK_ENABLE();
  //__GPIOD_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

  
  // Configure GPIO pins : PA0 PA1  PA12 PA14 
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_12|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    

  //Configure GPIO pins : PB1 PB2 PB8 PB10 PB12 PB13 PB14 PB15 
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2| GPIO_PIN_8 | GPIO_PIN_10 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  
  // PA6 LCDRST
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // PA11 BTN
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  // PB11 = MAX21105 drdy interrupt
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN; // necessary !
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
 // HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  }

