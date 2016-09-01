//
//  Header for teh class supporting the DS3234 RTC.
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
#ifndef _DS323xTimerFunctions_h_
#define _DS323xTimerFunctions_h_

#include "arduino.h"

#define MAX_BUFFER_SIZE     256

//
//  Time structure.
//
struct ts
{
    uint8_t seconds;    // Number of seconds, 0-59
    uint8_t minutes;    // Number of minutes, 0-59
    uint8_t hour;       // Number of hours, 0-23
    uint8_t day;        // Day of the month, 1-31
    uint8_t month;      // Month of the year, 1-12
    uint16_t year;      // Year >= 1900
    uint8_t wday;       // Day of the week, 1-7
};

//
//  Define the methods required to communicate with the DS3234 Real Tiem Clock.
//
class DS323xTimerFunctions
{
    public:
        //
        //  Enums.
        //
        enum Alarm { Alarm1Raised, Alarm2Raised, BothAlarmsRaised, Unknown };
        enum AlarmType { OncePerSecond, WhenSecondsMatch, WhenMinutesSecondsMatch,  // Alarm 1 options.
                         WhenHoursMinutesSecondsMatch, WhenDateHoursMinutesSecondsMatch, WhenDayHoursMinutesSecondsMatch,
                         OncePerMinute, WhenMinutesMatch, WhenHoursMinutesMatch,    // Alarm 2 options.
                         WhenDateHoursMinutesMatch, WhenDayHoursMinutesMatch };
        enum ControlRegisterBits { A1IE = 0x01, A2IE = 0x02, INTCON = 0x04, RS1 = 0x08, RS2 = 0x10, Conv = 0x20, BBSQW = 0x40, NotEOSC = 0x80 };
        enum StatusRegisterBits { A1F = 0x02, A2F = 0x02, BSY = 0x04, EN32Khz = 0x08, Crate0 = 0x10, Crate1 = 0x20, BB32kHz = 0x40, OSF = 0x80 };
        enum RateSelect { OneHz = 0, OnekHz = 1, FourkHz = 2, EightkHz = 3 };
        enum Registers { Alarm1 = 0x07, Alarm2 = 0x0b, Control = 0x0e, ControlStatus = 0x0f, AgingOffset = 0x10 };
        enum DayOfWeek { Sunday = 1, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday };
        //
        //  Construction and destruction.
        //
        ~DS323xTimerFunctions();
        //
        //  Methods.
        //
        uint8_t GetAgingOffset();
        void SetAgingOffset(uint8_t);
        float GetTemperature();
        uint8_t GetControlStatusRegister();
        void SetControlStatusRegister(uint8_t);
        uint8_t GetControlRegister();
        void SetControlRegister(uint8_t);
        ts *GetDateTime();
        Alarm WhichAlarm();
        void SetDateTime(ts *);
        String DateTimeString(ts *);
        void SetAlarm(Alarm, ts *, AlarmType);
        void InterruptHandler();
        void ClearInterrupt(Alarm);
        void EnableDisableAlarm(Alarm, bool);
        void ReadAllRegisters();
        void DumpRegisters(const uint8_t *);

    protected:
        //
        //  Make the default constructor protected so it can be called
        //  from derived classes only.
        //
        DS323xTimerFunctions();

    private:
        //
        //  Constants.
        //
        static const int REGISTER_SIZE = 0x14;
        static const int DATE_TIME_REGISTERS_SIZE = 0x07;
        //
        //  Variables
        //
        uint8_t m_Registers[REGISTER_SIZE];
        //
        //  Virtual functions.
        //
        virtual void BurstTransfer(uint8_t *, uint8_t) = 0;
        virtual void BurstTransfer(uint8_t *, uint8_t *, uint8_t) = 0;
        //
        //  Methods.
        //
        void Initialise(const uint8_t, const uint8_t, const uint8_t, const uint8_t);
        virtual void SetRegisterValue(const Registers, uint8_t) = 0;
        virtual uint8_t GetRegisterValue(const Registers) = 0;
        uint8_t ConvertUint8ToBCD(const uint8_t);
        uint8_t ConvertBCDToUint8(const uint8_t);
};

#endif