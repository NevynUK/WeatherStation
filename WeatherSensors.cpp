//
//  Weather sensors class.
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
#include "WeatherSensors.h"

//******************************************************************************
//
//  Constructors etc.
//

//
//  Default constructor.
//
WeatherSensors::WeatherSensors()
{
    //
    //  Populate the wind direction lookup table.  Each entry has the following form:
    //
    //  midPoint, reading, angle, direction, direction as text
    //
    //  Note that this table is order not in angles/wind direction but in terms of the
    //  mid points of the ADC readings found empirically.  This allows for easier
    //  searching of the table.
    //    
    _windDirectionLookupTable[0] = { 0, 1112, 112.5, EastSouthEast, (char *) "East-South-East" };
    _windDirectionLookupTable[1] = { 1262, 1412, 67.5, EastNorthEast, (char *) "East-North-East" };
    _windDirectionLookupTable[2] = { 1489, 1567, 90, East, (char *) "East" };
    _windDirectionLookupTable[3] = { 1851, 2136, 157.5, SouthSouthEast, (char *) "South-South-East" };
    _windDirectionLookupTable[4] = { 2630, 3124, 135, SouthEast, (char *) "South-East" };
    _windDirectionLookupTable[5] = { 3627, 4131, 202.5, SouthSouthWest, (char *) "South-South-West" };
    _windDirectionLookupTable[6] = { 4359, 4587, 180, South, (char *) "South" };
    _windDirectionLookupTable[7] = { 5726, 6866, 22.5, NorthNorthEast, (char *) "North-North-East" };
    _windDirectionLookupTable[8] = { 7333, 7801, 45, NorthEast, (char *) "North-East" };
    _windDirectionLookupTable[9] = { 9220, 10639, 225, SouthWest, (char *) "South-West" };
    _windDirectionLookupTable[10] = { 10640, 10642, 247.5, WestSouthWest, (char *) "West-South-West" };
    _windDirectionLookupTable[11] = { 11263, 11884, 337.5, NorthNorthWest, (char *) "North-North-West" };
    _windDirectionLookupTable[12] = { 12585, 13287, 0, North, (char *) "North" };
    _windDirectionLookupTable[13] = { 13639, 13991, 292.5, WestNorthWest, (char *) "West-North-West" };
    _windDirectionLookupTable[14] = { 14497, 15004, 315, NorthWest, (char *) "North-West" };
    _windDirectionLookupTable[15] = { 15491, 15978, 270, West, (char *) "West" };
}

//
//  Destructor - clean up any memory / objects in use.
//
WeatherSensors::~WeatherSensors()
{
    delete(_groundSensor);
}

//******************************************************************************
//
//  General methods (initialisation etc.)
//

//
//  Initialise all of the weather sensors.
//
void WeatherSensors::InitialiseSensors()
{
    SetupGroundTemperatureSensor();
    SetupLuminositySensor();
    SetupTemperatureHumidityPressureSensor();
    SetupRainfallSensor();
    SeupWindSpeedSensor();
}

//
//  Read all of the sensors.
//
//  This method forces all of the sensor readings to be taken in one go.
//
void WeatherSensors::ReadAllSensors()
{
    //ReadGroundTemperatureSensor();
    //ReadLuminositySensor();
    //ReadTemperatureHumidityPressureSensor();
    ReadSTM8SSensors();
    //ReadUltravioletLightSensor();
    //ReadRainfallSensor();
    //ReadWindSpeedSensor();
    //ReadWindDirection();
}

//******************************************************************************
//
//  DS18B20 temperature sensor.
//
//  Setup the DS18B20 sensor (ground temperature).
//
void WeatherSensors::SetupGroundTemperatureSensor()
{
    int index;
    String message;
    char number[20];

    _groundSensor = new OneWire(5);

    if (!_groundSensor->search(_groundTemperatureSensorAddress))
    {
        Debugger::DebugMessage("No more addresses.");
        _groundSensor->reset_search();
        delay(250);
        return;
    }

    message = "ROM =";
    for (index = 0; index < 8; index++)
    {
        message += " ";
        message += itoa(_groundTemperatureSensorAddress[index], number, 16);
    }
    Debugger::DebugMessage(message);

    if (OneWire::crc8(_groundTemperatureSensorAddress, 7) != _groundTemperatureSensorAddress[7])
    {
        Debugger::DebugMessage("CRC is not valid!");
        return;
    }

    switch (_groundTemperatureSensorAddress[0])
    {
    case 0x10:
        Debugger::DebugMessage("Chip = DS18S20");
        _groundTemperatureSensorType = 1;
        break;
    case 0x28:
        Debugger::DebugMessage("Chip = DS18B20");
        _groundTemperatureSensorType = 0;
        break;
    case 0x22:
        Debugger::DebugMessage("Chip = DS1822");
        _groundTemperatureSensorType = 0;
        break;
    default:
        Debugger::DebugMessage("Device is not a DS18x20 family device.");
        return;
    }
}

//
//  Read the ground temperature from the DS18B20 sensor.
//
float WeatherSensors::ReadGroundTemperatureSensor()
{
    int index;
    byte data[12];
    byte present = 0;
    String message;
    char number[20];

    _groundSensor->reset();
    _groundSensor->select(_groundTemperatureSensorAddress);
    _groundSensor->write(0x44, 1);    // Start conversion, with parasite power on at the end
    delay(1000);          // Maybe 750ms is enough, maybe not
    present = _groundSensor->reset();
    _groundSensor->select(_groundTemperatureSensorAddress);
    _groundSensor->write(0xBE);       // Read Scratchpad

    message = "DS18B20 Data = ";
    message += itoa(present, number, 16);
    message += " ";
    for (index = 0; index < 9; index++)
    {
        data[index] = _groundSensor->read();
        message += itoa(data[index], number, 16);
        message += " ";
    }
    message += " CRC=";
    message += itoa(OneWire::crc8(data, 8), number, 16);
    Debugger::DebugMessage(message);

    if (OneWire::crc8(data, 8) == data[8])
    {
        int16_t raw = (data[1] << 8) | data[0];
        if (_groundTemperatureSensorType)
        {
            raw = raw << 3; // 9 bit resolution default
            if (data[7] == 0x10)
            {
                // "count remain" gives full 12 bit resolution
                raw = (raw & 0xFFF0) + 12 - data[6];
            }
        }
        else
        {
            byte cfg = (data[4] & 0x60);
            // at lower res, the low bits are undefined, so let's zero them
            switch (cfg)
            {
            case 0x00:
                raw = raw & ~7;   // 9 bit resolution, 93.75 ms
                break;
            case 0x20:
                raw = raw & ~3;   // 10 bit res, 187.5 ms
                break;
            case 0x40:
                raw = raw & ~1;   // 11 bit res, 375 ms
                break;
            }
            //// default is 12 bit resolution, 750 ms conversion time
        }
        _groundTemperature = (double) (raw / 16.0);
    }
    else
    {
        Debugger::DebugMessage("DS18B20 CRC failure");
    }
    return(_groundTemperature);
}

//
//  Get the last ground temperature reading.
//
float WeatherSensors::GetGroundTemperatureReading()
{
    return(_groundTemperature);
}

//******************************************************************************
//
//  Ultraviolet light sensor.
//

//
//  Read the ultraviolet light sensor.
//
float WeatherSensors::ReadUltravioletLightSensor()
{
    return(_ultraviolet);
}

//
//  Get the last value read from the UV sensor.
//
float WeatherSensors::GetUltravioletLightReading()
{
    return(_ultraviolet);
}

//
//  Convert the ultraviolet light sensor reading into mW/cm2.
//
float WeatherSensors::GetUltravioletLightStrength()
{
    float uvStrength = _ultraviolet * _voltsPerDivision;

    if (uvStrength < _uvOffset)
    {
        uvStrength = 0;
    }
    else
    {
        uvStrength = (uvStrength - _uvOffset) / _uvGradient;
    }
    return(uvStrength);
}

//******************************************************************************
//
//  Get data from the STM8S.
//
//  Sensors connected to the STM8S:
//
//  1 - Wind Speed
//  2 - Wind Direction
//  3 - Ultraviolet Light
//  4 - Rain Guage (Pluviometer)
//
void WeatherSensors::ReadSTM8SSensors()
{
    uint8_t buffer[I2CBufferSize];

    Wire.begin();
    Wire.setClock(100000);
    Wire.beginTransmission(STM8SAddress);
    Wire.write(I2CGetSensorData);
    Wire.endTransmission();
    Wire.requestFrom(STM8SAddress, I2CBufferSize);
    for (int index = 0; index < I2CBufferSize; index++)
    {
        buffer[index] = Wire.read();
    }
    //Wire.readBytes(buffer, I2CBufferSize);
    Debugger::DebugMessage("STM8S Sensor data", buffer, I2CBufferSize);
}

//******************************************************************************
//
//  Sparkfun Luminosity sensor.
//

//
//  Translate the error code into a meaningful message.
//
String WeatherSensors::LuminositySensorErrorMessage(byte error)
{
    switch (error)
    {
    case 0:
        return("TSL2561 Error: Success");
    case 1:
        return("TSL2561 Error: Data too long for transmit buffer");
    case 2:
        return("TSL2561 Error: Received NACK on address (disconnected?)");
    case 3:
        return("TSL2561 Error: Received NACK on data");
    case 4:
        return("TSL2561 Error: Other error");
    default:
        return("TSL2561 Error: Unknown error code");
    }
}

//
//  Set up the luminsoity sensor.
//
void WeatherSensors::SetupLuminositySensor()
{
    char buffer[256];

    _light.begin();

    // Get factory ID from sensor:
    // (Just for fun, you don't need to do this to operate the sensor)
    unsigned char id;

    if (_light.getID(id))
    {
        sprintf(buffer, "Retrieved TSL2561 device ID: 0x%x", id);
        Debugger::DebugMessage(buffer);
    }
    else
    {
        byte error = _light.getError();
        Debugger::DebugMessage(LuminositySensorErrorMessage(error));
    }

    // The light sensor has a default integration time of 402ms,
    // and a default gain of low (1X).

    // If you would like to change either of these, you can
    // do so using the setTiming() command.

    // If gain = false (0), device is set to low gain (1X)
    // If gain = high (1), device is set to high gain (16X)  
    _gain = 0;

    // If time = 0, integration will be 13.7ms
    // If time = 1, integration will be 101ms
    // If time = 2, integration will be 402ms
    // If time = 3, use manual start / stop to perform your own integration
    unsigned char time = 2;

    // setTiming() will set the third parameter (ms) to the
    // requested integration time in ms (this will be useful later):
    _light.setTiming(_gain, time, _ms);

    // To start taking measurements, power up the sensor:

    Debugger::DebugMessage((char *) "Powering up the luminosity sensor.");
    _light.setPowerUp();
}

//
//  Read the luminosity frolm the TSL2561 luminosity sensor.
//
double WeatherSensors::ReadLuminositySensor()
{
    unsigned int data0, data1;
    String message;
    char number[20];

    if (_light.getData(data0, data1))
    {
        //
        //  To calculate lux, pass all your settings and readings to the getLux() function.
        //
        //  The getLux() function will return 1 if the calculation was successful, or 0 if one or both of the sensors was
        //  saturated (too much light). If this happens, you can reduce the integration time and/or gain.
        //  For more information see the hookup guide at: 
        //  https://learn.sparkfun.com/tutorials/getting-started-with-the-tsl2561-luminosity-sensor
        //
        // Perform lux calculation.
        //
        double localLux;
        _good = _light.getLux(_gain, _ms, data0, data1, localLux);
        if (_good)
        {
            _lux = localLux;
        }
    }
    else
    {
        byte error = _light.getError();
        Debugger::DebugMessage(LuminositySensorErrorMessage(error));
    }
    return(_lux);
}

//
//  Get the last luminosity reading.
//
double WeatherSensors::GetLuminosityReading()
{
    return(_lux);
}

//******************************************************************************
//
//  BME280 temperature, air pressure and humidity sensor.
//

//
//  Setup the Adafruit BME280 Temperature, pressure and humidity sensor.
//
void WeatherSensors::SetupTemperatureHumidityPressureSensor()
{
    if (!_bme.begin())
    {
        Debugger::DebugMessage("Could not find a valid BME280 sensor, check wiring!");
    }
    else
    {
        Debugger::DebugMessage("BME280 sensor located on I2C bus.");
    }
    _temperature = 0;
    _pressure = 0;
    _humidity = 0;
}

//
//  Read the data from the Temperature, pressure and humidity sensor.
//
void WeatherSensors::ReadTemperatureHumidityPressureSensor()
{
    _temperature = _bme.readTemperature();
    _pressure = _bme.readPressure();
    _humidity = _bme.readHumidity();
}

//
//  Get the last air temperature reading.
//
float WeatherSensors::GetAirTemperature()
{
    return(_temperature);
}

//
//  Get the last humidity reading.
//
float WeatherSensors::GetHumidity()
{
    return(_humidity);
}

//
//  Get thje last air pressure reading.
//
float WeatherSensors::GetAirPressure()
{
    return(_pressure);
}

//******************************************************************************
//
//  Pluvometer (Rainfall) sensor.

//
//  Set up the pluiometer, zero the counts and then attached and initialise the
//  counter to the output expander.
//
void WeatherSensors::SetupRainfallSensor()
{
}

//
//  Read the counter from the Rainfall sensor then reset the count.
//
//  It is theoretically possible for the counter to change while it is being read.
//  This can give an incorrect reading so the counter is read twice in succession, if
//  the two readings are the same then we can assume that the reading is correct.
//
void WeatherSensors::ReadRainfallSensor()
{
}

//
//  Reset the pluviometer pulse counter.
//
void WeatherSensors::ResetPluviometerPulseCounter()
{
}

//
//  Convert the number of pulses into mm of rain.
//
float WeatherSensors::CalculateRainfall(int pulseCount)
{
    return(pulseCount * RAINFALL_TIPPER_MM_PER_PULSE);
}

//
//  Get the rainfal in mm between the last sensor reading and this one.
//
float WeatherSensors::GetRainfall()
{
    return(CalculateRainfall(_pluviometerPulseCount));
}

//
//  Get the total rainfall in mm today.
//
float WeatherSensors::GetTotalRainfallToday()
{
    return(CalculateRainfall(_pluviometerPulseCountToday));
}

//******************************************************************************
//
//  Windspeed sensor.
//

//
//  Set up the anemometer.
//
void WeatherSensors::SeupWindSpeedSensor()
{
    pinMode(PIN_ANEMOMETER, INPUT);
    _windSpeedPulseCount = 0;
    WindSpeedISR = NULL;
}

//
//  Read the wind speed.
//
float WeatherSensors::ReadWindSpeedSensor()
{
    _windSpeedPulseCount = 0;
    if (WindSpeedISR != NULL)
    {
        attachInterrupt(PIN_ANEMOMETER, WindSpeedISR, RISING);
        delay(WINDSPEED_DURATION * 1000);
        detachInterrupt(PIN_ANEMOMETER);
    }
}

//
//  Get the last wind speed reading.
//
float WeatherSensors::GetWindSpeed()
{
    return(_windSpeedPulseCount * WINDSPEED_PER_PULSE);
}

//
//  Set the Wind speed ISR
//
void WeatherSensors::SetWindSpeedISR(void (*isr)())
{
    WindSpeedISR = isr;
}

//
//  Called by the Wind speed ISR.
//
void WeatherSensors::HandleWindSpeedInterrupt()
{
    _windSpeedPulseCount++;
}

//******************************************************************************
//
//  Wind direction sensor.
//

//
//  Read the wind direction sensor and calculate the direction the vane is pointing.
//
WeatherSensors::WindDirection WeatherSensors::ReadWindDirection()
{
    String message = "Wind direction: ";
    char buffer[20];
    //message += Debugger::FloatToAscii(buffer, windDirection * _voltsPerDivision, 4);
    message += "V";
    Debugger::DebugMessage(message);
    message = "Wind direction reading: ";
    //message += itoa(windDirection, buffer, 10);
    Debugger::DebugMessage(message);
    _windDirectionLookupEntry = 15;
    for (int index = 0; index < 15; index++)
    {
        //if ((windDirection > _windDirectionLookupTable[index].midPoint) && (windDirection <= _windDirectionLookupTable[index + 1].midPoint))
        //{
        //    _windDirectionLookupEntry = index;
        //    break;
        //}
    }
    message = "Wind is blowing from ";
    message += _windDirectionLookupTable[_windDirectionLookupEntry].directionAsText;
    Debugger::DebugMessage(message);
}

//
//  Get the last wind direction reading as a textural description.
//
char *WeatherSensors::GetWindDirectionAsString()
{
    return(_windDirectionLookupTable[_windDirectionLookupEntry].directionAsText);
}