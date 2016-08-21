// 
//  Static class holding the methods required to support the debugging on
//  the Oak microcontroller.
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
#include "Debug.h"

//
//  Initialise the static data members.
//
DS3234RealTimeClock *Debugger::rtc = NULL;

//
//  Default constructor.
//
Debugger::Debugger()
{
    //rtc = NULL;
}

//
//  Output a diagnostic message if debugging is turned on.
//
void Debugger::DebugMessage(String message)
{
    //    UDPDebug.beginPacket(udpDestination, UDP_DEBUG_PORT);
    //    UDPDebug.write(message.c_str());
    //    UDPDebug.endPacket();
    if (rtc != NULL)
    {
        Serial.print(rtc->DateTimeString(rtc->GetDateTime()));
        Serial.print(": ");
    }
    Serial.println(message);
}

//
//  Output an array of bytes preceeded by a message.
//
void Debugger::DebugMessage(String message, uint8_t *byteArray, int byteArraySize)
{
    String messageAndData;
    char number[5];

    messageAndData = message + ": [";
    for (int index = 0; index < byteArraySize; index++)
    {
        itoa((int) byteArray[index], number, 16);
        messageAndData += number;
        if (index != (byteArraySize - 1))
        {
            messageAndData += ", ";
        }
    }
    messageAndData += "]";
    DebugMessage(messageAndData);
}
//
//  Convert a float to a string for debugging.
//
char *Debugger::FloatToAscii(char *buffer, double number, int precision)
{
    long p[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

    char *ret = buffer;
    long integer = (long) number;
    itoa(integer, buffer, 10);
    while (*buffer != '\0')
    {
        buffer++;
    }
    if (precision != 0)
    {
        *buffer++ = '.';
        long decimal = abs((long) ((number - integer) * p[precision]));
        itoa(decimal, buffer, 10);
    }
    return ret;
}

//
//  Log the luminosity data to the debug stream.
//
void Debugger::LogLuminosityData(double lux)
{
    char number[20];
    String message;

    message = "Lux: ";
    message += FloatToAscii(number, lux, 2);
    DebugMessage(message);
}

//
//  Log the data from the temperature, pressure and humidity sensor.
//
void Debugger::LogTemperatureHumidityAndPressureData(float temperature, float humidity, float pressure)
{
    char number[20];
    String message;

    message = "Temperature: ";
    message += FloatToAscii(number, temperature, 2);
    message += " C";
    DebugMessage(message);
    message = "Humidity: ";
    message += FloatToAscii(number, humidity, 2);
    message += "%";
    DebugMessage(message);
    message = "Pressure: ";
    message += FloatToAscii(number, pressure / 100, 0);
    message += " hPa";
    DebugMessage(message);
}

//
//  Log the reading from the ultraviolet light sensor.
//
void Debugger::LogUltravioletData(float uv)
{
    char num[20];
    String message;

    message = "Ultraviolet light: ";
    message += itoa(uv, num, 10);
    DebugMessage(message);
}

//
//  Log the ground temperature.
//
void Debugger::LogGroundTemperature(float temperature)
{
    String message;
    char number[20];

    message = "Ground temperature: ";
    message += FloatToAscii(number, temperature, 2);
    DebugMessage(message);
}

//
//  Log the rainfall and the total rainfall today.
//
void Debugger::LogRainfall(float rainfall, float rainfallToday)
{
    String message;
    char number[20];

    message = "Last rainfall: ";
    message += FloatToAscii(number, rainfall, 2);
    message += "mm";
    DebugMessage(message);
    message = "Total rainfall today: ";
    message += FloatToAscii(number, rainfallToday, 2);
    message += "mm";
    DebugMessage(message);
}

//
//  Store a reference to the real time clock for recording the current time on debug messages.
//
void Debugger::AttachRTC(DS3234RealTimeClock *r)
{
    rtc = r;
}