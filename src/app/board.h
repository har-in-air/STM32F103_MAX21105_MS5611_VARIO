#ifndef BOARD_H_
#define BOARD_H_

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"

#define LOW     0
#define HIGH    1

#define I2C_ID_MAX21105     0xB0
#define I2C_ID_HMC5883L     0x3C
#define I2C_ID_MS5611       0xEE

#define I2CPORT     GPIOA
#define PIN_SDA     GPIO_PIN_2
#define PIN_SCL     GPIO_PIN_3

#define I2CLCDPORT     GPIOA
#define PIN_LCDSDA     GPIO_PIN_4
#define PIN_LCDSCL     GPIO_PIN_5

#define PIN_LCDRST	   GPIO_PIN_6
#define LCDRST_LO()    {GPIOA->BSRR = (PIN_LCDRST<<16);}
#define LCDRST_HI()    {GPIOA->BSRR = PIN_LCDRST;}

#define BTN_PIN         GPIO_PIN_11

#define BTN_PRESSED      ((GPIOA->IDR & BTN_PIN) ? 1 : 0)


#define delayMs(mSecs)  HAL_Delay(mSecs)
#define millis()        HAL_GetTick()

#endif // BOARD_H_
