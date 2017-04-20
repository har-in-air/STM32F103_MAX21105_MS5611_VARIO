// STM32F103C8T6 : MAX21105 + HMC5883L + MS5611 altimeter-vario

#include "common.h"
#include "config.h" 
#include "board.h"
#include "stm32f1xx.h"
#include "gpio.h"
#include "sysclk.h"
#include "uart.h"
#include "dwt.h"
#include "adc.h"
#include "uart.h"
#include "i2c.h"
#include "hmc5883l.h"
#include "max21105.h"
#include "ms5611.h"
#include "MadgwickAHRS.h"
#include "kalmanfilter3.h"
#include "audio.h"
#include "lcdmot80x48.h"
#include <math.h>

  
volatile int drdyCounter = 0;
volatile uint32_t gBtnState;
volatile int gbBtnPressed;

volatile float  zcmKF;
volatile float  vcpsKF;
volatile int bDataReady = 0;

int altitudeM;
int vertCps;

int getBatteryVoltagex10(void);
void AHRS_Update(void);

 
int main(void){
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();  
  i2c_Config();
  dwt_Init();
  uart1_Config(115200);  
  audio_Config();
  
  uart_Printf(&huart1, "\r\nc8t6 imu vario %s %s\r\n", __DATE__, __TIME__);	
  uart_Printf(&huart1, "SystemCoreClock %ld\r\n", HAL_RCC_GetHCLKFreq());	
  
  HAL_Delay(100);  	
  u32 hmcID = hmc5883L_GetID();
  uart_Printf(&huart1, "HMC5883L ID = 0x%X [0x483433]\r\n", hmcID);
  hmc5883L_Config();
  
  u08 maxID = max_GetID();
  uart_Printf(&huart1, "MAX21105 ID = 0x%X [0xB4]\r\n", maxID);
  max_Configure();
  
  lcd_Init();

  u32 us;


#if 0
    while (1) {
        delayMs(50);
        s16 ax,ay,az;
        max_GetRawData(MAX21105_ACC_X_H,&ax,&ay,&az);
        s16 gx,gy,gz;
        max_GetRawData(MAX21105_GYRO_X_H,&gx,&gy,&gz);
        s16 mx,my,mz;
        hmc5883L_GetRawData(&mx,&my,&mz);
        //uart_Printf(&huart1,"A :\t%d\t%d\t%d\r\n",ax,ay,az);
        //uart_Printf(&huart1,"G :\t%d\t%d\t%d\r\n",gx,gy,gz);
        uart_Printf(&huart1,"M :\t%d\t%d\t%d\r\n\r\n",mx,my,mz);
        }
#endif
        
  //max_CalibrateAccel();
  //max_CalibrateGyro();
  //hmc5883L_Calibrate();
  
  int bvx10 = getBatteryVoltagex10();
  uart_Printf(&huart1, "Battery Voltage = %d.%dV\r\n", bvx10/10, bvx10%10);
  delayMs(1000);
  lcd_Printf(0,0,"IMU Vario");
  lcd_Printf(1,0,"v0.9");
  lcd_Printf(2,0,"%d.%dV", bvx10/10, bvx10%10);
  __ADC1_CLK_DISABLE();
  delayMs(3000);
  lcd_Clear();
  
  MS5611_Config();
//  MS5611_Test(64);
  MS5611_AveragedSample(4);
  kalmanFilter3_Configure(MS5611_Z_VARIANCE, IMU_ACCEL_VARIANCE, IMU_ACCELBIAS_VARIANCE, zCmAvg_, 0.0f, 0.0f);
  
  MS5611_InitializeSampleStateMachine();
  drdyCounter = 0;
  
  dwt_SetMarker();
  AHRS_Update();
  us = dwt_ElapsedTimeUs();
  uart_Printf(&huart1, "ahrs update us = %duS\r\n", us); // 2.7ms
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  float iirCps = 0.0f;
  int displayCps  = 0;
  int displayCounter = 0;
  int userMode = MODE_VARIO;
  int yaw,pitch, roll;
  
  while (1) {
        if (bDataReady) {
            bDataReady = 0;
            if (gbBtnPressed) {
                gbBtnPressed = 0;
                userMode++; 
                if (userMode > MODE_CALIB) {
                    userMode = MODE_VARIO;
                    }
                lcd_Clear();
                }
            switch (userMode) {
                case MODE_VARIO :
                DINT();
                altitudeM =  (int)((zcmKF + 50.0f)/100.0f);  
                int audioCps =  vcpsKF >= 0.0f ? (int)(vcpsKF+0.5f) : (int)(vcpsKF-0.5f);
                iirCps = iirCps*0.95f + vcpsKF*0.05f; // use heavily damped vario reading for display
                EINT();
                CLAMP(audioCps, -1000, 1000);
                audio_VarioBeep(audioCps);        
                displayCps =  iirCps >= 0.0f ? (int)((iirCps+5.0f)/10.0f) : (int)((iirCps-5.0f)/10.0f);
                CLAMP(displayCps, -99, 99);
            
                if (displayCounter == 0 ) {
                    lcd_Printf13x16(0,28,"%4d",altitudeM);
                    }
                else
                if (displayCounter == 1 ) {
                    int neg = 0;
                    if (displayCps < 0) {
                        displayCps = -displayCps; 
                        neg = 1;
                        }
                    lcd_Printf13x16(4,28,"%c%d.%d",neg ? '-':'+',displayCps/10,displayCps%10);
                    }
                displayCounter++;
                if (displayCounter >= 5 ) {
                    displayCounter = 0;
                    }
                break;
            
                case MODE_YPR:
                DINT();
                yaw = (yawDeg >= 0.0f ? (int)(yawDeg+0.5f) : (int)(yawDeg - 0.5f) );
                pitch = (pitchDeg >= 0.0f ? (int)(pitchDeg+0.5f) : (int)(pitchDeg - 0.5f));
                roll = (rollDeg >= 0.0f ? (int)(rollDeg+0.5f) : (int)(rollDeg - 0.5f));
                EINT();
                int neg = 0;
                if (yaw < 0 ) {
                    neg = 1; 
                    yaw = -yaw;
                    }
                lcd_Printf13x16(0,0,"%c%03d",neg ? '-':'+', yaw);
                neg = 0;
                if (pitch < 0 ) {
                    neg = 1; 
                    pitch = -pitch;
                    }
                lcd_Printf13x16(2,0,"%c%03d",neg ? '-':'+', pitch);
                neg = 0;
                if (roll < 0 ) {
                    neg = 1; 
                    roll = -roll;
                    }
                lcd_Printf13x16(4,0,"%c%03d",neg ? '-':'+', roll);
                
                break;
            
                case MODE_CALIB:
                default :
                break;
                }
            }                
        }
    
  return 0;
  }


int getBatteryVoltagex10(void) {
  u32 adc0 = adc_Read(ADC_CHANNEL_0, 4); 
  float avcc = 3.276f; // measured at output of the LDO regulator
  float batVoltage = (avcc*2.0f*(float)adc0)/4091.0f;  // 10K and 10k potential divider
  int bvx10 = (int) (batVoltage*10.0f + 0.5f); // e.g. 39 = 3.9V
  return bvx10; 
  }



void AHRS_Update(void) {
    max_GetAllChannelData();
    //hmc5883L_GetValues(&mxned,&myned,&mzned);
    float accelMagnitude = sqrt(axned*axned + ayned*ayned + azned*azned); 
    // accelerometer data is used for computing orientation only when magnitude is between 0.75G and 1.25G.
    // otherwise, the computation only integrates the gyroscope data.
    int bAHRSUseAccel = ((accelMagnitude > 750.0f) && (accelMagnitude < 1250.0f)) ? 1 : 0;
    
//	imu_MadgwickAHRSupdate9DOF(bAHRSUseAccel,0.005f,
//					gxned*PI_DIV_180,gyned*PI_DIV_180,gzned*PI_DIV_180,
//					axned,ayned,azned,mxned,myned,mzned);
	imu_MadgwickAHRSupdate6DOF(bAHRSUseAccel,0.005f,
					gxned*PI_DIV_180,gyned*PI_DIV_180,gzned*PI_DIV_180,
					axned,ayned,azned);
    imu_GravityCompensatedAccel(axned,ayned,azned,q0,q1,q2,q3);
    imu_Quat2YPR(q0,q1,q2,q3);    
    }





/**
* @brief This function handles EXTI 15:10 line interrupts.
* MAX21105 drdy signal  is on PB11
*/
void EXTI15_10_IRQHandler(void){
    AHRS_Update();
    drdyCounter++;
    if (drdyCounter > 2) {
        drdyCounter = 0;
        // debounce button
        gBtnState = (gBtnState<<1) | ((uint32_t)BTN_PRESSED);
        if ((gBtnState | 0xFFFFFFF0) == 0xFFFFFFF8) {
            gbBtnPressed = 1;
            }
        if (MS5611_SampleStateMachine()) {
            kalmanFilter3_Update(zCmSample_,gravityCompensatedAccel, 0.030f, &zcmKF, &vcpsKF);
            bDataReady = 1;
            }
        }

  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
}