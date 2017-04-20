#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"
#include "adc.h"
#include "gpio.h"


ADC_HandleTypeDef hadc1;
static  ADC_ChannelConfTypeDef sConfig;

void MX_ADC1_Init(void) {
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  HAL_ADC_Init(&hadc1);
  HAL_ADCEx_Calibration_Start(&hadc1);
  sConfig.Channel = ADC_CHANNEL_0; // default
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  }

// 31uS  @ 9MHz adc clock, 4 samples
uint32_t adc_Read(uint32_t channel, int numSamples) {
  sConfig.Channel = channel;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
  uint32_t accum = 0;
  for (int inx = 0; inx < numSamples; inx++) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 1);
    uint32_t res = HAL_ADC_GetValue(&hadc1);
    accum += res;
    }
  return (accum/numSamples);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc){
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hadc->Instance==ADC1){
    /* Peripheral clock enable */
    __ADC1_CLK_ENABLE();
  
    /**ADC1 GPIO Configuration    
    PA0-WKUP     ------> ADC1_IN0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc){
  if(hadc->Instance==ADC1){
    __ADC1_CLK_DISABLE();
  
    /**ADC1 GPIO Configuration    
    PA0-WKUP     ------> ADC1_IN0
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
  }
} 


