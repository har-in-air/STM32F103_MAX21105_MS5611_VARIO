#include "common.h"
#include "board.h"
#include "config.h"
#include "i2c.h"
#include "uart.h"
#include "max21105.h"

#define max_Write8(addr, val) 	i2c_XmtByte(I2C_ID_MAX21105,addr,val)
#define max_Read8(addr) 		i2c_RcvByte(I2C_ID_MAX21105,addr)
#define max_ReadBuf(addr,burst,numBytes,pBuf) i2c_RcvBuf(I2C_ID_MAX21105,addr,numBytes,pBuf)

static s16 gxBias_;
static s16 gyBias_;
static s16 gzBias_;
static float gyroScale_;

static s16 ax0g_;
static s16 ay0g_;
static s16 axp1g_;
static s16 axm1g_;
static s16 ayp1g_;
static s16 aym1g_;
static s16 azp1g_;
static s16 azm1g_;
static s16 az0g_;
static float axScale_;
static float ayScale_;
static float azScale_;

float gxned,gyned,gzned; // gyro rotation rates in NED coordinates, in degrees/second
float axned,ayned,azned; // accelerometer values in NED coordinates, in Gs

// gyro sensitivity = counts / degree-per-second
// accel sensitivity = counts / G
// scale = 1 / sensitivity
void max_InitCalibrationData(void) {
#ifdef MAX21105_BREAKOUT_1
    // valid for +/-8G FS
    ax0g_ = 117;
    ay0g_ = -291;
    axm1g_ = -3624;
    axp1g_ = 3826;
    ayp1g_ = 3462;
    aym1g_ = -3985;
    azp1g_ = 3838;
    azm1g_ = -3665;
#endif
#ifdef MAX21105_BREAKOUT_2
    // valid for +/-8G FS
    ax0g_ = 117;
    ay0g_ = -291;
    axm1g_ = -3624;
    axp1g_ = 3826;
    ayp1g_ = 3462;
    aym1g_ = -3985;
    azp1g_ = 3838;
    azm1g_ = -3665;
#endif

    axScale_ = 2.0/(float)(axp1g_ - axm1g_);
    ayScale_ = 2.0/(float)(ayp1g_ - aym1g_);
    azScale_ = 2.0/(float)(azp1g_ - azm1g_);
    az0g_ = azp1g_ - ((azp1g_ - azm1g_)/2);
    
    // valid for +/-1000dps FS
    gxBias_ = 0; 
    gyBias_ = 0; 
    gzBias_ = 0; 
    
    gyroScale_  = 1.0/MAX21105_1000DPS_SENSITIVITY; 
    }
	

u08 max_GetID(void) {
  return max_Read8(MAX21105_WHOAMI);
  }	


void max_Configure(void) {
    max_InitCalibrationData();
    max_Write8(MAX21105_BANK_SELECT, 0); // select bank 0
    max_Write8(MAX21105_SET_PWR, 0); // power down
    max_Write8(MAX21105_SNS_CFG_1, 0x11); // gyro bw = 10hz, fs = 1000dps 
    max_Write8(MAX21105_SNS_CFG_2, 0x04); // gyro hpf disabled, odr = 400Hz
    //max_Write8(MAX21105_SNS_CFG_3, 0x02); // gyro hpf cutoff = 0.24hz use defaults 
    max_Write8(MAX21105_SET_ACC_PWR, 0x40); // accel fs = 8G, no selftest 
    max_Write8(MAX21105_ACC_CFG_1, 0x02); // hpf = odr/400, lpf= odr/48, odr = 400hz
    max_Write8(MAX21105_ACC_CFG_2, 0x50); // acc lpf, avg 8 readings    
    max_Write8(MAX21105_MIF_CFG, 0x10); // I2C highspeed 3.4MHz interface, big endian
    max_Write8(MAX21105_SET_PU_PD_PAD, 0x29); // pulldown int1 and int2, disconnect slave i2c pullups
 //   max_Write8(MAX21105_SET_TEMP_DR, 0x01);//clear DRDY when all registers read, enable temp
    max_Write8(MAX21105_SET_PWR, 0x78); // power mode = acc low-noise + gyro low-noise
    }


void max_Configure_OIS_DRDY(void) {
    max_InitCalibrationData();
    max_Write8(MAX21105_BANK_SELECT, 0); // select bank 0
    max_Write8(MAX21105_SET_PWR, 0); // power down
    max_Write8(MAX21105_SNS_CFG_1, 0x10); // gyro bw = 10hz, fs = 1000dps (ois lpf enabled) 
    max_Write8(MAX21105_SNS_CFG_2, 0x24); // ois lpf enable,gyro hpf disabled, gyro odr = 400Hz
    max_Write8(MAX21105_SET_ACC_PWR, 0x40); // accel fs = 8G, no selftest 
    max_Write8(MAX21105_ACC_CFG_1, 0x02); // hpf = odr/400, lpf= odr/48, acc odr = 400hz
    max_Write8(MAX21105_ACC_CFG_2, 0x50); // acc lpf, avg 8 readings    
    max_Write8(MAX21105_MIF_CFG, 0x10); // I2C highspeed 3.4MHz interface, big endian
    max_Write8(MAX21105_SET_PU_PD_PAD, 0x29); // pulldown int1 and int2, disconnect slave i2c pullups
	max_Write8(MAX21105_SET_TEMP_DR,0x00); // DRDY clear on all

    max_Write8(MAX21105_BANK_SELECT, 1); // select bank 1
    max_Write8(MAX21105_INT_SRC_CFG,0x20); // only gyro drdy
    max_Write8(MAX21105_INT_MSK, 0x80); // enable drdy on INT1
    max_Write8(MAX21105_INT_TM0_CFG, 0x00); // INT1 unlatched
    max_Write8(MAX21105_INT_CFG_2, 0x20); // INT1 enable

    max_Write8(MAX21105_BANK_SELECT, 0); // select bank 0
    max_Write8(MAX21105_SET_PWR, 0x78); // power mode = acc low-noise + gyro low-noise
    }


void max_GetAllChannelData(void) {
    u08 buf[14];
	s16 gx,gy,gz,ax,ay,az;
//    max_ReadBuf(MAX21105_GYRO_X_H, AUTO_INCR, 14, buf);
    max_ReadBuf(MAX21105_GYRO_X_H, AUTO_INCR, 12, buf);
    gx = (s16)((((u16)buf[0]) << 8) | (u16)buf[1]);
    gy = (s16)((((u16)buf[2]) << 8) | (u16)buf[3]);
    gz = (s16)((((u16)buf[4]) << 8) | (u16)buf[5]);
    
	ax = (s16)((((u16)buf[6]) << 8) | (u16)buf[7]);
    ay = (s16)((((u16)buf[8]) << 8) | (u16)buf[9]);
    az = (s16)((((u16)buf[10]) << 8) | (u16)buf[11]);
	
	max_GetGyroValues(gx, gy, gz, &gxned, &gyned, &gzned);
	max_GetAccelValues(ax, ay, az, &axned, &ayned, &azned);
	}



#ifdef MAX21105_RCPLANE
// We are using the sensor board with the ldo regulator side facing "forward"
// and with standard N-E-D aviation right-handed coordinate system
// North = forward = x,  East = right = y, Down = z
// Roll right +ve, Pitch up +ve, Yaw right +ve
// To map the gyro data to this convention, we need to do this :
// gy_ned = -gyro_x, gx_ned = -gyro_y, gz_ned = -gyro_z
// returns rotation rates in degrees/second

void max_GetGyroValues(s16 gx, s16 gy, s16 gz, float* pgx, float* pgy, float* pgz) {
    *pgy = (float)(gxBias_ - gx) * gyroScale_;
    *pgx = (float)(gyBias_ - gy) * gyroScale_;
    *pgz = (float)(gzBias_ - gz) * gyroScale_;
    }


// We are using the sensor board with the ldo regulator side facing forward
// and with standard N-E-D aviation right-handed coordinate system
// North = forward = x,  East = right = y, Down = z
// To map the accerometer data to these coordinates, we need to do this :
// ax_ned = acc_y, ay_ned = acc_x, az_ned = acc_z
// returns acceleration values in G

void max_GetAccelValues(s16 ax, s16 ay, s16 az, float* pax, float* pay, float* paz) {
    *pax = (float)(ay - ay0g_) * ayScale_;   
    *pay = (float)(ax - ax0g_) * axScale_;
    *paz = (float)(az - az0g_) * azScale_;
    }
#endif

#ifdef MAX21105_PHONEGIMBAL
// We are using the sensor board with the ldo regulator side facing "forward"
// and with standard N-E-D aviation right-handed coordinate system
// North = forward = x,  East = right = y, Down = z
// Roll right +ve, Pitch up +ve, Yaw right +ve
// To map the gyro data to this convention, we need to do this :
// gy_ned = -gyro_x, gx_ned = -gyro_y, gz_ned = -gyro_z
// returns rotation rates in degrees/second

void max_GetGyroValues(s16 gx, s16 gy, s16 gz, float* pgx, float* pgy, float* pgz) {
    *pgy = (float)(gxBias_ - gx) * gyroScale_;
    *pgx = (float)(gyBias_ - gy) * gyroScale_;
    *pgz = (float)(gzBias_ - gz) * gyroScale_;
    }


// We are using the sensor board with the ldo regulator side facing forward
// and with standard N-E-D aviation right-handed coordinate system
// North = forward = x,  East = right = y, Down = z
// To map the accerometer data to these coordinates, we need to do this :
// ax_ned = acc_y, ay_ned = acc_x, az_ned = acc_z
// returns acceleration values in G

void max_GetAccelValues(s16 ax, s16 ay, s16 az, float* pax, float* pay, float* paz) {
    *pax = (float)(ay - ay0g_) * ayScale_;   
    *pay = (float)(ax - ax0g_) * axScale_;
    *paz = (float)(az - az0g_) * azScale_;
    }
#endif



#ifdef MAX_CALIBRATE

void max_GetRawData(u08 addr, s16 *px, s16* py, s16* pz) {
    u08 buf[6];
    max_ReadBuf(addr, AUTO_INCR, 6, buf);
    max_BufTos16(buf, px, py, pz);
    }

void max_BufTos16(u08* buf, s16* px, s16* py, s16*pz) {
    *px = (s16)((((u16)buf[0]) << 8) | (u16)buf[1]);
    *py = (s16)((((u16)buf[2]) << 8) | (u16)buf[3]);
    *pz = (s16)((((u16)buf[4]) << 8) | (u16)buf[5]);
    }


int max_AverageSamples(s16 buf[], int numSamples) {
    int cnt;
    int average;
    cnt = numSamples;
    average = 0;
    while (cnt--) {
        average += (int)buf[cnt];
        }
    average +=  ((average < 0)? -numSamples/2 : numSamples/2);
    average /= numSamples;
    return average;
    }
    
#define MAX_CALIB_SAMPLES   30

s16 gXBuf[MAX_CALIB_SAMPLES];
s16 gYBuf[MAX_CALIB_SAMPLES];
s16 gZBuf[MAX_CALIB_SAMPLES];


void max_CalibrateGyro(void) {
    int x,y,z,bdone;

    bdone = 0;
	uart_ClearFifo(&huart1);
    while (!bdone) {
        uart_Printf(&huart1, "Do not disturb sensor during calibration\r\n");
        delayMs(1000);
        max_GetAverageRawData(30,MAX21105_GYRO_X_H, &x, &y, &z);
        gxBias_ = (s16)x;
        gyBias_ = (s16)y;
        gzBias_ = (s16)z;
        uart_Printf(&huart1,"gxBias = %d\r\n",x);
        uart_Printf(&huart1,"gyBias = %d\r\n",y);
        uart_Printf(&huart1,"gzBias = %d\r\n",z);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        }
	}


void max_GetAverageRawData(int numSamples, u08 addr, int* pXavg, int* pYavg, int* pZavg)  {
    for (int cnt = 0; cnt < numSamples; cnt++){
        max_GetRawData(addr, &gXBuf[cnt], &gYBuf[cnt], &gZBuf[cnt]);
        delayMs(MAX21105_SAMPLE_DELAY_MS);
        }
    *pXavg = max_AverageSamples(gXBuf,numSamples);
    *pYavg = max_AverageSamples(gYBuf,numSamples);
    *pZavg = max_AverageSamples(gZBuf,numSamples);
    }

// Step A
// Place each axis in +1g and -1g orientation and get the test axis averaged readings
// The difference of the +1g and -1g readings divided by 2 is the axis sensitivity in digits/G
// The inverse of the sensitivity is the axis scale factor in G's

// Step B
// Place board horizontally and get the X,Y and Z averaged readings
// The X and Y readings are in 0g, therefore these are the 0g offsets. Subtract them from
// the X and Y raw readings to get the corrected X and Y readings
// The Z reading is in +1g, so subtract the Z sensitivity (LSb/g) found in Step A
// to get the Z axis 0g offset.
// Subtract the Z axis 0g offset from the Z raw reading to get the
// corrected Z reading.
//
// To get the acceleration in g's, multiply the (0g corrected) sensor output by the scale
// To get the acceleration in cm/s*s, multiply the g's by by 980cm/s*s


void max_CalibrateAccel(void) {
    int x,y,z,bdone;

    bdone = 0;
	uart_ClearFifo(&huart1);
    while (!bdone) {
        uart_Printf(&huart1, "Orient board Z axis UP\r\n");
        delayMs(1000);
        max_GetAverageRawData(20,MAX21105_ACC_X_H, &x, &y, &z);
        ax0g_ = (s16)x;
        ay0g_ = (s16)y;
        azp1g_ = (s16)z;
        uart_Printf(&huart1,"ax0g = %d\r\n",x);
        uart_Printf(&huart1,"ay0g = %d\r\n",y);
        uart_Printf(&huart1,"azp1g = %d\r\n",z);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        }

    bdone = 0;
	uart_ClearFifo(&huart1);
    while (!bdone) {
        uart_Printf(&huart1,"Orient board Z axis DOWN\r\n");
        delayMs(1000);
        max_GetAverageRawData(20, MAX21105_ACC_X_H,&x, &y, &z);
        azm1g_ = (s16)z;
        uart_Printf(&huart1,"azm1g = %d\r\n",z);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        }

    bdone = 0;
	uart_ClearFifo(&huart1);
    while (!bdone) {
        uart_Printf(&huart1, "Orient board X axis UP\r\n");
        delayMs(1000);
        max_GetAverageRawData(20,MAX21105_ACC_X_H, &x, &y, &z);
        axp1g_ = (s16)x;
        uart_Printf(&huart1,"axp1g = %d\r\n", x);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        }

    bdone = 0;
	uart_ClearFifo(&huart1);
    while (!bdone) {
        uart_Printf(&huart1, "Orient board X axis down\r\n");
        delayMs(1000);
        max_GetAverageRawData(20,MAX21105_ACC_X_H, &x, &y, &z);
        axm1g_ = (s16)x;
        uart_Printf(&huart1,"axm1g = %d\r\n", x);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        }

    bdone = 0;
	uart_ClearFifo(&huart1);
    while (!bdone) {
        uart_Printf(&huart1, "Orient board Y axis up\r\n");
        delayMs(1000);
        max_GetAverageRawData(20, MAX21105_ACC_X_H,&x, &y, &z);
        ayp1g_ = (s16)y;
        uart_Printf(&huart1,"ayp1g = %d\r\n", y);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        }

    bdone = 0;
	uart_ClearFifo(&huart1);
    while (!bdone) {
        uart_Printf(&huart1, "Orient board Y axis down\r\n");
        delayMs(1000);
        max_GetAverageRawData(20, MAX21105_ACC_X_H,&x, &y, &z);
        aym1g_ = (s16)y;
        uart_Printf(&huart1,"aym1g = %d\r\n", y);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        }

    uart_Printf(&huart1,"Accelerometer Calibration Data\r\n");

    uart_Printf(&huart1,"ax0g = %d\r\n", (int)ax0g_);
    uart_Printf(&huart1,"ay0g = %d\r\n", (int)ay0g_);
    uart_Printf(&huart1,"axp1g = %d\r\n", (int)axp1g_);
    uart_Printf(&huart1,"axm1g = %d\r\n", (int)axm1g_);
    uart_Printf(&huart1,"ayp1g = %d\r\n", (int)ayp1g_);
    uart_Printf(&huart1,"aym1g = %d\r\n", (int)aym1g_);
    uart_Printf(&huart1,"azp1g = %d\r\n", (int)azp1g_);
    uart_Printf(&huart1,"azm1g = %d\r\n", (int)azm1g_);
    }

#endif
