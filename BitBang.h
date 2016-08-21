//
//  Header for the soft SPI class.
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
#ifndef _BITBANG_h
#define _BITBANG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class BitBang
{
    private:
        uint8_t m_ChipSelect = -1;
        uint8_t m_Clock = 9;
        uint8_t m_MOSI = 7;
        uint8_t m_MISO = 8;
        void Initialise(uint8_t, uint8_t, uint8_t, uint8_t);

    public:
        BitBang();
        BitBang(uint8_t);
        BitBang(uint8_t, uint8_t, uint8_t, uint8_t);
        uint8_t WriteRead(uint8_t b);
        void WriteRead(uint8_t *, uint8_t *, uint8_t);
        void ChipSelect(uint8_t state);
};

#endif

