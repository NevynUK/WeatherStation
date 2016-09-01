//
//  STM8S code to read the following sensors:
//      - Rain guage (pluviometer)
//      - Wind direction
//      - Wind speed
//      - UV Light intensity
//
//  The data from the sensors is made available over I2C.  The sensor readings
//  are triggered by an external interrupt.
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
#include <iostm8s103f3.h>
#include <intrinsics.h>

//
//  Define some pins to output diagnostic data.
//
#define PIN_BIT_BANG_DATA       PC_ODR_ODR3
#define PIN_BIT_BANG_CLOCK      PC_ODR_ODR4
//
//  Wind Direction sensor.
//
#define PIN_WIND_DIRECTION      PD_ODR_ODR6

//
//  Work out the timer values for the reset pulse (300 uS).
//
#define CLOCK_SPEED             16
#define RESET_LENGTH            1000
#define RESET_PULSE_COUNT       (CLOCK_SPEED * RESET_LENGTH)
#define TIMER2_RESET_LOW_BYTE   ((unsigned char) ((RESET_PULSE_COUNT & 0xff)))
#define TIMER2_RESET_HIGH_BYTE  ((unsigned char) ((RESET_PULSE_COUNT >> 8) & 0xff))
//
//  Now the timer values for the sensor reading (2 S = 2,000,000 uS).
//
#define SENSOR_READING_LENGTH       		2000000
#define TIMER2_SENSOR_READING_PRESCALAR     0x0f
#define SENSOR_READING_PULSE_COUNT  		(CLOCK_SPEED * SENSOR_READING_LENGTH / 32768)
#define TIMER2_SENSOR_LOW_BYTE   			((unsigned char) ((SENSOR_READING_PULSE_COUNT & 0xff)))
#define TIMER2_SENSOR_HIGH_BYTE  			((unsigned char) ((SENSOR_READING_PULSE_COUNT >> 8) & 0xff))

//
//  Define which ADC is for which sensor.
//
#define ADC_UV_SENSOR               0x05
#define ADC_WIND_DIRECTION_SENSOR   0x06

//
//  Interrupt masks and pins for sensors and the RTC.
//
volatile unsigned char _lastPortDInput = 0;
#define MASK_WIND_SPEED         0x10
#define MASK_RTC                0x04
#define MASK_RAIN_GAUGE         0x08

//
//  Indicate if the sensor data is ready to be retrieved.
//
volatile bool _dataReady = false;

//
//  Determine if we have just powered on.
//
volatile unsigned char _resetCode;

//
//  Set aside some storage for the sensor readings.
//
volatile unsigned short _uvReading = 0;
volatile unsigned short _windDirectionReading = 0;
volatile unsigned short _rainGaugePulseCount = 0;
volatile unsigned short _windSpeedPulseCount = 0;

//
//  I2C config information
//
#define DEVICE_ADDRESS          			0x48
//
//  Somewhere to hold the data for the I2C interface.
//
#define I2C_INPUT_BUFFER_LENGTH             2
unsigned char _rxBuffer[I2C_INPUT_BUFFER_LENGTH];
volatile int _rxBufferPointer = 0;
//
#define I2C_OUTPUT_RAINFALL_COUNTER_MSB     0
#define I2C_OUTPUT_RAINFALL_COUNTER_LSB     1
#define I2C_OUTPUT_WINDSPEED_MSB            2
#define I2C_OUTPUT_WINDSPEED_LSB            3
#define I2C_OUTPUT_WIND_DIRECTION_MSB       4
#define I2C_OUTPUT_WIND_DIRECTION_LSB       5
#define I2C_OUTPUT_UV_READING_MSB           6
#define I2C_OUTPUT_UV_READING_LSB           7
#define I2C_OUTPUT_BUFFER_LENGTH            8
unsigned char _txBuffer[I2C_OUTPUT_BUFFER_LENGTH];
volatile int _txBufferPointer = 0;
volatile int _amountToSend = 0;
volatile bool _processCommand = false;
//
//  Commands that this application understands.
//
#define I2C_RESET_STATE                     0x00
#define I2C_READ_SENSORS                    0x01
#define I2C_GET_SENSOR_DATA                 0x02
#define I2C_DATA_READY                      0x03
#define I2C_RESET_RAINFALL_COUNTER          0x04

//--------------------------------------------------------------------------------
//
//  Bit bang data on the diagnostic pins.
//
void BitBang(unsigned char byte)
{
    for (short bit = 7; bit >= 0; bit--)
    {
        if (byte & (1 << bit))
        {
            PIN_BIT_BANG_DATA = 1;
        }
        else
        {
            PIN_BIT_BANG_DATA = 0;
        }
        PIN_BIT_BANG_CLOCK = 1;
        __no_operation();
        PIN_BIT_BANG_CLOCK = 0;
    }
    PIN_BIT_BANG_DATA = 0;
}

//--------------------------------------------------------------------------------
//
//  Write the default values into EEPROM.
//
void SetDefaultValues()
{
    //
    //  Check if the EEPROM is write-protected.  If it is then unlock the EEPROM.
    //
//    if (FLASH_IAPSR_DUL == 0)
//    {
//        FLASH_DUKR = 0xae;
//        FLASH_DUKR = 0x56;
//    }
//    //
//    //  Write the data to the EEPROM.
//    //
//    char *address = (char *) EEPROM_DATA_START;     //  Data location in EEPROM.
//    *address = (char *) DEVICE_ADDRESS9;            //  Default I2C address of this device.
//    //
//    //  Now write protect the EEPROM.
//    //
//    FLASH_IAPSR_DUL = 0;
}

//--------------------------------------------------------------------------------
//
//  Setup the ADC to perform a single conversion and then generate an interrupt.
//
void InitialiseADC()
{
    ADC_CR1_ADON = 1;                   //  Turn ADC on, note a second set is
                                        //  required to start the conversion.
    ADC_CSR_CH = ADC_UV_SENSOR;         //  ADC for the UV sensor first.
    ADC_CR3_DBUF = 0;
    ADC_CR2_ALIGN = 1;                  //  Data is right aligned.
    ADC_CSR_EOCIE = 1;                  //  Enable the interrupt after conversion completed.
}

//--------------------------------------------------------------------------------
//
//  Set up the system clock to run at 16MHz using the internal oscillator.
//
void InitialiseSystemClock()
{
    CLK_ICKR = 0;                       //  Reset the Internal Clock Register.
    CLK_ICKR_HSIEN = 1;                 //  Enable the HSI.
    CLK_ECKR = 0;                       //  Disable the external clock.
    while (CLK_ICKR_HSIRDY == 0);       //  Wait for the HSI to be ready for use.
    CLK_CKDIVR = 0;                     //  Ensure the clocks are running at full speed.
    CLK_PCKENR1 = 0xff;                 //  Enable all peripheral clocks.
    CLK_PCKENR2 = 0xff;                 //  Ditto.
    CLK_CCOR = 0;                       //  Turn off CCO.
    CLK_HSITRIMR = 0;                   //  Turn off any HSIU trimming.
    CLK_SWIMCCR = 0;                    //  Set SWIM to run at clock / 2.
    CLK_SWR = 0xe1;                     //  Use HSI as the clock source.
    CLK_SWCR = 0;                       //  Reset the clock switch control register.
    CLK_SWCR_SWEN = 1;                  //  Enable switching.
    while (CLK_SWCR_SWBSY != 0);        //  Pause while the clock switch is busy.
}

//--------------------------------------------------------------------------------
//
//  Configure the GPIO pins on ports C & D.
//
void InitialiseGPIO()
{
    PD_ODR = 0;             //  All pins are turned off.
    //
    //  RTC is on port D2.
    //
    PD_DDR_DDR2 = 0;        //  D2 is an input pin.
    PD_CR1_C12 = 0;         //  Floating input.
    PD_CR2_C22 = 1;         //  Now enable the interrupt on this pin.
    //
    //  Rain gauge is on port D3.
    //
    PD_DDR_DDR3 = 0;        //  D2 is an input pin.
    PD_CR1_C13 = 0;         //  Floating input.
    PD_CR2_C23 = 1;         //  Now enable the interrupt on this pin.
    //
    //  Wind speed switch is on port D4.
    //
    PD_DDR_DDR4 = 0;        //  D4 is an input pin.
    PD_CR1_C14 = 0;         //  Floating input.
    PD_CR2_C24 = 1;         //  Now enable the interrupt on this pin.
    //
    //  Set port D to detect falling edge interrupts only.
    //
    EXTI_CR1_PDIS = 3;      //  Falling edge only 0b10.
    EXTI_CR2_TLIS = 0;      //  Interrupt on falling edge.
    //
    //  Port C7 is output and is used to reset the ESP8266.  Configure
    //  as an output, push-pull operating at upto 10 MHz, turn the pin
    //  off inititially.
    //
    PC_DDR_DDR7 = 1;    	//  Output pin.
    PC_CR1_C17 = 1;         //  Push-pull output.
    PC_CR2_C27 = 1;         //  10 MHz output.
    PC_ODR_ODR7 = 1;        //  Turn pin on.
    //
    //  Diagnostic output pins, one data, one clock.
    //
    PC_DDR_DDR3 = 1;    	//  Output pin.
    PC_CR1_C13 = 1;         //  Push-pull output.
    PC_CR2_C23 = 1;         //  10 MHz output.
    PC_ODR_ODR3 = 0;        //  Turn pin off.
    //
    PC_DDR_DDR4 = 1;    	//  Output pin.
    PC_CR1_C14 = 1;         //  Push-pull output.
    PC_CR2_C24 = 1;         //  10 MHz output.
    PC_ODR_ODR4 = 0;        //  Turn pin off.
}

//--------------------------------------------------------------------------------
//
//  Initialise the I2C bus.
//
void InitialiseI2C()
{
    I2C_CR1_PE = 0;                     //  Disable I2C before configuration starts.
    //
    //  Set up the clock information.
    //
    I2C_FREQR = 16;                     //  Set the internal clock frequency (MHz).
    //
    //  Set the address of this device.
    //
    I2C_OARH_ADDMODE = 0;               //  7 bit address mode.
    I2C_OARH_ADD = 0;                   //  Set this device address.
    I2C_OARL_ADD = DEVICE_ADDRESS;      //
    I2C_OARH_ADDCONF = 1;               //  Docs say this must always be 1.
    //
    //  Setup the bus characteristics.
    //
    I2C_TRISER = 17;                    //  Set the maximum SCL rise time.
    //
    //  Turn on the interrupts.
    //
    I2C_ITR_ITBUFEN = 1;                //  Buffer interrupt enabled.
    I2C_ITR_ITEVTEN = 1;                //  Event interrupt enabled.
    I2C_ITR_ITERREN = 1;                //  Error interrupt enabled.
    //
    //  Configuration complete so turn the peripheral on.
    //
    I2C_CR1_PE = 1;
    //
    //  Send acknowledge after every byte received.
    //
    I2C_CR2_ACK = 1;
}

//--------------------------------------------------------------------------------
//
//  Set up Timer 1, channel 3 to output a single pulse lasting 240 uS.
//
void InitialiseTimer1()
{
    TIM1_ARRH = 0x02;       //  Reload counter = 512
    TIM1_ARRL = 0x00;
    TIM1_PSCRH = 0xf4;      //  Prescalar = 62,500
    TIM1_PSCRL = 0x24;
    TIM1_IER_UIE = 1;       //  Turn interrupts on.
}

//--------------------------------------------------------------------------------
//
//  Process the command in the I2C receive buffer.
//
void ExecuteI2CCommand()
{
    BitBang(0x06);
    BitBang(_rxBuffer[0]);
    switch (_rxBuffer[0])
    {
        case I2C_RESET_STATE:
            _txBuffer[0] = _resetCode;
            _amountToSend = 1;
            _resetCode = 1;
            break;
        case I2C_READ_SENSORS:
            //
            //  Start sensor reading, do not need to respond with any data yet.
            //
            _dataReady = false;
            ADC_CSR_CH = ADC_UV_SENSOR;     //  ADC for the UV sensor first.
            ADC_CR1_ADON = 1;
            __no_operation();
            ADC_CR1_ADON = 1;
            break;
        case I2C_GET_SENSOR_DATA:
            if (_dataReady)
            {
                //
                //  Copy the sensor data into the buffer ready for transmission.
                //
                _txBuffer[I2C_OUTPUT_RAINFALL_COUNTER_LSB] = (unsigned char) (_rainGaugePulseCount & 0xff);
                _txBuffer[I2C_OUTPUT_RAINFALL_COUNTER_MSB] = (unsigned char) ((_rainGaugePulseCount >> 8) & 0xff);
                _txBuffer[I2C_OUTPUT_WINDSPEED_LSB] = (unsigned char) (_windSpeedPulseCount & 0xff);
                _txBuffer[I2C_OUTPUT_WINDSPEED_MSB] = (unsigned char) ((_windSpeedPulseCount >> 8) & 0xff);
                _txBuffer[I2C_OUTPUT_UV_READING_LSB] = (unsigned char) (_uvReading & 0xff);
                _txBuffer[I2C_OUTPUT_UV_READING_MSB] = (unsigned char) ((_uvReading >> 8) & 0xff);
                _txBuffer[I2C_OUTPUT_WIND_DIRECTION_LSB] = (unsigned char) (_windDirectionReading & 0xff);
                _txBuffer[I2C_OUTPUT_WIND_DIRECTION_MSB] = (unsigned char) ((_windDirectionReading >> 8) & 0xff);
            }
            else
            {
                //
                //  Fill the buffer with 0xaa to indicate that the data is not
                //  ready yet.
                //
                for (int index = 0; index < sizeof(_txBuffer); index++)
                {
                    _txBuffer[index] = 0xaa;
                }
            }
            _amountToSend = sizeof(_txBuffer);
            break;
        case I2C_DATA_READY:
            _txBuffer[0] = _dataReady ? 1 : 0;
            _amountToSend = 1;
            break;
        case I2C_RESET_RAINFALL_COUNTER:
            _rainGaugePulseCount = 0;
            break;
    }
    _txBufferPointer = 0;
    _processCommand = false;
}

//--------------------------------------------------------------------------------
//
//  ADC End Of Conversion (EOC) interrupt handler.
//
#pragma vector = ADC1_EOC_vector
__interrupt void ADC1_EOC_IRQHandler()
{
    unsigned char low, high;
    unsigned short reading;

    ADC_CR1_ADON = 0;       //  Disable the ADC.
    ADC_CSR_EOC = 0;    	// 	Indicate that ADC conversion is complete.

    low = ADC_DRL;			//	Extract the ADC reading.
    high = ADC_DRH;
    reading = ((high * 256) + low);
    if (ADC_CSR_CH == ADC_UV_SENSOR)
    {
        _uvReading = reading;
        //
        //  Now let the conversion start for the wind direction sensor.
        //
        ADC_CSR_CH = ADC_WIND_DIRECTION_SENSOR;
        ADC_CR1_ADON = 1;
        __no_operation();
        ADC_CR1_ADON = 1;
    }
    else
    {
        _windDirectionReading = reading;
    }
}

//--------------------------------------------------------------------------------
//
//  Timer 1 Overflow handler.
//
//  This handler deals with the 2 second timer pulse for the ESP8266 and also
//  indicates if we should be taking wind speed readings.
//
#pragma vector = TIM1_OVR_UIF_vector
__interrupt void TIM1_UPD_OVF_IRQHandler(void)
{
    _dataReady = true;
    PC_ODR_ODR7 = 1;        //  Tell the ESP8266 that data is ready.
    TIM1_CR1_CEN = 0;       //  Stop Timer 1.
    TIM1_SR1_UIF = 0;       //  Reset the interrupt otherwise it will fire again straight away.
}

//--------------------------------------------------------------------------------
//
//  Process the interrupt on Port D.
//
//  The following devices can generate an interrupt on Port D:
//      - RTC
//      - Wind speed sensor
//      - Rain gauge
//
//  All interrupts will be generateed on the falling edge of a pulse.
//
#pragma vector = EXTI3_vector
__interrupt void EXTI_PORTD_IRQHandler(void)
{
    unsigned char portDInput = PD_IDR;
    //
    //  XORing the current reading with the last reading will leave us with the
    //  changed bits.
    //
    unsigned char changedBits = (portDInput ^ _lastPortDInput);
    _lastPortDInput = portDInput;
    //
    //  Now work out what has changed and act accordingly.
    //
    if (changedBits & MASK_RTC)
    {
        //
        //  RTC has generated an interrupt, generate a short interrupt pulse to
        //  tell the ESP8266 that it is time to read the sensors on the falling
        //	edge of the RTC pulse.
        //
        if (!(portDInput & MASK_RTC))
        {
	        //
    	    //  Start the ADC for the UV and wind direction sensor.
        	//
 	       	ADC_CSR_CH = ADC_UV_SENSOR; //  ADC for the UV sensor first.
    	    ADC_CR1_ADON = 1;			// Reference manual says do this twice.
        	__no_operation();
        	ADC_CR1_ADON = 1;
        	//
        	//	Now kick off the timer for the wind speed reading.
        	//
	        _windSpeedPulseCount = 0;
	        _dataReady = false;
            BitBang(0x01);
		    //
    		//  Force Timer 1 to update without generating an interrupt.
    		//  This is necessary to makes sure we start off with the correct
    		//  count and prescalar values in the Timer 1 registers.
    		//
    		TIM1_CR1_URS = 1;
    		TIM1_EGR_UG = 1;
    		//
    		//  Enable Timer 1
    		//
    		TIM1_CR1_CEN = 1;           //  Start Timer 1.
            PC_ODR_ODR7 = 0;
        }
    }
    if (changedBits & MASK_RAIN_GAUGE)
    {
        //
        //  Increment on the rising edge only.
        //
        BitBang(0x02);
        if (portDInput & MASK_RAIN_GAUGE)
        {
        	BitBang(0x04);
            _rainGaugePulseCount++;
        }
    }
    //
    //  We only increment the wind speed counter when Timer 2 is running,
    //  i.e. during a reset or shortly afterwards when sesnor readings are
    //  being collected.
    //
    if (changedBits & MASK_WIND_SPEED)
    {
        //
        //  Increment on the rising edge only.
        //
        BitBang(0x03);
        if ((portDInput & MASK_WIND_SPEED) && TIM1_CR1_CEN)
        {
        	BitBang(0x05);
            _windSpeedPulseCount++;
        }
    }
}

//--------------------------------------------------------------------------------
//
//  I2C interrupt handler.
//
#pragma vector = I2C_RXNE_vector
__interrupt void I2C_IRQHandler()
{
    unsigned char reg;

    BitBang(0x07);
    if (I2C_SR1_ADDR)
    {
        _txBufferPointer = 0;
        _rxBufferPointer = 0;
        _processCommand = false;
        reg = I2C_SR1;
        reg = I2C_SR3;
        return;
    }
    if (I2C_SR1_RXNE)
    {
        if (_rxBufferPointer < I2C_INPUT_BUFFER_LENGTH)
        {
            reg = I2C_DR;
            _rxBuffer[_rxBufferPointer++] = reg;
        }
        else
        {
            //
            //  If we get here then the calling application has sent too much
            //  data.  We have no where to store this so we will discard it.
            //
            reg = I2C_DR;
        }
        return;
    }
    if (I2C_SR1_TXE)
    {
        unsigned int amount = _amountToSend;
        if (_txBufferPointer < amount)
        {
            reg =  _txBuffer[_txBufferPointer++];
            I2C_DR = reg;
        }
        else
        {
            //
            //  Calling application is requesting too much data, return 0xff
            //  for this and all subsequent requests.
            //
            I2C_SR2_AF = 0;         // End of slave transmission.
            _processCommand = 0;
        }
        return;
    }
    if (I2C_SR2_AF)
    {
        I2C_SR2_AF = 0;             //  End of slave transmission.
        _txBufferPointer = 0;
        return;
    }
    if (I2C_SR1_STOPF)
    {
        _processCommand = true;
        _rxBufferPointer = 0;
        I2C_CR2_STOP = 0;
        reg = I2C_SR1;
        return;
    }
    //
    //  If we get here then we have an error so clear the error by reading the
    //  status registers.
    //
    reg = I2C_SR1;
    reg = I2C_SR2;
    reg = I2C_SR3;
}

//--------------------------------------------------------------------------------
//
//  Main program loop.
//
int main()
{
    __disable_interrupt();
    InitialiseSystemClock();
    InitialiseGPIO();
    _lastPortDInput = PD_IDR;
    InitialiseI2C();
    InitialiseADC();
    InitialiseTimer1();
    _resetCode = 0;
    __enable_interrupt();
    while (1)
    {
        __wait_for_interrupt();
        if (_processCommand)
        {
            ExecuteI2CCommand();
        }
    }
}
