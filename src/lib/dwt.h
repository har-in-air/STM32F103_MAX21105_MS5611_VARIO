#ifndef DWT_H_
#define DWT_H_

extern uint32_t dwtCPUTicksPerUs;

void        dwt_Init(void);
void        dwt_DelayUs(uint32_t uSecs); // busy delay
void        dwt_SetMarker(void);  // set marker  for ElapsedTimeUs
uint32_t    dwt_ElapsedTimeUs(void); // elapsed time in microseconds after call to SetMarker
uint32_t    dwt_IntervalUs(uint32_t before, uint32_t after);
float       dwt_IntervalSecs(uint32_t before, uint32_t after);

#endif
