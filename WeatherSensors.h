//
//  Header for the class supporting the weather station sensors.
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
#ifndef __WEATHERSENSORS_H__
#define __WEATHERSENSORS_H__

#include <OneWire.h>
#include <Adafruit_BME280.h>
#include <SparkFunTSL2561.h>
#include "Debug.h"

class WeatherSensors
{
    //
    //  Interrupt Service Routine function pointer type.
    //
    typedef void (*ISRPointer)();

    public:
        WeatherSensors();
        ~WeatherSensors();
        void InitialiseSensors();
        void ReadAllSensors();
        //
        //  Ground temperature sensor.
        //
        float ReadGroundTemperatureSensor();
        float GetGroundTemperatureReading();
        //
        //  Ultraviolet light sensor.
        //
        float ReadUltravioletLightSensor();
        float GetUltravioletLightReading();
        float GetUltravioletLightStrength();
        //
        //  Luminosity sensor.
        //
        double ReadLuminositySensor();
        double GetLuminosityReading();
        //
        //  Air temperature, pressure and humidity.
        //
        void ReadTemperatureHumidityPressureSensor();
        float GetAirTemperature();
        float GetHumidity();
        float GetAirPressure();
        //
        //  Rain fall sensor.
        //
        void SetupRainfallSensor();
        void ReadRainfallSensor();
        float GetRainfall();
        float GetTotalRainfallToday();
        //
        //  Wind speed sensor.
        //
        void SeupWindSpeedSensor();
        float ReadWindSpeedSensor();
        float GetWindSpeed();
        void SetWindSpeedISR(ISRPointer);
        void HandleWindSpeedInterrupt();
        //
        //  Wind direciton sensor.
        //
        enum WindDirection
        {
            North, NorthNorthEast, NorthEast, EastNorthEast, East, EastSouthEast, SouthEast, SouthSouthEast,
            South, SouthSouthWest, SouthWest, WestSouthWest, West, WestNorthWest, NorthWest, NorthNorthWest
        };
        WindDirection ReadWindDirection();
        char *GetWindDirectionAsString();

    private:
        //
        //  Entry in the wind direction lookup table.
        //
        struct WindDirectionLookup
        {
            uint16_t midPoint;
            uint16_t reading;
            float angle;
            WindDirection direction;
            char *directionAsText;
        };
        //
        //  Private members not related to the sensors or their readings directly.
        //
        //
        //  Ground temperature variables.
        //
        OneWire *_groundSensor;
        double _groundTemperature = 0;
        byte _groundTemperatureSensorType = 0;
        byte _groundTemperatureSensorAddress[8];
        void SetupGroundTemperatureSensor();
        //
        //  Ultraviolet analog reading.
        //
        int _ultraviolet;
        const float _uvGradient = 0.12;             // Gradient of the UV curve from the data sheet.
        const int _maximumAnalogValue = 1023;
        const float _referenceVoltage = 3.3;
        const float _uvOffset = 1.025;              // Base reading from the sensor (observation).
        const uint8_t _ultravioletLightChannel = 0; // ADC channel connected to the UV sensor.
        const float _voltsPerDivision = 0.0001875;  // Volts per division in the ADS1115 ADC.
        //
        //  STM8S I2C Address and commands.
        //
        const uint8_t STM8SAddress = 0x48;
        const uint8_t I2CReset = 0x00;
        const uint8_t I2CReadSensors = 0x01;
        const uint8_t I2CGetSensorData = 0x02;
        const uint8_t I2CDataReady = 0x03;
        const uint8_t I2CResetRainFallCounter = 0x04;
        const uint8_t I2CBufferSize = 8;
        //
        //  Light sensor (luminosity).
        //
        SFE_TSL2561 _light;
        boolean _gain;          //  Gain setting, 0 = X1, 1 = X16;
        unsigned int _ms;       //  Integration ("shutter") time in milliseconds
        double _lux;            //  Luminosity in lux.
        boolean _good;          //  True if neither sensor is saturated
        void SetupLuminositySensor();
        String LuminositySensorErrorMessage(byte);
        //
        //  Create a Temperature, humidity and pressure sensor.
        //
        Adafruit_BME280 _bme;
        float _temperature = 0;
        float _pressure = 0;
        float _humidity = 0;
        void SetupTemperatureHumidityPressureSensor();
        //
        //  Rain fall sensor.
        //
        int _pluviometerPulseCount = 0;
        int _pluviometerPulseCountToday = 0;
        const int PIN_RAINFALL_RESET = 0;
        const float RAINFALL_TIPPER_MM_PER_PULSE = 0.2794;
        void ResetPluviometerPulseCounter();
        float CalculateRainfall(int);
        //
        //  Wind Speed sensor, each pulse per second represents 1.492 miles per hour.
        //
        volatile int _windSpeedPulseCount;
        ISRPointer WindSpeedISR;
        const int WINDSPEED_DURATION = 5;
        const float WINDSPEED_PER_PULSE = 1.492;
        const int PIN_ANEMOMETER = 5;
        //
        //  Wind direction sensor.
        //
        const uint8_t _windDriectionAnalogChannel = 1;
        WindDirectionLookup _windDirectionLookupTable[16];
        uint8_t _windDirectionLookupEntry;
        //
        //  Sensors attached to the STM8S
        //
        void ReadSTM8SSensors();
};

#endif
