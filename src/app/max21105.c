#include "common.h"
#include "board.h"
#include "i2c.h"
#include "uart.h"
#include "max21105.h"

#define max_Write8(addr, val) 	i2c_XmtByte(I2C_ID_MAX21105,addr,val)
#define max_Read8(addr) 		i2c_RcvByte(I2C_ID_MAX21105,addr)
#define max_ReadBuf(addr,burst,numBytes,pBuf) i2c_RcvBuf(I2C_ID_MAX21105,addr,numBytes,pBuf)

s16 gxBias;
s16 gyBias;
s16 gzBias;
static float gyroScale_;

s16 axBias;
s16 ayBias;
s16 azBias;
static float aScale_;

float gxned,gyned,gzned; // gyro rotation rates in NED coordinates, in degrees/second
float axned,ayned,azned; // accelerometer values in NED coordinates, in milli-Gs

// gyro sensitivity = counts / degree-per-second
// accel sensitivity = counts / milli-G
// scale = 1 / sensitivity
void max_InitCalibrationData(void) {

	// valid for FS = +/- 4g
    axBias = 142;
    ayBias = -545;
    azBias = 220;
    aScale_ = 1.0f/MAX21105_4G_SENSITIVITY;
    
    // valid for FS = 1000dps
    gxBias = -14; 
    gyBias = -11; 
    gzBias = -30; 
    
    gyroScale_  = 1.0/MAX21105_1000DPS_SENSITIVITY; 
    }
	

u08 max_GetID(void) {
  return max_Read8(MAX21105_WHOAMI);
  }	


void max_Configure(void) {
    max_InitCalibrationData();
    max_Write8(MAX21105_BANK_SELECT, 0); // select bank 0
    max_Write8(MAX21105_SET_PWR, 0); // power down
//    max_Write8(MAX21105_SNS_CFG_1, 0x2A); // selftest disabled,gyro bw = 100hz, gyro fs = 500dps 00:1010:10
    max_Write8(MAX21105_SNS_CFG_1, 0x31); // selftest disabled,gyro bw = 200hz, gyro fs = 1000dps 00:1100:01
//    max_Write8(MAX21105_SNS_CFG_2, 0x05); // rfu, normal fullscale, gyro hpf disabled, gyro odr = 200Hz 00:0:0:0101
    max_Write8(MAX21105_SNS_CFG_2, 0x04); // rfu, normal fullscale, gyro hpf disabled, gyro odr = 400Hz 00:0:0:0100
    //max_Write8(MAX21105_SNS_CFG_3, 0x00); //  (default) 
//    max_Write8(MAX21105_SET_ACC_PWR, 0xC0); // accel fs = +/-2G, selftest disabled, rfu 11:000:000
    max_Write8(MAX21105_SET_ACC_PWR, 0x80); // accel fs = +/-4G, selftest disabled, rfu 10:000:000
    max_Write8(MAX21105_ACC_CFG_1, 0x32); // hpf = odr/400, lpf= odr/3, acc odr = 400hz 00:11:0010
   // max_Write8(MAX21105_ACC_CFG_2, 0x40); // (default, acc data is low pass filtered)
   // max_Write8(MAX21105_MIF_CFG, 0x00); // I2C fast interface, big endian (default)
    max_Write8(MAX21105_SET_PU_PD_PAD, 0x29); // pulldown int1 and int2, disconnect slave i2c pullups
 //   max_Write8(MAX21105_SET_TEMP_DR, 0x01);//clear DRDY when all registers read, enable temp (default)
 
    max_Write8(MAX21105_BANK_SELECT, 1); // select bank 1
    max_Write8(MAX21105_INT_SRC_CFG, 0x20); // gyro data ready generates drdy interrupt
    max_Write8(MAX21105_INT_MSK, 0x80); // drdy interrupt on INT1
    max_Write8(MAX21105_INT_TM0_CFG, 0x00); // INT1 unlatched
    max_Write8(MAX21105_INT_CFG_2, 0x20); // INT1 enable
    
    max_Write8(MAX21105_BANK_SELECT, 0); // select bank 0
    max_Write8(MAX21105_SET_PWR, 0x78); // power mode = acc low-noise + gyro low-noise
    }
    

void max_GetAllChannelData(void) {
    u08 buf[14];
	s16 gx,gy,gz,ax,ay,az;
    max_ReadBuf(MAX21105_GYRO_X_H, AUTO_INCR, 14, buf);
    gx = (s16)((((u16)buf[0]) << 8) | (u16)buf[1]);
    gy = (s16)((((u16)buf[2]) << 8) | (u16)buf[3]);
    gz = (s16)((((u16)buf[4]) << 8) | (u16)buf[5]);
    
	ax = (s16)((((u16)buf[6]) << 8) | (u16)buf[7]);
    ay = (s16)((((u16)buf[8]) << 8) | (u16)buf[9]);
    az = (s16)((((u16)buf[10]) << 8) | (u16)buf[11]);
	
	max_GetGyroValues(gx, gy, gz, &gxned, &gyned, &gzned);
	max_GetAccelValues(ax, ay, az, &axned, &ayned, &azned);
	}




// We are using the sensor board upside down, the interface connector facing "forward"
// and with standard N-E-D aviation right-handed coordinate system
// North = forward = x,  East = right = y, Down = z
// Roll right +ve, Pitch up +ve, Yaw right +ve
// To map the gyro data to this convention, we need to do this :
// gx_ned = gyro_x, gy_ned = gyro_y, gz_ned = gyro_z
// returns rotation rates in degrees/second

void max_GetGyroValues(s16 gx, s16 gy, s16 gz, float* pgx, float* pgy, float* pgz) {
    *pgx = (float)(gx - gxBias) * gyroScale_;
    *pgy = (float)(gy - gyBias) * gyroScale_;
    *pgz = (float)(gz - gzBias) * gyroScale_;
    }



// We are using the sensor board upside down, the interface connector facing "forward"
// and with standard N-E-D aviation right-handed coordinate system
// North = forward = x,  East = right = y, Down = z
// To map the accerometer data to these coordinates, we need to do this :
// ax_ned = -acc_x, ay_ned = -acc_y, az_ned = -acc_z
// returns acceleration values in milliG

void max_GetAccelValues(s16 ax, s16 ay, s16 az, float* pax, float* pay, float* paz) {
    *pax = (float)(axBias - ax) * aScale_;   
    *pay = (float)(ayBias - ay) * aScale_;
    *paz = (float)(azBias - az) * aScale_;
    }




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
    
#define MAX_CALIB_SAMPLES   50

s16 gXBuf[MAX_CALIB_SAMPLES];
s16 gYBuf[MAX_CALIB_SAMPLES];
s16 gZBuf[MAX_CALIB_SAMPLES];


void max_CalibrateGyro(void) {
    int x,y,z,bdone;

    bdone = 0;
	uart_ClearFifo(&huart1);
    //while (!bdone) {
        uart_Printf(&huart1, "Do not disturb sensor during calibration\r\n");
        delayMs(1000);
        max_GetAverageRawData(50,MAX21105_GYRO_X_H, &x, &y, &z);
        gxBias = (s16)x;
        gyBias = (s16)y;
        gzBias = (s16)z;
        uart_Printf(&huart1,"gxBias = %d\r\n",x);
        uart_Printf(&huart1,"gyBias = %d\r\n",y);
        uart_Printf(&huart1,"gzBias = %d\r\n",z);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
        //}
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

//
// To get the acceleration in g's, multiply the (0g corrected) sensor output by the scale
// To get the acceleration in cm/s*s, multiply the g's by by 980cm/s*s


void max_CalibrateAccel(void) {
   int x,y,z,bdone;
   s16 az1g;
   bdone = 0;
 	uart_ClearFifo(&huart1);
   //while (!bdone) {
        uart_Printf(&huart1, "Orient board Z axis UP\r\n");
        delayMs(1000);
        max_GetAverageRawData(50,MAX21105_ACC_X_H, &x, &y, &z);
        axBias = (s16)x;
        ayBias = (s16)y;
        az1g = (s16)z;
        uart_Printf(&huart1,"axBias = %d\r\n",x);
        uart_Printf(&huart1,"azBias = %d\r\n",y);
        uart_Printf(&huart1,"azp1g = %d\r\n",z);
        uart_Printf(&huart1,"type any character if OK\r\n");
        bdone = uart_IsCharReady(&huart1,1000);
  //      }
    uart_Printf(&huart1,"Accelerometer Calibration Data\r\n");
    azBias = az1g > 0 ? az1g - (s16)(1000.0f*MAX21105_4G_SENSITIVITY) : az1g + (s16)(1000.0f*MAX21105_4G_SENSITIVITY);

    uart_Printf(&huart1,"axBias = %d\r\n", (int)axBias);
    uart_Printf(&huart1,"ayBias = %d\r\n", (int)ayBias);
    uart_Printf(&huart1,"azBias = %d\r\n", (int)azBias);
    }


#endif
