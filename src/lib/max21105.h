#ifndef MAX21105_H_
#define MAX21105_H_

#define MAX_CALIBRATE

#define MAX21105_START_UP_DELAY_MS   100
#define MAX21105_SAMPLE_DELAY_MS      3  // more than enough for 400Hz ODR

#define MAX21105_2000DPS_SENSITIVITY 15.0 
#define MAX21105_1000DPS_SENSITIVITY 30.0  // 30 digits / degree-per-sec


// MAX21105 Common Bank
#define MAX21105_WHOAMI        0x20    // should read 0xB4              
#define MAX21105_SILICON_REV_OTP 0x21                                                                          
#define MAX21105_EXT_STATUS    0x22      
#define MAX21105_BANK_SELECT    0x22      
#define MAX21105_SYSTEM_STATUS 0x23
#define MAX21105_GYRO_X_H      0x24
#define MAX21105_GYRO_X_L      0x25
#define MAX21105_GYRO_Y_H      0x26
#define MAX21105_GYRO_Y_L      0x27
#define MAX21105_GYRO_Z_H      0x28
#define MAX21105_GYRO_Z_L      0x29
#define MAX21105_ACC_X_H       0x2A
#define MAX21105_ACC_X_L       0x2B
#define MAX21105_ACC_Y_H       0x2C
#define MAX21105_ACC_Y_L       0x2D
#define MAX21105_ACC_Z_H       0x2E
#define MAX21105_ACC_Z_L       0x2F
#define MAX21105_TEMP_H        0x30
#define MAX21105_TEMP_L        0x31
#define MAX21105_TRM_BANK_REG  0x38
#define MAX21105_FIFO_COUNT    0x3C
#define MAX21105_FIFO_STATUS   0x3D
#define MAX21105_FIFO_DATA     0x3E
#define MAX21105_RST_REG       0x3F

#define MAX21105_XOFFSET  8
#define MAX21105_YOFFSET  -7
#define MAX21105_ZOFFSET  -7

// MAX21105 Bank 0
#define MAX21105_SET_PWR       0x00
#define MAX21105_SNS_CFG_1     0x01
#define MAX21105_SNS_CFG_2     0x02
#define MAX21105_SNS_CFG_3     0x03
#define MAX21105_SET_ACC_PWR   0x04
#define MAX21105_ACC_CFG_1     0x05
#define MAX21105_ACC_CFG_2     0x06

#define MAX21105_SET_TEMP_DR   0x13
#define MAX21105_SET_PU_PD_PAD 0x14
#define MAX21105_SET_I2C_PAD   0x15
#define MAX21105_MIF_CFG       0x16
#define MAX21105_FIFO_STORE    0x17
#define MAX21105_FIFO_THS      0x18
#define MAX21105_FIFO_CFG      0x19
#define MAX21105_OTP_STS_CFG   0x1C

// MAX21105 Bank 1
#define MAX21105_INT_REF_X     0x00
#define MAX21105_INT_REF_Y     0x01
#define MAX21105_INT_REF_Z     0x02
#define MAX21105_INT_DEB_X     0x03
#define MAX21105_INT_DEB_Y     0x04
#define MAX21105_INT_DEB_Z     0x05
#define MAX21105_INT_THS_X     0x06
#define MAX21105_INT_THS_Y     0x07
#define MAX21105_INT_THS_Z     0x08
#define MAX21105_INT_COND      0x09
#define MAX21105_INT_CFG_1     0x0A
#define MAX21105_INT_CFG_2     0x0B
#define MAX21105_INT_TM0_CFG   0x0C
#define MAX21105_INT_STATUS_UL 0x0D
#define MAX21105_INT_STS       0x0E
#define MAX21105_INT_MSK       0x0F
#define MAX21105_INT_SRC_CFG   0x17
#define MAX21105_SERIAL_NUM_B5 0x1A
#define MAX21105_SERIAL_NUM_B4 0x1B
#define MAX21105_SERIAL_NUM_B3 0x1C
#define MAX21105_SERIAL_NUM_B2 0x1D
#define MAX21105_SERIAL_NUM_B1 0x1E
#define MAX21105_SERIAL_NUM_B0 0x1F

// MAX21105 Bank 2
#define MAX21105_BIAS_COMP_GX_MSB   0x13
#define MAX21105_BIAS_COMP_GX_LSB   0x14
#define MAX21105_BIAS_COMP_GY_MSB   0x15
#define MAX21105_BIAS_COMP_GY_LSB   0x16
#define MAX21105_BIAS_COMP_GZ_MSB   0x17
#define MAX21105_BIAS_COMP_GZ_LSB   0x18
#define MAX21105_BIAS_COMP_AX 		0x19
#define MAX21105_BIAS_COMP_AY 		0x1A
#define MAX21105_BIAS_COMP_AZ 		0x1B
#define MAX21105_GYR_ODR_TRIM     	0x1F

#define MAX21105_MAXOFFSET    12

extern float gxned,gyned,gzned;
extern float axned,ayned,azned;



void max_InitCalibrationData(void);
void max_Configure(void);
void max_Configure_OIS_DRDY(void);
void max_GetAllChannelData(void);
//void max_GetAccelValues(float* pax, float* pay, float* paz);
void max_GetAccelValues(s16 ax, s16 ay, s16 az, float* pax, float* pay, float* paz);
//void max_GetGyroValues(float* pgx, float* pgy, float* pgz);
void max_GetGyroValues(s16 gx, s16 gy, s16 gz, float* pgx, float* pgy, float* pgz);
u08 max_GetID(void);

#ifdef MAX_CALIBRATE
void max_ReadBuf(u08 addr, int burst, int numBytes, u08* pBuf);
void max_BufTos16(u08* buf, s16* px, s16* py, s16*pz);
void max_GetRawData(u08 addr, s16 *px, s16* py, s16* pz);
void max_GetAverageRawData(int numSamples, u08 addr, int* pXavg, int* pYavg, int* pZavg);
void max_SelfTest(float * selfTest);
void max_CalibrateGyro(void);
void max_CalibrateAccel(void);
#endif


#endif
