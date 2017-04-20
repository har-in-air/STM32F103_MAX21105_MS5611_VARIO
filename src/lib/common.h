#ifndef COMMON_H_
#define COMMON_H_

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"

#define boolean int
#define bool    int
#define true 	1
#define false   0
#define TRUE    1
#define FALSE   0

typedef unsigned long long  u64;
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u08;

typedef signed long long   s64;
typedef signed long    s32;
typedef signed short   s16;
typedef signed char    s08;

typedef union _un16 {
  u16 u;
  s16 i;
  u08 b[2];
} un16;

typedef union _un32 {
  u32 u;
  s32 i;
  u16 w[2];
  u08 b[4];
} un32;

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE    0
#endif

#define OUT	1
#define IN	0


#define MIN(x,y)                 ((x) < (y) ? (x) : (y))
#define MAX(x,y)                 ((x) > (y) ? (x) : (y))
#define ABS(x)                 ((x) < 0 ? -(x) : (x))
#define CLAMP(x,mn,mx)       {if (x <= (mn)) x = (mn); else if (x >= (mx)) x = (mx);}
#define CORE(x,t)              {if (ABS(x) <= (t)) x = 0;}
#define MCORE(x,t)              {if (x > (t)) x -= (t); else if (x < -(t)) x += (t); else x = 0;}
#define CORRECT(x,mx,mn)  		(((float)(x-mn)/(float)(mx-mn)) - 0.5f)

#define M_PI 		  3.1415927f
#define RAD2DEG(r)   ((r)*57.29577951f)
#define DEG2RAD(d)   ((d)*0.017453292f)

//#define TWO_PI               6.283185
#define _180_DIV_PI         57.295779f
#define PI_DIV_180          0.017453292f

#define NOP()   {asm volatile("mov r0,r0");}
#define DINT()  {asm volatile ("CPSID i");}
#define EINT()  {asm volatile ("CPSIE i");}

extern char gszBuf[];


#endif // COMMON_H_
