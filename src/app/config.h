#ifndef CONFIG_H_
#define CONFIG_H_

// configure uart2 receive irq handler for different applications

#define UART_STANDARD

#define MS5611_Z_VARIANCE 	      400.0f
#define IMU_ACCEL_VARIANCE       50000.0f
#define IMU_ACCELBIAS_VARIANCE   1.0f

//#define IMU_MAHONEY
//#define IMU_MADGWICK

#define DT_MEAS  0.00236f

#define MODE_VARIO      0
#define MODE_YPR        1
#define MODE_RESERVED   2

#define CFG_LCD_DISPLAY
//#define CFG_USE_MAGNETOMETER


#endif
