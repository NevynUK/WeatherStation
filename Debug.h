//
//  Header for the static debugger class.
//
//  MIT License
//  
//  Copyright(c) 2016 Mark Stevens
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//  
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

