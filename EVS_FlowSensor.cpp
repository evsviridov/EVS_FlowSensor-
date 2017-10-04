#include <EVS_FlowSensor.h>

//#define DEBUG_EVS_FlowSensor

#ifdef DEBUG_EVS_FlowSensor 
	#define __sdebug(s) {Serial.println((String)s);}
#else
	#define __sdebug(s) 
#endif

EVS_FlowSensor::EVS_FlowSensor(void) 
{
	__sdebug("EVS_FlowSensor(void)");
	_pinHALL = -1;
	_pinLED = -1;
}

EVS_FlowSensor::EVS_FlowSensor(uint8_t pinHALL) 
{
	__sdebug("EVS_FlowSensor(" + pinHALL + ")");
	_pinHALL = pinHALL;
	_pinLED = -1;
}

EVS_FlowSensor::EVS_FlowSensor(uint8_t pinHALL, uint8_t pinLED) 
{
	__sdebug("EVS_FlowSensor(pinHALL=" + pinHALL + " pinLED=" + pinLED+")");
	_pinHALL=pinHALL;
	_pinLED = pinLED;
}


void EVS_FlowSensor::init(void)
{
	__sdebug("init()");
	pinMode(_pinHALL, INPUT_PULLUP);
	if(_pinLED != -1)
		pinMode(_pinLED, OUTPUT);
	_counterTotal=0;
	reset();
}
void EVS_FlowSensor::init(uint8_t pinHALL, uint8_t pinLED)
{
	__sdebug("init(pinHALL=" + pinHALL + " pinLED=" + pinLED + ")");
	_pinHALL = pinHALL;
	_pinLED = pinLED;
	init();
}

unsigned long EVS_FlowSensor::getCounter(void) {
	__sdebug("getCounter()");
	return _counter;
}
unsigned long EVS_FlowSensor::getCounterTotal(void) {
	__sdebug("getCounterTotal()");
	return _counterTotal;
}

int EVS_FlowSensor::addPulseCounter(int addition)
{
	__sdebug((String) "addPulseCounter()"+addition + ")");
	_counter += addition;
	_counterTotal += addition;
	return _counter;
}

int EVS_FlowSensor::addPulseCounter(void)
{
	++_counter;
	++_counterTotal;
	__sdebug((String) "addPulseCounter()" + _counter);
	return _counter;
}

bool EVS_FlowSensor::getCurrentState(void)
{
	_previousState = _currentState;
	_currentState = _measureHall();
	return _currentState;
}

int EVS_FlowSensor::checkPulse(void)
{
	static unsigned long _startHi, _startLo;
	getCurrentState();
	if (_previousState == _currentState) return 0;
	
	if (_currentState)
	{
		if(_counterPulseHi == 0) _startHi=_measureTime;
		++_counterPulseHi;
		_lengthHi = _measureTime - _startHi;
	}
	else
	{
		if(_counterPulseLo == 0) _startLo=_measureTime;
		++_counterPulseLo;
		_lengthLo = _measureTime - _startHi;

	}
	return (int)_currentState + 1;
	
}

void EVS_FlowSensor::setLed(bool state)
{
	_setLed(state);
}

void EVS_FlowSensor::reset(void)
{
/*		
  		bool _currentState, _previousState;
		unsigned int _counter;
		unsigned long _measureTime, _lastPulseHiTime, _lastPulseLowTime;
*/
	__sdebug("reset()");
	_counterPulseHi = 0;
	_counterPulseLo = 0;
	_counter = 0;
	_previousState = getCurrentState();
	_resetTime = _measureTime;
}
void EVS_FlowSensor::resetCounterTotal(void)
{
	reset();
	_counterTotal=0;
}

void EVS_FlowSensor::_setLed(bool state)
{
	if(_pinLED !=0xFF)
		digitalWrite(_pinLED, state);
}

bool EVS_FlowSensor::_measureHall(void)
{
	_measureTime = micros();
	return digitalRead(_pinHALL);
}

bool EVS_FlowSensor::isTimeElapsed(void)
{
	return (_measureTime-_resetTime) >= _freqMeasurePeriod;
}

float EVS_FlowSensor::getFrequency(void)
{
	static long _periodInMicros = -1L;
	__sdebug("getFreq. Length=" + _lengthHi + " _counterPulseHi=" + _counterPulseHi );

	if (_counterPulseHi > 1)
	{
		_periodInMicros = _lengthHi / (_counterPulseHi -1);
		_frequency = 1.0E6/float(_periodInMicros);
	}
	else
	{
		_frequency = 0.0;
	}
	return _frequency;
}

float EVS_FlowSensor::getFlowRate(void)
{
	getFrequency();	
	_flowRate = ((_frequency > 0.0) ? (_frequency +_coef_B) : _frequency) / _coef_K;
	return _flowRate;
}
String EVS_FlowSensor::getLogString(int mode)
{
	static char logString[16];
	switch(mode)
	{
		case -1 : 	return (String) "EVS_FlowSensor" + EVS_FlowSensor_VERSION; break;
		case 0  :	return (String) _flowRate; break;
		case 1  :	return (String) _frequency; break;
		default :	return "Unknown mode";
	}
}

	
void EVS_FlowSensor::setFreqMeasurePeriod(long freqMeasurePeriod)
{
	_freqMeasurePeriod = freqMeasurePeriod;
}

#ifdef DEBUG_EVS 
#undef __sdebug(s)
#endif

#ifdef DEBUG_EVS_FlowSensor
#undef DEBUG_EVS_FlowSensor
#endif
