#include "common.h"
#include "board.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "config.h"
#include "piezo.h"

TIM_HandleTypeDef    htim1;
TIM_ClockConfigTypeDef sClockSourceConfig;
TIM_MasterConfigTypeDef sMasterConfig;
TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
TIM_OC_InitTypeDef sConfigOC;

void Error_Handler(void){
  while(1) ;
}


GPIO_InitTypeDef GPIO_InitStruct;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base){
  if(htim_base->Instance==TIM1){
    __HAL_RCC_TIM1_CLK_ENABLE();
  }
 }

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base){
  if(htim_base->Instance==TIM1)  {
    __HAL_RCC_TIM1_CLK_DISABLE();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim){
  if(htim->Instance==TIM1) {  
    /**TIM1 GPIO Configuration    
    PA7     ------> TIM1_CH1N
    PA8     ------> TIM1_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    __HAL_AFIO_REMAP_TIM1_PARTIAL();
  }

}



void piezo_Config(void){
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 4-1; // tim1 clock= 24000000 / 4 = 6000000
  htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim1.Init.Period = 99; // dummy value for initialization
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK){
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK){
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK){
    Error_Handler();
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0; // dummy value for initialization
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim1);
}


void piezo_SetFrequency(int freqHz) {
  HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1); 
  HAL_TIM_Base_Stop(&htim1);
  if (freqHz) {
    int period = 6000000/freqHz; // minimum frequency is 6000000/65535 = 91.5 Hz
    htim1.Init.Period = period-1; 
    HAL_TIM_Base_Init(&htim1);
    HAL_TIM_PWM_Init(&htim1);
    sConfigOC.Pulse =  period/2;  // 50% duty cycle
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1); // generate pulse on TIM1_CH1
  
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);    //starts PWM on CH1 pin
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1); //starts PWM on CH1N pin
    }
}