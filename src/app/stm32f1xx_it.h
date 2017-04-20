#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 


void SysTick_Handler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void EXTI15_10_IRQHandler(void); // in main.c
#ifdef __cplusplus
}
#endif

#endif /* __STM32F1xx_IT_H */

