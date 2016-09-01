// 
//  Implement the methods for the DS3231 class.
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
#include "DS3231.h"
#include "Wire.h"

//
//  Construct a new DS3231 object using the default I2C address.
//
DS3231::DS3231()
{
    m_Address = 0x68;
}

//
//  Construct a new DS3231 object using a user specified I2C address.
//
DS3231::DS3231(uint8_t address)
{
    m_Address = address;
}

//
//  Clean up this class removing any resources.
//
DS3231::~DS3231()
{
}

//
//  Transfer a sequence of bytes to the DS3231.
//
void DS3231::BurstTransfer(uint8_t *dataToChip, uint8_t amountOfData)
{
    Wire.beginTransmission(m_Address);
    Wire.write(dataToChip, amountOfData);
    Wire.endTransmission();
}

//
//  Transfer a sequence of bytes to the DS3231 and then read the same
//  number of bytes from the DS3231.
//
//  This chip uses I2C for communication and so only uses the first byte of the
//  dataToSend parameter.  In SPI this is used to enable the writeRead method
//  to work efficiently.
//
void DS3231::BurstTransfer(uint8_t *dataToChip, uint8_t *dataFromChip, uint8_t amountOfData)
{
    Wire.beginTransmission(m_Address);
    Wire.write(dataToChip[0]);
    Wire.endTransmission();
    Wire.requestFrom(m_Address, (uint8_t) (amountOfData - 1));
    for (int index = 1; index < amountOfData; index++)
    {
        dataFromChip[index] = (Wire.read() & 0xff);
    }
}

//
//  Get a value from a register.
//
uint8_t DS3231::GetRegisterValue(const Registers reg)
{
    Wire.beginTransmission(m_Address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(m_Address, (uint8_t) 1);
    return(Wire.read() & 0xff);
}

//
//  Set the byte value of the specified register.
//
void DS3231::SetRegisterValue(const Registers reg, const uint8_t value)
{
    Wire.beginTransmission(m_Address);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

