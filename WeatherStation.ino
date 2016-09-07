//
//  Weather Station
//
//  Main application for the Weather Station project based around the Digistup 
//  Oak board.
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
#include <Arduino.h>
#include "DS3231.h"
#include "Debug.h"
#include "Secrets.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Time.h>
#include <NtpClientLib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SparkFunTSL2561.h>
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include "WeatherSensors.h"
#include <Ticker.h>

//
//  Definitions used in the code for pins etc.
//
#define VERSION                 "0.19"
//
#define PIN_ONBOARD_LED         1
#define PIN_WIND_SPEED          9
#define PIN_PLUVIOMETER         8
#define PIN_GROUND_TEMPERATURE  7
#define PIN_RTC_INTERRUPT       5
#define PIN_WIND_DIRECTION      A0

//
//  Weather Sensor definitions.
//
WeatherSensors *_sensors;

//
//  Local variables to deal with the analogue and digital sensors
//  connected to the Oak.
//
volatile unsigned int _pluviometerCountToday = 0;
volatile unsigned int _windSpeedCount = 0;
volatile unsigned short _windDirectionReading = 0;
volatile unsigned int _lastFiveSecondWindSpeedCount = 0;
Ticker _fiveSecondTicker;

//
//  Indicate if we should read the sesnosrs.
//
volatile bool _readSensors = false;
unsigned int _readingNumber = 0;

//
//  DS3234 real time clock object.
//
DS3231 *rtc;
#define MAX_NTP_RETRIES 10

//
//  We are logging to Phant and we need somewhere to store the client and keys.
//
#define PHANT_DOMAIN        "data.sparkfun.com"
#define PHANT_PAGE          "/input/zDA9M8dQlahOqo4bx5Dd"
#define PHANT_PORT          80
WiFiClient _wifiClient;

//
//  Used for debugging, determine the output state of the onboard LED.
//
bool _ledOutput = false;

//
//  Post the data to the Sparkfun web site.
//
void PostDataToPhant()
{
    char number[20];
    
    String url = PHANT_PAGE "?private_key=" PHANT_PRIVATE_KEY;
    url += "&airpressure=";
    url += Debugger::FloatToAscii(number, _sensors->GetAirPressure() / 100, 0);
    url += "&groundtemperature=";
    url += Debugger::FloatToAscii(number, _sensors->GetGroundTemperatureReading(), 2);
    url += "&airtemperature=";
    url += Debugger::FloatToAscii(number, _sensors->GetAirTemperature(), 2);
    url += "&humidity=";
    url += Debugger::FloatToAscii(number, _sensors->GetHumidity(), 2);
    url += "&luminosity=";
    url += Debugger::FloatToAscii(number, _sensors->GetLuminosityReading(), 2);
    url += "&rainfall=";
    url += Debugger::FloatToAscii(number, _pluviometerCountToday, 2);
    url += "&winddirection=";
    url += _sensors->GetWindDirectionAsString();
    url += "&windspeed=";
    url += Debugger::FloatToAscii(number, (_lastFiveSecondWindSpeedCount * 1.492) / 5, 2);
    //
    //  Send the data to Phant (Sparkfun's data logging service).
    //
    HTTPClient http;
    http.begin(PHANT_DOMAIN, PHANT_PORT, url);
    int httpCode = http.GET();
    String message = "Status code: ";
    message += itoa(httpCode, number, 10);
    Debugger::DebugMessage(message);
    if (httpCode == 200)
    {
        String response = http.getString();
        message = "Phant response code: ";
        message += response[3];
        Debugger::DebugMessage(message);
        if (response[3] != '1')
        {
            //
            //  Need to put some error handling here.
            //  
        }
    }
    else
    {
        Debugger::DebugMessage("Error sending data to Phant.");
    }
    http.end();
}

//
//  Update the RTC with the time from the Internet.
//
void UpdateRTCWithInternetTime(DS3231 *rtc)
{
    ntpClient *ntp;
    ts *dateTime;
    int retries = 0;

    Debugger::DebugMessage("Getting Internet time and setting RTC.");
    ntp = ntpClient::getInstance("time.nist.gov", 0);
    ntp->setInterval(1, 1800);
    delay(1000);
    ntp->begin();
    time_t ntpTime = ntp->getTime();
    while ((year(ntpTime) == 1970) && (retries < MAX_NTP_RETRIES))
    {
        delay(50);
        ntpTime = ntp->getTime();
        retries++;
    }
    if (retries < MAX_NTP_RETRIES)
    {
        dateTime = new(ts);
        dateTime->hour = hour(ntpTime);
        dateTime->minutes = minute(ntpTime);
        dateTime->seconds = second(ntpTime);
        dateTime->day = day(ntpTime);
        dateTime->month = month(ntpTime);
        dateTime->year = year(ntpTime);
        dateTime->wday = weekday(ntpTime);
        rtc->SetDateTime(dateTime);
        Debugger::DebugMessage("Setting RTC to: " + rtc->DateTimeString(dateTime));
        delay(1000);
    }
    else
    {
        Debugger::DebugMessage("NTP retry count exceeded.");
    }
    ntp->stop();
}

//
//  Reset the alarm.
//
void SetAlarm(DS3231 *rtc, uint8_t period)
{
    ts *dateTime = rtc->GetDateTime();
    Debugger::DebugMessage("Current time retrieved: " + rtc->DateTimeString(dateTime));
    uint8_t minutes = dateTime->minutes;
    //
    //  Note that the alarm we are using is When minutes and seconds match.
    //  This mean we do not have to worry about the hours, day etc rolling
    //  over.  A bit dirty I know :)
    //
    minutes += period;
    if (minutes >= 60)
    {
        minutes = 0;
    }
    dateTime->minutes = minutes;
    dateTime->seconds = 0;
    Debugger::DebugMessage("Setting alarm for " + rtc->DateTimeString(dateTime));
    rtc->ClearInterrupt(DS3231::Alarm1Raised);
    rtc->SetAlarm(DS3231::Alarm1Raised, dateTime, DS3231::WhenMinutesSecondsMatch);
    delete(dateTime);
}

//
//  Take sensor readings an push the data to the Internet.
//
void ReadAndPublishData()
{
    digitalWrite(PIN_ONBOARD_LED, HIGH);
    Debugger::DebugMessage("Reading sensor data (", _readingNumber, 10, ")");
    _sensors->ReadAllSensors();
    Debugger::DebugMessage("Luminosity:", (float) _sensors->GetLuminosityReading(), 2u, "lumens");
    Debugger::DebugMessage("Air temperature:", _sensors->GetAirTemperature(), 2u, "C");
    Debugger::DebugMessage("Humidity:", _sensors->GetHumidity(), 2u, "%");
    Debugger::DebugMessage("Humidity:", _sensors->GetAirPressure() / 100, 2u, "hPa");
    Debugger::DebugMessage("Ground temperature:", _sensors->GetGroundTemperatureReading(), 2u, "C");
    Debugger::DebugMessage("Rainfall today:", _pluviometerCountToday * 0.2794, 2u, "mm");
    Debugger::DebugMessage("Wind speed pulse count:", _lastFiveSecondWindSpeedCount, 10, "");
    Debugger::DebugMessage("Wind speed:", (_lastFiveSecondWindSpeedCount * 1.492) / 5, 2u, "mph");
    //
    //  Now post to the Internet.
    //
    Debugger::DebugMessage("Posting to Internet.");
    PostDataToPhant();
    digitalWrite(PIN_ONBOARD_LED, LOW);
}

//
//  Handle the RTC interrupt.
//
//  - Update the RTC from the Internet if necessary.
//  - Reset the alarm
//  - Indicate that the sensors need reading if necessary (this is an 
//    ISR and we need to get out of here as soon as possible)
//
void RTCAlarmHandler()
{
    Debugger::DebugMessage("--------------------------------------------------");
    Debugger::DebugMessage("");
    Debugger::DebugMessage("Alarm interrupt raised.");
    //
    //  Reset the alarm.
    //
    SetAlarm(rtc, 1);
    //
    //  Indicate that the sensors should be read.
    //
    _readSensors = true;
    Debugger::DebugMessage("Exiting RTC alarm handler");
}

//
//  Handle the wind speed interrupts.
//
void WindSpeedInterruptHandler()
{
    _windSpeedCount++;
}

//
//  Hnadle the Ticker event every five seconds.
//
void FiveSecondTickerInterruptHandler()
{
    _lastFiveSecondWindSpeedCount = _windSpeedCount;
    _windSpeedCount = 0;
}

//
//  Simply increment the pluviometer counter for today.
//
void PluviometerInterruptHandler()
{
    _pluviometerCountToday++;
}

//
//  Setup the application.
//
void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Wire.begin();
    Wire.setClock(100000);
    //
    rtc = new DS3231();
    Debugger::AttachRTC(rtc);
    Debugger::DebugMessage("-----------------------------");
    Debugger::DebugMessage("Weather Station Starting (version " VERSION ", built: " __TIME__ " on " __DATE__ ")");
    //
    //  Connect to the WiFi.
    //
    Debugger::DebugMessage("Connecting to default network");
    WiFi.begin();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
    }
    Debugger::DebugMessage("WiFi connected, IP address: " + WiFi.localIP().toString());
    //
    //  Get the current date and time from the RTC or a time server.
    //
    UpdateRTCWithInternetTime(rtc);
    SetAlarm(rtc, 1);
    _sensors = new WeatherSensors();
    _sensors->InitialiseSensors();
    pinMode(PIN_RTC_INTERRUPT, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_RTC_INTERRUPT), RTCAlarmHandler, FALLING);
    pinMode(PIN_WIND_SPEED, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_WIND_SPEED), WindSpeedInterruptHandler, FALLING);
    pinMode(PIN_PLUVIOMETER, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_PLUVIOMETER), PluviometerInterruptHandler, FALLING);
    pinMode(PIN_WIND_DIRECTION, INPUT);
    pinMode(PIN_ONBOARD_LED, OUTPUT);
    //
    _fiveSecondTicker.attach(5.0, FiveSecondTickerInterruptHandler);
}

//
//  Main program loop.
//
void loop()
{
    if (_readSensors)
    {
        _readSensors = false;
        _readingNumber++;
        ReadAndPublishData();
    }
    digitalWrite(PIN_ONBOARD_LED, _ledOutput ? 1 : 0);
    _ledOutput = !_ledOutput;
    delay(250);
}