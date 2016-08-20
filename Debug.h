// Debug.h

#ifndef _DEBUG_h
#define _DEBUG_h

#include "arduino.h"
#include "DS3234.h"

class Debugger
{
    private:
        static DS3234RealTimeClock *rtc;
        Debugger();

    public:
        static void DebugMessage(String);
        static void DebugMessage(String, uint8_t *, int);
        static char *FloatToAscii(char *, double , int);
        static void LogLuminosityData(double);
        static void LogTemperatureHumidityAndPressureData(float, float, float);
        static void LogUltravioletData(float);
        static void LogGroundTemperature(float);
        static void LogRainfall(float, float);
        static void AttachRTC(DS3234RealTimeClock *r);
};

#endif

