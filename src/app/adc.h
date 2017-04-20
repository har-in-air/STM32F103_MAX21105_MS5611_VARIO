#ifndef __adc_H
#define __adc_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"

extern ADC_HandleTypeDef hadc1;
void MX_ADC1_Init(void);
uint32_t adc_Read(uint32_t channel, int numSamples);

#ifdef __cplusplus
}
#endif
#endif /*__ adc_H */

