#ifndef CONFIG_H_
#define CONFIG_H_

// configure uart2 receive irq handler for different applications

#define UART_STANDARD

#define MS5611_Z_VARIANCE 	    400.0f
#define IMU_ACCEL_VARIANCE      200.0f
#define IMU_ACCELBIAS_VARIANCE   1.0f


#define MODE_VARIO      0
#define MODE_YPR        1
#define MODE_CALIB      2



#endif
