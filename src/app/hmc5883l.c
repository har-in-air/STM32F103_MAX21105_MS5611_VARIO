#include "common.h"
#include "board.h"
#include "hmc5883l.h"
#include "i2c.h"
#include "uart.h"

static float mxScale_;
static float myScale_;
static float mzScale_;
static s16 mxBias_;
static s16 myBias_;
static s16 mzBias_;


float mxned,myned,mzned;


void hmc5883L_Config(void) {
    //valid for +/-1.3Gauss FS
    mxScale_ = 1.0/475.0;
    myScale_ = 1.0/449.0;
    mzScale_ = 1.0/410.0;

    mxBias_ = 110;
    myBias_ = -203;
    mzBias_ = 6;
    
    delayMs(5);
    // 8 sample internal averaging, 75Hz sample rate, normal measurement flow
    i2c_XmtByte(I2C_ID_HMC5883L, HMC5883L_CONFIG_A,0x78); 
    // set range +/- 1.3Gauss
    i2c_XmtByte(I2C_ID_HMC5883L, HMC5883L_CONFIG_B,0x20);
    // set range +/- 1.9Gauss
    //i2c_XmtByte(I2C_ID_HMC5883L, HMC5883L_CONFIG_B,0x40);
    // set normal mode
    i2c_XmtByte(I2C_ID_HMC5883L, HMC5883L_MODE,0x00);
    }

int hmc5883L_IsDataReady(void) {
    u08 status = i2c_RcvByte(I2C_ID_HMC5883L, HMC5883L_STATUS);
    return (status & 0x01)  ? 1 : 0;
    }

// We are using the sensor board upside down, the interface connector facing "forward"
// and with standard N-E-D aviation right-handed coordinate system
// North = forward = x,  East = right = y, Down = z
// Mapping the sensor axes to the board NED axes we have the mapping
// mxned = mx
// myned = my
// mzned = mz
// we don't need the values in tesla, just the vector magnitudes in the range [0,1]
void hmc5883L_GetValues(float* px, float* py, float* pz) {
    s16 mx, my, mz;
    hmc5883L_GetRawData(&mx, &my, &mz);
    *px = (float)(mx - mxBias_) * mxScale_;
    *py = (float)(my - myBias_) * myScale_;
    *pz = (float)(mz - mzBias_) * mzScale_;
    }

u32 hmc5883L_GetID(void) {
    u32 id = 0;
    id = (u32) i2c_RcvByte(I2C_ID_HMC5883L, HMC5883L_IDA);
    id <<= 8;
    id |= (u32)i2c_RcvByte(I2C_ID_HMC5883L,HMC5883L_IDB);
    id <<= 8;
    id |= (u32)i2c_RcvByte(I2C_ID_HMC5883L,HMC5883L_IDC);
    return id;
	}

void hmc5883L_GetRawData(s16* px, s16* py, s16* pz) {
    u08 buf[6];
    i2c_RcvBuf(I2C_ID_HMC5883L, HMC5883L_DATA,6,buf);
    *px = (s16)((((u16)buf[0]) << 8) | (u16)buf[1]); // NOTE : data is MSB first !!
    *pz = (s16)((((u16)buf[2]) << 8) | (u16)buf[3]); // NOTE : order is x,z,y!!
    *py = (s16)((((u16)buf[4]) << 8) | (u16)buf[5]);
    }

#ifdef HMC5883L_CALIBRATE
	
// sensitivity = (max - min)/2, scale = 1/sensitivity
// bias = (max+min)/2

void hmc5883L_Calibrate(void) {
    s16 mx,my,mz;
    s16 mxmax = -32767;
    s16 mymax = -32767;
    s16 mzmax = -32767;
    s16 mxmin = 32767;
    s16 mymin = 32767;
    s16 mzmin = 32767;

	uart_Printf(&huart1, "\r\nHMC5883L Calibration : Wave unit around in 3D figure of 8 slowly ");
    for (int cnt = 0; cnt < 5000; cnt++) {
        hmc5883L_GetRawData(&mx,&my,&mz);
        if (mx > mxmax) mxmax = mx;
        if (my > mymax) mymax = my;
        if (mz > mzmax) mzmax = mz;
        if (mx < mxmin) mxmin = mx;
        if (my < mymin) mymin = my;
        if (mz < mzmin) mzmin = mz;
        delayMs(14); // 1/75Hz = 13.3ms sample interval
		if ((cnt%100) == 0) uart_Printf(&huart1, ".");
        }
    uart_Printf(&huart1,"\r\nmxmax %d  mxmin %d mxsens %d mxbias %d\r\n",(int)mxmax,(int)mxmin, ((int)mxmax-(int)mxmin)/2, (int)(mxmax+mxmin)/2); 
    uart_Printf(&huart1,"mymax %d  mymin %d mysens %d mybias %d\r\n",(int)mymax,(int)mymin, ((int)mymax-(int)mymin)/2, (int)(mymax+mymin)/2);
    uart_Printf(&huart1,"mzmax %d  mzmin %d mzsens %d mzbias %d\r\n",(int)mzmax,(int)mzmin, ((int)mzmax-(int)mzmin)/2, (int)(mzmax+mzmin)/2); 
    }

#endif