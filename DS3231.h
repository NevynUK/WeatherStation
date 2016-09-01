// 
//  Header for the DS3131 class.
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
#ifndef _DS3231_H_
#define _DS3231_H_

#include "DS323xTimerFunctions.h"

class DS3231 : public DS323xTimerFunctions
{
    public:
        DS3231();
        DS3231(uint8_t);
        ~DS3231();

    private:
        //
        //  Private variable to support this class.
        //
        uint8_t m_Address;
        //
        //  Private methods holding the chip specific implementation
        //  of the communication protocol.
        //
        void BurstTransfer(uint8_t *, uint8_t);
        void BurstTransfer(uint8_t *, uint8_t *, uint8_t);
        uint8_t GetRegisterValue(const Registers);
        void SetRegisterValue(const Registers reg, const uint8_t);
};

#endif