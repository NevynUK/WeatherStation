// DS3234.h

#ifndef _DS3234_h
#define _DS3234_h

#include <BitBang\BitBang.h>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

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
class DS3234RealTimeClock
{
    public:
        //
        //  Enums.
        //
        enum Alarm { Alarm1, Alarm2, Both, Unknown };
        enum AlarmType { OncePerSecond, WhenSecondsMatch, WhenMinutesSecondsMatch,  // Alarm 1 options.
                         WhenHoursMinutesSecondsMatch, WhenDateHoursMinutesSecondsMatch, WhenDayHoursMinutesSecondsMatch,
                         OncePerMinute, WhenMinutesMatch, WhenHoursMinutesMatch,    // Alarm 2 options.
                         WhenDateHoursMinutesMatch, WhenDayHoursMinutesMatch };
        enum ControlRegisterBits { A1IE = 0x01, A2IE = 0x02, INTCON = 0x04, RS1 = 0x08, RS2 = 0x10, Conv = 0x20, BBSQW = 0x40, NotEOSC = 0x80 };
        enum StatusRegisterBits { A1F = 0x02, A2F = 0x02, BSY = 0x04, EN32Khz = 0x08, Crate0 = 0x10, Crate1 = 0x20, BB32kHz = 0x40, OSF = 0x80 };
        enum RateSelect { OneHz = 0, OnekHz = 1, FourkHz = 2, EightkHz = 3 };
        enum Registers { ReadAlarm1 = 0x07, WriteAlarm1 = 0x87,
                         ReadAlarm2 = 0x0b, WriteAlarm2 = 0x8b,
                         ReadControl = 0x0e, WriteControl = 0x8e,
                         ReadControlStatus = 0x0f, WriteControlStatus = 0x8f,
                         ReadAgingOffset = 0x10, WriteAgingOffset = 0x90, 
                         ReadSRAMAddress = 0x18, WriteSRAMAddress = 0x98,
                         ReadSRAMData = 0x19, WriteSRAMData = 0x99 };
        enum DayOfWeek { Sunday = 1, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday };
        //
        //  Construction and destruction.
        //
        DS3234RealTimeClock(const uint8_t);
        DS3234RealTimeClock(const uint8_t, const uint8_t, const uint8_t, const uint8_t);
        ~DS3234RealTimeClock();
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
        void WriteToSRAM(uint8_t, uint8_t);
        uint8_t ReadFromSRAM(uint8_t);
        void ReadAllRegisters();
        void DumpRegisters(const uint8_t *);
        void ReadSRAM();
        void DumpSRAM();

    private:
        //
        //  Constants.
        //
        static const int REGISTER_SIZE = 0x14;
        static const int DATE_TIME_REGISTERS_SIZE = 0x07;
        static const int MEMORY_SIZE = 0xff;
        //
        //  Variables
        //
        int m_ChipSelect = -1;
        int m_MOSI = -1;
        int m_MISO = -1;
        int m_Clock = -1;
        uint8_t m_Registers[REGISTER_SIZE];
        uint8_t m_SRAM[MEMORY_SIZE];
        BitBang *m_Port;
        //
        //  Make the default constructor private so it cannot be called explicitly.
        //
        DS3234RealTimeClock();
        //
        //  Methods.
        //
        void Initialise(const uint8_t, const uint8_t, const uint8_t, const uint8_t);
        void SetRegisterValue(const Registers, uint8_t);
        uint8_t GetRegisterValue(const Registers);
        uint8_t ConvertUint8ToBCD(const uint8_t);
        void BurstTransfer(uint8_t *, uint8_t);
        void BurstTransfer(uint8_t *, uint8_t *, uint8_t);
        uint8_t ConvertBCDToUint8(const uint8_t);
};

#endif