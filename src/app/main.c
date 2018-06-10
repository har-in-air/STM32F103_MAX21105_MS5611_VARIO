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
#include "MahonyAHRS.h"
#include "kalmanfilter3.h"
#include "audio.h"
#include "lcdmot80x48.h"
#include "ringbuf.h"
#include <math.h>

  
volatile int drdyCounter;
volatile uint32_t gBtnState;
volatile int gbBtnPressed;

volatile float  zcmKF;
volatile float  vcpsKF;
volatile int bDataReady;
volatile float gravityCompensatedAccel;
float yawDeg, pitchDeg, rollDeg;

int altitudeM;
int vertCps;

int getBatteryVoltagex10(void);
void AHRS_Update(void);

 
int main(void){
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
   DBG_LO();
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
  //max_CalibrateAccel();
  //lcd_Printf(0,0,"ax %d", axBias);
  //lcd_Printf(1,0,"ay %d", ayBias);
  //lcd_Printf(2,0,"az %d", azBias);
 // max_CalibrateGyro();
  //lcd_Printf(0,0,"gx %d", gxBias);
  //lcd_Printf(1,0,"gy %d", gyBias);
  //lcd_Printf(2,0,"gz %d", gzBias);
  //while(1);
  MS5611_Config();
//  MS5611_Test(64);
  MS5611_AveragedSample(4);
  kalmanFilter3_Configure(MS5611_Z_VARIANCE, IMU_ACCEL_VARIANCE, IMU_ACCELBIAS_VARIANCE, zCmAvg_, 0.0, 0.0);
  
  MS5611_InitializeSampleStateMachine();
  drdyCounter = 0;
  bDataReady = 0;
  gbBtnPressed = 0;
  
  dwt_SetMarker();
  ringbuf_Init();
  AHRS_Update();
  us = dwt_ElapsedTimeUs();
  uart_Printf(&huart1, "ahrs update us = %duS\r\n", us); // 2.7ms
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
  float iirCps = 0.0f;
  int displayCps  = 0;
  int displayCounter = 0;
  int nMode = MODE_VARIO;
  int yaw,pitch, roll;
  
  while (1) {
        if (bDataReady) {
            bDataReady = 0;
            if (gbBtnPressed) {
                gbBtnPressed = 0;
                nMode++; 
                if (nMode > MODE_RESERVED) {
                    nMode = MODE_VARIO;
                    }
                lcd_Clear();
                }
            switch (nMode) {
            case MODE_VARIO : 
                altitudeM =  (int)((zcmKF + 50.0f)/100.0f);  
                int audioCps =  vcpsKF >= 0.0f ? (int)(vcpsKF+0.5f) : (int)(vcpsKF-0.5f);
                CLAMP(audioCps, -1000, 1000);
                audio_VarioBeep(audioCps);        
                iirCps = iirCps*0.97f + vcpsKF*0.03f; // use heavily damped vario reading for display
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
                yaw = (yawDeg >= 0.0f ? (int)(yawDeg+0.5f) : (int)(yawDeg - 0.5f) );
                pitch = (pitchDeg >= 0.0f ? (int)(pitchDeg+0.5f) : (int)(pitchDeg - 0.5f));
                roll = (rollDeg >= 0.0f ? (int)(rollDeg+0.5f) : (int)(rollDeg - 0.5f));
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
            
            case MODE_RESERVED:
            default :
            break;
                }
            }                
        }
    
  return 0;
  }


int getBatteryVoltagex10(void) {
  u32 adc0 = adc_Read(ADC_CHANNEL_0, 4); // 31us with ADC clock = 9mhz
  float avcc = 3.276f;
  float batVoltage = (avcc*2.0f*(float)adc0)/4091.0f;  // 10K and 10k potential divider
  int bvx10 = (int) (batVoltage*10.0f + 0.5f);
  return bvx10;
  }



void AHRS_Update(void) {
    max_GetAllChannelData();
    //hmc5883L_GetValues(&mxned,&myned,&mzned);
    float accelMagnitude = sqrt(axned*axned + ayned*ayned + azned*azned); 
    int bUseAccel = ((accelMagnitude > 750.0f) && (accelMagnitude < 1250.0f)) ? 1 : 0;
//	imu_MadgwickAHRSupdate9DOF(bUseAccel,DT_MEAS,
//					gxned*PI_DIV_180,gyned*PI_DIV_180,gzned*PI_DIV_180,
//					axned,ayned,azned,mxned,myned,mzned);
//	imu_MadgwickAHRSupdate6DOF(bUseAccel,DT_MEAS,
//					gxned*PI_DIV_180,gyned*PI_DIV_180,gzned*PI_DIV_180,
//					axned,ayned,azned);
	imu_MahonyAHRSupdate6DOF(bUseAccel,DT_MEAS, gxned*PI_DIV_180,gyned*PI_DIV_180,gzned*PI_DIV_180,
					axned,ayned,azned);				
    gravityCompensatedAccel = imu_GravityCompensatedAccel(axned,ayned,azned,q0,q1,q2,q3);
    ringbuf_AddSample(gravityCompensatedAccel);
    imu_Quaternion2YawPitchRoll(q0,q1,q2,q3, &yawDeg, &pitchDeg, &rollDeg);    
    }





/**
* @brief This function handles EXTI 15:10 line interrupts.
* MAX21105 drdy signal  is on PB11
*/
// called every ~2.5mS at drdy interrupt (sampling at ~400Hz)
void EXTI15_10_IRQHandler(void){
    DBG_HI();
    AHRS_Update();
    drdyCounter++;
    if (drdyCounter > 3) {
        drdyCounter = 0;
        gBtnState = (gBtnState<<1) | ((uint32_t)BTN_PRESSED);
        if ((gBtnState | 0xFFFFFFF0) == 0xFFFFFFF8) {
            gbBtnPressed = 1;
            }
        if (MS5611_SampleStateMachine()) {
            float zAccelAverage = ringbuf_AverageOldestSamples(8);
            kalmanFilter3_Update(zCmSample_,zAccelAverage, DT_MEAS*8, &zcmKF, &vcpsKF);
            bDataReady = 1;
            }
        }

  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  DBG_LO();
}
