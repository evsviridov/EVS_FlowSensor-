#ifndef __EVS_FlowSensor_H
#define __EVS_FlowSensor_H

#define EVS_FlowSensor_VERSION "1.0.3"

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class EVS_FlowSensor 
{
	public:
		// Constructors
		EVS_FlowSensor(void);
		EVS_FlowSensor(uint8_t pinHALL);
		EVS_FlowSensor(uint8_t pinHALL, uint8_t pinLED);
		// public properties

		// public methods
		void init(void);
		void init(uint8_t pinHALL, uint8_t pinLED);
		void reset(void);
		void resetCounterTotal(void);
		unsigned long getCounter(void);
		unsigned long getCounterTotal(void);
		bool getCurrentState(void);
		int checkPulse(void); // 0 - no pulse, 1 - falling slope, 2 - rising slope
		bool isTimeElapsed(void);
		void setLed(bool state);
		int addPulseCounter(int addition);
		int addPulseCounter(void);
		float getFrequency(void);
		float getFlowRate(void);
		void setFreqMeasurePeriod(long freqMeasurePeriod);
		String getLogString(int mode);
		
	private:
		uint8_t _pinHALL, _pinLED=0xFF;
		float _frequency, _flowRate;
		float _coef_K=7.8, _coef_B=0.84; // Hz(FlowRate[l/min]) equation is F = coef_K * Q[l/min] - coef_B;
		bool _currentState, _previousState;
		unsigned long _counter=0, _counterTotal=0, _counterPulseHi, _counterPulseLo;
		unsigned long _measureTime=0, _resetTime, _lengthHi, _lengthLo;
		long _freqMeasurePeriod=1000000L;
		// private methods
		bool _measureHall(void);
		void _setLed(bool state);
};


#endif
