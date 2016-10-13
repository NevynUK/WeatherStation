// 
//  Implement the methods for the DS323xTimerFunctions class.
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
#include "DS323xTimerFunctions.h"

//
//  Default constructor.
//
//  Prepare any resources needed by this class.
//
DS323xTimerFunctions::DS323xTimerFunctions()
{
}

//
//  Destructor should tidy up any memory allocation etc.
//
DS323xTimerFunctions::~DS323xTimerFunctions()
{
}

//
//  Read all of the registers from the DS323x and store them in memory.
//
void DS323xTimerFunctions::ReadAllRegisters()
{
    //m_Port->ChipSelect(LOW);
    //m_Port->WriteRead(0x00);
    //for (int index = 0; index < REGISTER_SIZE; index++)
    //{
    //    m_Registers[index] = m_Port->WriteRead(0x00);
    //}
    //m_Port->ChipSelect(HIGH);
}

//
//  Dump the registers to the serial port.
//
void DS323xTimerFunctions::DumpRegisters(const uint8_t *registers)
{
    Serial.println("      0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f");
    for (int index = 0; index < REGISTER_SIZE; index++)
    {
        if ((index % 0x10) == 0)
        {
            if (index > 0)
            {
                Serial.println("");
            }
            Serial.printf("0x%02x: ", index);
        }
        Serial.printf("0x%02x ", registers[index]);
    }
    Serial.println("");
}

//
//  Get the current control register.
//
uint8_t DS323xTimerFunctions::GetControlRegister()
{
    return(GetRegisterValue(Control));
}

//
//  Set the control register.
//
//  This method will mask off bits 0,1 and 2 as these are status bits.
//
void DS323xTimerFunctions::SetControlRegister(const uint8_t controlRegister)
{
    SetRegisterValue(Control, controlRegister);
}

//
//  Get the current control/status register.
//
uint8_t DS323xTimerFunctions::GetControlStatusRegister()
{
    return(GetRegisterValue(ControlStatus));
}

//
//  Set the control/status register.
//
void DS323xTimerFunctions::SetControlStatusRegister(const uint8_t controlRegister)
{
    SetRegisterValue(ControlStatus, controlRegister & 0xf8);
}

//
//  Get the value from the Aging Offset register.
//
uint8_t DS323xTimerFunctions::GetAgingOffset()
{
    return(GetRegisterValue(AgingOffset));
}

//
//  Set the aging offset register.
//
void DS323xTimerFunctions::SetAgingOffset(uint8_t agingOffset)
{
    SetRegisterValue(AgingOffset, agingOffset);
}

//
//  Get the last temperature reading.
//
float DS323xTimerFunctions::GetTemperature()
{
    uint8_t dataToChip[3], dataFromChip[3];
    uint16_t temperature;

    memset(dataFromChip, 0, 3);
    memset(dataToChip, 0, 3);
    dataToChip[0] = 0x11;
    BurstTransfer(dataToChip, dataFromChip, 3);
    temperature = (dataFromChip[1] << 2) | (dataFromChip[2] >> 6);
    return(temperature * 0.25);
}

//
//  Get the date and time from the DS323x and return a ts structure containing
//  the decoded time.
//
ts *DS323xTimerFunctions::GetDateTime()
{
    uint8_t dataToChip[DATE_TIME_REGISTERS_SIZE + 1], dataFromChip[DATE_TIME_REGISTERS_SIZE + 1];
    ts *result;

    //
    //  The trick here is to send one more byte than necessary.  The first byte
    //  contains the address of the first register we want to read, in this case
    //  0x00 (so we don't need to explicitly set it).  The subsequent bytes written
    //  to the chip will be ignored and we are accessing the data from the registers.
    //
    memset(dataToChip, 0, DATE_TIME_REGISTERS_SIZE + 1);
    memset(dataFromChip, 0, DATE_TIME_REGISTERS_SIZE + 1);
    BurstTransfer(dataToChip, dataFromChip, DATE_TIME_REGISTERS_SIZE + 1);
    //
    //  We should now have the date/time in dataFromChip[1..DATE_TIME_REGISTERS_SIZE + 1].
    //
    result = new(ts);
    result->seconds = ConvertBCDToUint8(dataFromChip[1]);
    result->minutes = ConvertBCDToUint8(dataFromChip[2]);
    if (dataFromChip[3] & 0x40)
    {
        result->hour = ConvertBCDToUint8(dataFromChip[3] & 0x1f);
        if (dataFromChip[3] & 0x20)
        {
            result->hour += 12;
        }
    }
    else
    {
        result->hour = ConvertBCDToUint8(dataFromChip[3] & 0x3f);
    }
    result->wday = dataFromChip[4];
    result->day = ConvertBCDToUint8(dataFromChip[5]);
    result->month = ConvertBCDToUint8(dataFromChip[6] & 0x7f);
    result->year = 1900 + ConvertBCDToUint8(dataFromChip[7]);
    if (dataFromChip[6] & 0x80)
    {
        result->year += 100;
    }
    return(result);
}

//
//  Set the date and time.
//
void DS323xTimerFunctions::SetDateTime(ts *dateTime)
{
    uint8_t dataToChip[DATE_TIME_REGISTERS_SIZE + 1];

    //
    //  The trick here is to send one more byte than necessary.  The first byte
    //  contains the address of the first register we want to read, in this case
    //  0x00.  The subsequent bytes contain the encoded date and time.
    //
    memset(dataToChip, 0, DATE_TIME_REGISTERS_SIZE + 1);
    dataToChip[0] = 0x00;
    dataToChip[1] = ConvertUint8ToBCD(dateTime->seconds);
    dataToChip[2] = ConvertUint8ToBCD(dateTime->minutes);
    dataToChip[3] = ConvertUint8ToBCD(dateTime->hour);           // Using 24 hour notation.
    dataToChip[4] = dateTime->wday;
    dataToChip[5] = ConvertUint8ToBCD(dateTime->day);
    dataToChip[6] = ConvertUint8ToBCD(dateTime->month);
    if (dateTime->year > 1999)
    {
        dataToChip[6] |= 0x80;
        dataToChip[7] = ConvertUint8ToBCD((dateTime->year - 2000) & 0xff);
    }
    else
    {
        dataToChip[7] = ConvertUint8ToBCD((dateTime->year - 1900) & 0xff);
    }
    //
    //  Now we can set the time.
    //
    BurstTransfer(dataToChip, DATE_TIME_REGISTERS_SIZE + 1);
}

//
//  Check the alarm status flags to work out which alarm has just triggered an interrupt.
//
DS323xTimerFunctions::Alarm DS323xTimerFunctions::WhichAlarm()
{
    uint8_t controlStatusRegister;

    controlStatusRegister = GetControlStatusRegister();
    if ((controlStatusRegister & 0x01) && (controlStatusRegister & 0x02))
    {
        return(DS323xTimerFunctions::BothAlarmsRaised);
    }
    if (controlStatusRegister & 0x01)
    {
        return(DS323xTimerFunctions::Alarm1Raised);
    }
    if (controlStatusRegister & 0x02)
    {
        return(DS323xTimerFunctions::Alarm2Raised);
    }
    return(DS323xTimerFunctions::Unknown);
}

//
//  Set the date and time of the specified alarm.
//
void DS323xTimerFunctions::SetAlarm(Alarm alarm, ts *time, AlarmType type)
{
    uint8_t dataToChip[5];
    int element = 1;
    uint8_t amount;
    uint8_t a1 = 0, a2 = 0, a3 = 0, a4 = 0;

    if (alarm == Alarm1Raised)
    {
        dataToChip[0] = Alarm1;
        dataToChip[1] = ConvertUint8ToBCD(time->seconds);
        element = 2;
        amount = 6;
    }
    else
    {
        dataToChip[0] = Alarm2;
        amount = 5;
    }
    dataToChip[element++] = ConvertUint8ToBCD(time->minutes);
    dataToChip[element++] = ConvertUint8ToBCD(time->hour);
    if ((type == WhenDayHoursMinutesMatch) || (type == WhenDayHoursMinutesSecondsMatch))
    {
        dataToChip[element] = ConvertUint8ToBCD(time->wday) | 0x40;
    }
    else
    {
        dataToChip[element] = ConvertUint8ToBCD(time->day);
    }
    switch (type)
    {
        //
        //  Alarm 1 interrupts.
        //
        case OncePerSecond:
            dataToChip[1] |= 0x80;
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            dataToChip[4] |= 0x80;
            break;
        case WhenSecondsMatch:
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            dataToChip[4] |= 0x80;
            break;
        case WhenMinutesSecondsMatch:
            dataToChip[3] |= 0x80;
            dataToChip[4] |= 0x80;
            break;
        case WhenHoursMinutesSecondsMatch:
            dataToChip[4] |= 0x80;
            break;
        case WhenDateHoursMinutesSecondsMatch:
            break;
        case WhenDayHoursMinutesSecondsMatch:
            dataToChip[4] |= 0x40;
            break;
        //
        //  Alarm 2 interupts.
        //
        case OncePerMinute:
            dataToChip[1] |= 0x80;
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            break;
        case WhenMinutesMatch:
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            break;
        case WhenHoursMinutesMatch:
            dataToChip[3] |= 0x80;
            break;
        case WhenDateHoursMinutesMatch:
            break;
        case WhenDayHoursMinutesMatch:
            dataToChip[3] |= 0x40;
            break;
    }
    BurstTransfer(dataToChip, amount);
    SetControlRegister(INTCON | A1IE);
}

//
//  Enable or disable the specified alarm.
//
void DS323xTimerFunctions::EnableDisableAlarm(const Alarm alarm, const bool enable)
{
    uint8_t controlRegister;

    controlRegister = GetControlRegister();
    if (alarm == Alarm1Raised)
    {
        if (enable)
        {
            controlRegister |= A1IE;
        }
        else
        {
            controlRegister &= ~A1IE;
        }
    }
    else
    {
        if (enable)
        {
            controlRegister |= A2IE;
        }
        else
        {
            controlRegister &= ~A2IE;
        }
    }
    SetControlRegister(controlRegister);
}

//
//  Process the interrupts generated by the alarms.
//
void DS323xTimerFunctions::InterruptHandler()
{
}

//
//  Clear the interrupt flag for the specified alarm.
//
void DS323xTimerFunctions::ClearInterrupt(Alarm alarm)
{
    uint8_t controlStatusRegister;

    controlStatusRegister = GetControlStatusRegister();
    if (alarm == Alarm1Raised)
    {
        controlStatusRegister &= ~A1F;
    }
    else
    {
        controlStatusRegister &= ~A2F;
    }
    SetControlStatusRegister(controlStatusRegister);
}

//
//  Convert a BCD number into an integer.
//
uint8_t DS323xTimerFunctions::ConvertBCDToUint8(const uint8_t value)
{
    uint8_t result;

    result = (value & 0xf0) >> 4;
    result *= 10;
    result += (value & 0x0f);
    return (result);
}

//
//  Convert a number to BCD format.
//
uint8_t DS323xTimerFunctions::ConvertUint8ToBCD(const uint8_t value)
{
    uint8_t result;

    result = value % 10;
    result |= (value / 10) << 4;
    return (result);
}

//
//  Get the current date and time as a string.
//
String DS323xTimerFunctions::DateTimeString(ts *theTime)
{
    char buff[MAX_BUFFER_SIZE];

    sprintf(buff, "%02d-%02d-%04d %02d:%02d:%02d", theTime->day, theTime->month, theTime->year, theTime->hour, theTime->minutes, theTime->seconds);
    return(buff);
}
