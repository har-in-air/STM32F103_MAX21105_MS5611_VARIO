#ifndef HMC6883L_H_
#define HMC6883L_H_


#define  HMC5883L_CONFIG_A      0
#define  HMC5883L_CONFIG_B      1
#define  HMC5883L_MODE          2
#define  HMC5883L_DATA          3 // note : order = xmsb,xlsb,zmsb,zlsb,ymsb,ylsb
#define  HMC5883L_STATUS        9
#define  HMC5883L_IDA           10
#define  HMC5883L_IDB           11
#define  HMC5883L_IDC           12


#define  HMC5883L_SCALE_09      0
#define  HMC5883L_SCALE_13      1
#define  HMC5883L_SCALE_19      2
#define  HMC5883L_SCALE_25      3
#define  HMC5883L_SCALE_40      4
#define  HMC5883L_SCALE_47      5
#define  HMC5883L_SCALE_56      6
#define  HMC5883L_SCALE_81      7


// At 75Hz continuous measurement rate, the delay between samples = 1/75 = 13.3 = ~14mS
#define  HMC5883L_MEAS_PERIOD_MS     14
#define  HMC5883L_MEAS_NORMAL       0

#define HMC5883L_MEAS_CONTINUOUS        0
#define HMC5883L_MEAS_SINGLE_SHOT       1
#define HMC5883L_MEAS_IDLE              3


#define HMC5883L_CALIBRATE

#define HMC5883L_LOCAL_DECLINATION  ((double)-0.031125)  // W 1 deg 47 minutes at Bangalore airport



extern float mxned,myned,mzned;


void hmc5883L_Config(void);
int hmc5883L_IsDataReady(void);
void hmc5883L_GetRawData(s16* px, s16* py, s16* pz);
void hmc5883L_GetValues(float* px, float* py, float* pz);
u32 hmc5883L_GetID(void);
void hmc5883L_Calibrate(void);

#endif

