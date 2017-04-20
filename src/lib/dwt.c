#include "common.h"
#include "board.h"
#include "stm32f1xx_hal.h"
#include "dwt.h"


uint32_t dwtCPUTicksPerUs;
static uint32_t dwtMarker;

void dwt_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= 1;
    dwtCPUTicksPerUs = SystemCoreClock/1000000;
    }

void dwt_DelayUs(uint32_t us) {
	volatile uint32_t waitCycCnt = (dwtCPUTicksPerUs * us ) + (uint32_t)DWT->CYCCNT;
	do  {} while (DWT->CYCCNT  < waitCycCnt);
}


uint32_t  dwt_IntervalUs(uint32_t before, uint32_t after) {
	return  (before <= after ?
    ((after - before)+dwtCPUTicksPerUs/2)/dwtCPUTicksPerUs :
    (after + (0xFFFFFFFF - before) + dwtCPUTicksPerUs/2)/dwtCPUTicksPerUs);
}

float  dwt_IntervalSecs(uint32_t before, uint32_t after) {
	return  (before <= after ?
    (float)(after - before)/(float)SystemCoreClock :
    (float)(after + (0xFFFFFFFF - before))/(float)SystemCoreClock);
}

void dwt_SetMarker(void) {
	dwtMarker = (uint32_t)DWT->CYCCNT;
}

uint32_t dwt_ElapsedTimeUs(void) {
	uint32_t now = (uint32_t)DWT->CYCCNT;
	return  (dwtMarker <= now ?
    ((now - dwtMarker)+dwtCPUTicksPerUs/2)/dwtCPUTicksPerUs :
    (now + (0xFFFFFFFF - dwtMarker) + dwtCPUTicksPerUs/2)/dwtCPUTicksPerUs);
}
