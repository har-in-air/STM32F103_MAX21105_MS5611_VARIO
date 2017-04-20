#ifndef MS5611_H_
#define MS5611_H_

//#define MS5611_TEST

#define MS5611_SAMPLE_PERIOD_MS         12

#define MS5611_READ_TEMPERATURE 		11
#define MS5611_READ_PRESSURE			22

// max conversion time with max OSR (4096) =  9.04mS
#define SENSOR_TEMPERATURE_TIMEOUT      15
#define SENSOR_PRESSURE_TIMEOUT         15

extern volatile float paSample_;
extern volatile float zCmSample_;
extern float zCmAvg_;

void MS5611_TriggerPressureSample(void);
void MS5611_TriggerTemperatureSample(void);
u32  MS5611_ReadPressureSample(void);
void MS5611_AveragedSample(int nSamples);
u32  MS5611_ReadTemperatureSample(void);
void MS5611_CalculateTemperatureCx10(void);
float MS5611_CalculatePressurePa(void);
void MS5611_CalculateSensorNoisePa(void);
void MS5611_Config(void);

void MS5611_ReadCoefficients(void);
float MS5611_Pa2Cm(float pa);
void MS5611_Test(int nSamples);

int  MS5611_SampleStateMachine(void);
void MS5611_InitializeSampleStateMachine(void);
	
#endif // MS5611_H_
