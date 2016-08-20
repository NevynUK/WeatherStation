// BitBang.h

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

