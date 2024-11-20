#include <stdint.h>
#include "LCD.h"
#include "TimerA0.h"
#include "PLL.h"
#include "ADCSWTrigger.h"

// function to translate output from temperature sensor to temperature in Celsius
uint32_t TempSensor(uint32_t data)
{
    uint32_t celcius = data / 0.01;

    uint32_t farenheit = (celcius * 9 / 5) + 32;

    return farenheit;
}

void main()
{
    PLL_Init(Bus80MHz); // set system clock to 80 MHz

    // Initialize LCD
    LCD_Init();

    // Initialize ADC0
    ADC0_InitSWTriggerSeq3_Ch8();

    // A
    // LCD_OutString("Test");
    // TimerA0_Wait10ms(100); // Wait 1s

    // B
    // LCD_OutChar(0x42);
    // TimerA0_Wait10ms(100);      // Wait 1s

    // C
    // LCD_OutChar(0x43);
    // TimerA0_Wait10ms(100);      // Wait 1s

    // Move cursor to the 2nd line
    // LCD_OutCmd(0xC0);

    // 1
    // LCD_OutUFix(9999);
    // TimerA0_Wait10ms(100); // Wait 1s

    // 2
    // LCD_OutChar(0x32);
    // TimerA0_Wait10ms(100);      // Wait 1s

    // 3
    // LCD_OutChar(0x33);

    // read ADC value
    uint32_t adcValue;

    while (1)
    {
        adcValue = TempSensor(ADC0_InSeq3());
        LCD_OutUDec(adcValue);
        TimerA0_Wait10ms(10); // Wait 1s
        LCD_Clear();
    }

    // while (1); // Main loop
}
