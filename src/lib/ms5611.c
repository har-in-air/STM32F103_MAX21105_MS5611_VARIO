#include "common.h"
#include "board.h"
#include "ms5611.h"
#include <math.h>
#include "i2c.h"
#include "uart.h"


float zCmAvg_;
volatile float zCmSample_;
volatile float paSample_;
int  celsiusSample_;
volatile int sensorState;


static u16 cal_[6];
static s64 tref_;
static s64 offT1_;
static s64 sensT1_;
static s32 tempCx100_;
static u32 D1_;
static u32 D2_;
static s64 dT_;

void MS5611_Config(void) {
	paSample_ = 0.0f;
	zCmSample_ = 0.0f;
	celsiusSample_ = 0;
	zCmAvg_ = 0.0f;

	MS5611_ReadCoefficients();
    //uart_Printf(&huart1,"\r\nCalib Coeffs : %d %d %d %d %d %d\r\n",cal_[0],cal_[1],cal_[2],cal_[3],cal_[4],cal_[5]);
    tref_ = (s64)(cal_[4])<<8;
    offT1_ = (s64)(cal_[1])<<16;
    sensT1_ = (s64)(cal_[0])<<15;
	}




#if 0

#define MAX_TEST_SAMPLES    100
extern char gszBuf[];
static float pa[MAX_TEST_SAMPLES];
static float z[MAX_TEST_SAMPLES];

void MS5611_Test(int nSamples) {
	int n;
    float paMean, zMean, zVariance, paVariance;
    paMean = 0.0f;
    zMean = 0.0f;
    paVariance = 0.0f;
    zVariance = 0.0f;
    for (n = 0; n < nSamples; n++) {
	    MS5611_TriggerTemperatureSample();
	    delayMs(MS5611_SAMPLE_PERIOD_MS);
	    D2_ = MS5611_ReadTemperatureSample();
	    MS5611_CalculateTemperatureCx10();
		MS5611_TriggerPressureSample();
		delayMs(MS5611_SAMPLE_PERIOD_MS);
		D1_ = MS5611_ReadPressureSample();
		pa[n] = MS5611_CalculatePressurePa();
        z[n] =  MS5611_Pa2Cm(pa[n]);
        paMean += pa[n];
        zMean += z[n];
        }
    paMean /= nSamples;
    zMean /= nSamples;
    uart_Printf(&huart1,"paMean = %dPa, zMean = %dcm\r\n",(int)paMean,(int)zMean);
    for (n = 0; n < nSamples; n++) {
        paVariance += (pa[n]-paMean)*(pa[n]-paMean);
        zVariance += (z[n]-zMean)*(z[n]-zMean);
        //uart_Printf(&huart1,"%d %d\r\n",(int)pa[n],(int)z[n]);
       }
    paVariance /= (nSamples-1);
    zVariance /= (nSamples-1);
    uart_Printf(&huart1,"\r\npaVariance %d  zVariance %d\r\n",(int)paVariance, (int)zVariance);
	}
#endif

void MS5611_AveragedSample(int nSamples) {
	s32 tc,tAccum,n;
    float pa,pAccum;
	pAccum = 0.0f;
    tAccum = 0;
	n = nSamples;
    while (n--) {
    	MS5611_TriggerTemperatureSample();
    	delayMs(MS5611_SAMPLE_PERIOD_MS);
		D2_ = MS5611_ReadTemperatureSample();
		MS5611_CalculateTemperatureCx10();
		MS5611_TriggerPressureSample();
		delayMs(MS5611_SAMPLE_PERIOD_MS);
		D1_ = MS5611_ReadPressureSample();
		pa = MS5611_CalculatePressurePa();
		pAccum += pa;
		tAccum += tempCx100_;
		}
	tc = tAccum/nSamples;
	celsiusSample_ = (tc >= 0 ?  (tc+50)/100 : (tc-50)/100);
	paSample_ = (pAccum+nSamples/2)/nSamples;
	zCmAvg_ = zCmSample_ = MS5611_Pa2Cm(paSample_);
#if 1
   uart_Printf(&huart1,"Tavg : %dC\r\n",celsiusSample_);
   uart_Printf(&huart1,"Pavg : %dPa\r\n",(int)paSample_);
   uart_Printf(&huart1,"Zavg : %dcm\r\n",(int)zCmAvg_);
#endif

	}
	
	

/// Fast Lookup+Interpolation method for converting pressure readings to altitude readings.
#include "pztbl.txt"

float MS5611_Pa2Cm(float paf)  {
   	s32 pa,inx,pa1,z1,z2;
    float zf;
    pa = (s32)(paf);

   	if (pa > PA_INIT) {
      	zf = (float)(gPZTbl[0]);
      	}
   	else {
      	inx = (PA_INIT - pa)>>10;
      	if (inx >= PZLUT_ENTRIES-1) {
         	zf = (float)(gPZTbl[PZLUT_ENTRIES-1]);
         	}
      	else {
         	pa1 = PA_INIT - (inx<<10);
         	z1 = gPZTbl[inx];
         	z2 = gPZTbl[inx+1];
         	zf = (float)(z1) + (((float)(pa1)-paf)*(float)(z2-z1))/1024.0f;
         	}
      	}
   	return zf;
   	}

void MS5611_CalculateTemperatureCx10(void) {
	dT_ = (s64)D2_ - tref_;
	tempCx100_ = 2000 + ((dT_*((s32)cal_[5]))>>23);
	}


float MS5611_CalculatePressurePa(void) {
	float pa;
    s64 offset, sens,offset2,sens2,t2;
	offset = offT1_ + ((((s64)cal_[3])*(s64)dT_)>>7);
	sens = sensT1_ + ((((s64)cal_[2])*(s64)dT_)>>8);
    if (tempCx100_ < 2000) {
        t2 = ((dT_*dT_)>>31); 
        offset2 = (5*(tempCx100_-2000)*(tempCx100_-2000))/2;
        sens2 = offset2/2;
        } 
    else {
        t2 = 0;
        sens2 = 0;
        offset2 = 0;
        }
    tempCx100_ -= t2;
    offset -= offset2;
    sens -= sens2;
	pa = ((float)((s64)D1_ * sens)/2097152.0f - (float)(offset)) / 32768.0f;
	return pa;
	}



void MS5611_ReadCoefficients(void) {
    int cnt;
    u08 buf[2];
    for (cnt = 0; cnt < 6; cnt++) {
    	i2c_RcvBuf(I2C_ID_MS5611,0xA2 + cnt*2,2,buf);// skip the factory data in addr A0, and the checksum at last addr
        cal_[cnt] = (((u16)buf[0])<<8) | (u16)buf[1];
        }
    }


/// Trigger a pressure sample with max oversampling rate
void MS5611_TriggerPressureSample(void) {
    i2c_XmtData(I2C_ID_MS5611, 0x48);
   }

/// Trigger a temperature sample with max oversampling rate
void MS5611_TriggerTemperatureSample(void) {
    i2c_XmtData(I2C_ID_MS5611, 0x58);
   }

u32 MS5611_ReadTemperatureSample(void)	{
   u32 w;
   u08 buf[3];
   i2c_RcvBuf(I2C_ID_MS5611,0x00,3,buf);
   w = (((u32)buf[0])<<16) | (((u32)buf[1])<<8) | (u32)buf[2];
   return w;
   }


u32 MS5611_ReadPressureSample(void)	{
   u32 w;
   u08 buf[3];
   i2c_RcvBuf(I2C_ID_MS5611,0x00,3,buf);
   w = (((u32)buf[0])<<16) | (((u32)buf[1])<<8) | (u32)buf[2];
   return w;
   }



void MS5611_InitializeSampleStateMachine(void) {
   MS5611_TriggerTemperatureSample();
   sensorState = MS5611_READ_TEMPERATURE;
   }

int MS5611_SampleStateMachine(void) {
   if (sensorState == MS5611_READ_TEMPERATURE) {
      D2_ = MS5611_ReadTemperatureSample();
      MS5611_TriggerPressureSample();
      //DBG_1(); // turn on the debug pulse for timing the critical computation
      MS5611_CalculateTemperatureCx10();
      //celsiusSample_ = (tempCx100_ >= 0? (tempCx100_+50)/100 : (tempCx100_-50)/100);
      paSample_ = MS5611_CalculatePressurePa();
	  zCmSample_ = MS5611_Pa2Cm(paSample_);
      //DBG_0();
	  sensorState = MS5611_READ_PRESSURE;
      return 1;
      }
   else
   if (sensorState == MS5611_READ_PRESSURE) {
      D1_ = MS5611_ReadPressureSample();
      MS5611_TriggerTemperatureSample();
      sensorState = MS5611_READ_TEMPERATURE;
      return 0;
      }
   return 0;    
   }









