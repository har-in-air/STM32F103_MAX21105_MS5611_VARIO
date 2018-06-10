#include "common.h"
#include "config.h" 
#include "board.h"
#include "stm32f1xx.h"
#include "gpio.h"
#include "piezo.h"
#include "audio.h"



int gnDiscrimThreshold =  25;
int gnBeepPeriodTicks = 0;
int gnBeepEndTick = 0;
int gnBeepCps = 0;
int gnVarioCps = 0;
int gnFreqHz  = 0;

int gnSinkToneCps       = SINK_THRESHOLD_DEFAULT;
int gnClimbToneCps      = CLIMB_THRESHOLD_DEFAULT;
int gnLiftyAirToneCps   = ZERO_THRESHOLD_DEFAULT;
int gnVarioState;

const int gnOffScaleHiTone[10]= {2000,1000,2000,1000,2000,1000,2000,1000,2000,1000};
const int gnOffScaleLoTone[10]= {650,600,550,500,450,400,350,300,250,200};


// table for beep duration and repeat rate based on vertical speed
const BEEP gBeepTbl[10] = {
// repetition rate saturates at 8m/s
{13,4},
{11,4},
{9,4},
{8,4},
{7,4},
{6,4},
{5,3},
{4,1},
{3,1},
{3,1},
};


void audio_Config(void) {
    gnVarioState = VARIO_STATE_QUIET;
    piezo_Config();
    }

void audio_VarioBeep(int nCps) {
   static int nFreqHz,nTick;
   int nRange;
   int newFreqHz = 0;

   if (	(gnBeepPeriodTicks <= 0) || 
		((nTick > gnBeepPeriodTicks/2) && (ABS(nCps-gnVarioCps) > gnDiscrimThreshold)) || 
		((nCps > gnClimbToneCps) && (gnVarioCps <= gnClimbToneCps)) ||
		((nCps > gnLiftyAirToneCps) && (gnVarioCps <= gnLiftyAirToneCps))		
		) {
      gnVarioCps = nCps;

      // if sinking much faster than glider sink rate, generate continuous tone alarm
      if (gnVarioCps <= gnSinkToneCps) {
         gnVarioState = VARIO_STATE_SINK;
         gnBeepCps = gnVarioCps;
         if (gnBeepCps < -VARIO_MAX_CPS) {
			gnBeepCps = -VARIO_MAX_CPS;
			}
		 nTick = 0;
		 if (gnBeepCps == -VARIO_MAX_CPS) {
    		gnBeepPeriodTicks = 10;
    		gnBeepEndTick = 10;
    		newFreqHz = gnOffScaleLoTone[0];
    		nFreqHz = newFreqHz;
            piezo_SetFrequency(nFreqHz);
            }
         else {
            gnBeepPeriodTicks = 30;
            gnBeepEndTick  = 25;
            newFreqHz = VARIO_SINK_FREQHZ + ((gnSinkToneCps-gnBeepCps)*(VARIO_MAX_FREQHZ - VARIO_SINK_FREQHZ))/(VARIO_MAX_CPS+gnSinkToneCps);
            CLAMP(newFreqHz,VARIO_MIN_FREQHZ,VARIO_MAX_FREQHZ);
            nFreqHz = newFreqHz;
            piezo_SetFrequency(nFreqHz);
         	}
        }
      //if climbing, generate beeps
      else {
         if (gnVarioCps >= gnClimbToneCps) {
            gnVarioState = VARIO_STATE_CLIMB;
            gnBeepCps = gnVarioCps;
            if (gnBeepCps > VARIO_MAX_CPS) {
				gnBeepCps = VARIO_MAX_CPS;
				}
		    nTick = 0;
			if (gnBeepCps == VARIO_MAX_CPS) {
    		    gnBeepPeriodTicks = 10;
    		    gnBeepEndTick = 10;
    		    newFreqHz =gnOffScaleHiTone[0];
    		    nFreqHz = newFreqHz;
                piezo_SetFrequency(nFreqHz);
                }
            else {
           	    nRange = gnBeepCps/100;
           	    if (nRange > 9) nRange = 9;
           	    gnBeepPeriodTicks = gBeepTbl[nRange].periodTicks;
           	    gnBeepEndTick = gBeepTbl[nRange].endTick;
         	    if (gnBeepCps > VARIO_XOVER_CPS) {
                    newFreqHz = VARIO_XOVER_FREQHZ + ((gnBeepCps - VARIO_XOVER_CPS)*(VARIO_MAX_FREQHZ - VARIO_XOVER_FREQHZ))/(VARIO_MAX_CPS - VARIO_XOVER_CPS);
                    }
                else {
                    newFreqHz = VARIO_MIN_FREQHZ + (gnBeepCps*(VARIO_XOVER_FREQHZ - VARIO_MIN_FREQHZ))/VARIO_XOVER_CPS;
                    }
                CLAMP(newFreqHz,VARIO_MIN_FREQHZ,VARIO_MAX_FREQHZ);
                nFreqHz = newFreqHz;
                piezo_SetFrequency(nFreqHz);
                }
            }
         else   // in "lifty-air" band, indicate with different sound
         if (gnVarioCps >= gnLiftyAirToneCps) {
            gnVarioState = VARIO_STATE_LIFTY_AIR;
    		gnBeepCps = gnVarioCps;
    		nTick = 0;
    		gnBeepPeriodTicks = 30;
    		gnBeepEndTick = 2;
    		newFreqHz = VARIO_TICK_FREQHZ + ((gnBeepCps - gnLiftyAirToneCps)*400)/(gnClimbToneCps-gnLiftyAirToneCps);
            CLAMP(newFreqHz,VARIO_MIN_FREQHZ,VARIO_MAX_FREQHZ);
            nFreqHz = newFreqHz;
    		piezo_SetFrequency(nFreqHz);  // higher frequency as you approach climb threshold
            }

      // not sinking enough to trigger alarm,  be quiet
         else{
            gnVarioState = VARIO_STATE_QUIET;
            nTick = 0;
            gnBeepPeriodTicks = 0;
            gnBeepEndTick  = 0;
            newFreqHz = nFreqHz = 0;
            piezo_SetFrequency(nFreqHz);
            }
         }
      }
   else{
      nTick++;
      gnBeepPeriodTicks--;
	  newFreqHz = nFreqHz;
      if (nTick >= gnBeepEndTick){ // shut off tone
         newFreqHz = 0;
         }
	  else
	  if (gnBeepCps == VARIO_MAX_CPS) {
    	  newFreqHz = gnOffScaleHiTone[nTick];
            }
      else
	  if (gnBeepCps == -VARIO_MAX_CPS) {
    	  newFreqHz = gnOffScaleLoTone[nTick];
          }
		else
	 if (gnVarioState == VARIO_STATE_SINK) {
    	 newFreqHz = nFreqHz - 10;
         }
     if (newFreqHz != nFreqHz) {
         nFreqHz = newFreqHz;
         piezo_SetFrequency(nFreqHz);
         }
	  }
   }


/// Generates audio alarm for a fault event as 4 tones
/// with the same frequency
/// @param nFrequencyHz alarm tone frequency.
void audio_IndicateFault(int nFrequencyHz) {
    piezo_SetFrequency(nFrequencyHz);
    delayMs(300);
    piezo_SetFrequency(0);
    delayMs(100);
    piezo_SetFrequency(nFrequencyHz);
    delayMs(300);
    piezo_SetFrequency(0);
    delayMs(100);
    piezo_SetFrequency(nFrequencyHz);
    delayMs(300);
    piezo_SetFrequency(0);
    delayMs(100);
    piezo_SetFrequency(nFrequencyHz);
    delayMs(300);
    piezo_SetFrequency(0);
    }
