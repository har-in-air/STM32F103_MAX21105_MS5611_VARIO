# STM32F103 altimeter/variometer with LCD display

## Features
Basic altimeter/variometer  that fuses accelerometer and barometric sensor data to minimize response lag.

## Hardware
1.STM32F103C8T6 microcontroller
2.MAX21105 accelerometer+gyroscope
3.MS5611 barometric pressure sensor
4.Salvaged 80x48 lcd front display from old motorola clamshell phone

I am using a homebrew imu breakout board with MAX21105+MS5611+HMC5883L. The magnetometer is not required for the vario functionality. Using it gives you absolute yaw (compass heading) information.  

I salvaged a tiny 80x48 pixel LCD display from an old Motorola clamshell phone and used this for altitude/climbrate display. It has an I2C interface.

I used separate breakout boards for the micro-controller and the sensors that I'd designed earlier. So there is no 
PCB layout for the unified schematic. The battery is a salvaged 3.7V 500mAH lipoly battery. Also added a USB to 
lipoly charger board.  

## Software development environment
Ubuntu 18.04LTS, gcc-arm toolchain, openocd, stlink v2 clone for flashing. STM32CubeMx for generating the low level hardware driver files and headers, and startup code.

## Issues
1.OpenOCD does not sync with this particular micro-controller unless the reset button is pushed. No such issue with 
GD32F103RCT6  (which is a cheap clone of the STM32F103RCT6). Go figure.
2.Uart serial printf with floats does not work. 
