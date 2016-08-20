// 
//  Soft SPI class.
//
#include "BitBang.h"

//
//  Create anew object using the SPI bus pins and pin 6 as chip select.
//
BitBang::BitBang()
{
    Initialise(6, MOSI, MISO, SCK);
}

//
//  Create a new object using the SPI pins and the specified chip select pin.
//
BitBang::BitBang(uint8_t chipSelect)
{
    Initialise(chipSelect, MOSI, MISO, SCK);
}

//
//  Craete a new object using the specified SPI pins.
//
BitBang::BitBang(uint8_t chipSelect, uint8_t mosi, uint8_t miso, uint8_t clock)
{
    Initialise(chipSelect, mosi, miso, clock);
}

//
//  Initialise the class.
//
void BitBang::Initialise(uint8_t chipSelect, uint8_t mosi, uint8_t miso, uint8_t clock)
{
    m_ChipSelect = chipSelect;
    m_MOSI = mosi;
    m_MISO = miso;
    m_Clock = clock;
    pinMode(m_ChipSelect, OUTPUT);
    pinMode(m_Clock, OUTPUT);
    pinMode(m_MOSI, OUTPUT);
    pinMode(m_MISO, INPUT);
    digitalWrite(m_ChipSelect, HIGH);
}

//
//  Change the state of the chip select line.
//
void BitBang::ChipSelect(uint8_t state)
{
    digitalWrite(m_ChipSelect, state);
}

//
//  Transfer a single byte.
//
uint8_t BitBang::WriteRead(uint8_t b)
{
    uint8_t result = 0;
    uint8_t mask = 0x80;

    digitalWrite(m_Clock, LOW);
    for (int index = 0; index < 8; index++)
    {
        if (b & mask)
        {
            digitalWrite(m_MOSI, HIGH);
        }
        else
        {
            digitalWrite(m_MOSI, LOW);
        }
        digitalWrite(m_Clock, HIGH);
        delayMicroseconds(2);
        digitalWrite(m_Clock, LOW);
        result <<= 1;
        if (digitalRead(m_MISO) == HIGH)
        {
            result |= 0x01;
        }
        mask >>= 1;
    }
    return(result);
}

//
//  Transfer a number of bytes in a burst.
//
void BitBang::WriteRead(uint8_t *dataToChip, uint8_t *dataFromChip, uint8_t size)
{
    digitalWrite(m_ChipSelect, LOW);
    for (uint8_t count = 0; count < size; count++)
    {
        dataFromChip[count] = WriteRead(dataToChip[count]);
    }
    digitalWrite(m_ChipSelect, HIGH);
}
