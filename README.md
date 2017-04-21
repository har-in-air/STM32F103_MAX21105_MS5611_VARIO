# imu-vario
Basic altimeter/variometer  that fuses accelerometer and barometric sensor data to minimize response lag.

Uses MAX21105 accelerometer+gyroscope, HMC5883L magnetometer, MS5611 barometric pressure sensor and STM32F103C8T6 cortex m3 micro-controller. The magnetometer is not required for the vario functionality. Using it gives you absolute yaw (compass heading) information.  I salvaged a tiny 80x48 pixel LCD display from an old Motorola clamshell phone and used this for altitude/climbrate display.

I used separate breakout boards for the micro-controller and the sensors that I'd designed earlier. So there is no PCB layout for the unified schematic. The battery is a 3.7V 500mAH lipoly battery that i salvaged. Also added a USB to lipoly charger board.  Current draw now is about 25mA with the microcontroller cpu running at 24MHz. Could likely be optimized further.

There are compile options (#defines in the header file config.h) for including the magnetometer and the LCD display. So you can build  a bare-bones audio-only variometer that is very fast and precise.
