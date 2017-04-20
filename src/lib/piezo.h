#ifndef PIEZO_H_
#define PIEZO_H_

extern TIM_HandleTypeDef    htim1;

void piezo_Config(void);
void piezo_SetFrequency(int freqHz);

#endif