#ifndef AUDIO_H_
#define AUDIO_H_

// climb audio beep period and "duty cycle"
typedef struct BEEP_ {
    int periodTicks;
    int endTick;
} BEEP;


#define VARIO_MAX_CPS         1000

// more audio discrimination for climbrates under this threshold
#define VARIO_XOVER_CPS       300

// change these parameters based on the frequency bandwidth of the speaker

#define VARIO_MAX_FREQHZ      4000
#define VARIO_XOVER_FREQHZ    2000
#define VARIO_MIN_FREQHZ      200

#define VARIO_SINK_FREQHZ     400
#define VARIO_TICK_FREQHZ     200

#define VARIO_STATE_SINK    	11
#define VARIO_STATE_QUIET   	22
#define VARIO_STATE_LIFTY_AIR	33
#define VARIO_STATE_CLIMB   	44

#define CLIMB_DISCRIMINATION_THRESHOLD 25

#define CLIMB_THRESHOLD     50
#define SINK_THRESHOLD      -100
#define ZERO_THRESHOLD	    5


void audio_Config(void);
void audio_Beep(int nFreqHz, int nMsecs);
void audio_VarioBeep(int nCps);
void audio_IndicateFault(int nFrequencyHz);
void audio_GenerateTone(int nFrequencyHz, int ms);

#endif
