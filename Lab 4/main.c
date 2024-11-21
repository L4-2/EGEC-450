#include <stdint.h>
#include "LCD.h"
#include "TimerA0.h"
#include "PLL.h"
#include "SysTickInts.h"
#include "ADCSWTrigger.h"
#include "tm4c123gh6pm.h"

// mailbox to signal SysTick_Handler (0 = no signal, 1 = signal)
uint16_t mailbox = 0;

// read ADC value
uint32_t adcValue = 0;

// function to translate output from temperature sensor to temperature in Celsius
uint32_t TempSensor(uint32_t data)
{
    // uint32_t celcius = data / 10;

    uint32_t farenheit = (((data * 594) / 4095) + 32) * 10;

    return farenheit;
}

// Initialization function to enable the led on PB0
void PortB_Init(void)
{
    volatile unsigned long delay;
    SYSCTL_RCGC2_R |= 0x00000002;   // 1) B clock
    delay = SYSCTL_RCGC2_R;         // delay
    GPIO_PORTB_CR_R = 0x01;         // allow changes to PB0
    GPIO_PORTB_AMSEL_R = 0x00;      // 3) disable analog on PB
    GPIO_PORTB_PCTL_R = 0x00000000; // 4) PCTL GPIO on PB0
    GPIO_PORTB_DIR_R = 0x01;        // 5) PB0 output
    GPIO_PORTB_AFSEL_R = 0x00;      // 6) disable alt funct on PB7-0
    GPIO_PORTB_DEN_R = 0x01;        // 7) enable digital I/O on PB0
}

void main(void)
{
    PLL_Init(Bus80MHz); // set system clock to 80 MHz

    // Initialize SysTick to sample at 10Hz or 0.1s
    SysTick_Init(800000); // 80MHz / 800000 = 0.1s

    // Initialize LED
    PortB_Init();

    // Initialize LCD
    LCD_Init();

    // Initialize ADC0
    ADC0_InitSWTriggerSeq3_Ch8();

    // while (1)
    // {
    //     adcValue = ADC0_InSeq3();
    //     LCD_OutUFix(TempSensor(adcValue));
    //     TimerA0_Wait10ms(100); // Wait 1s
    //     LCD_Clear();
    // }

    // // simple test to blink LED
    // while (1)
    // {
    //     GPIO_PORTB_DATA_R = 0x01; // LED on
    //     TimerA0_Wait10ms(100);    // Wait 1s
    //     GPIO_PORTB_DATA_R = 0x00; // LED off
    //     TimerA0_Wait10ms(100);    // Wait 1s
    // }

    while (1)
    {
        if (mailbox)
        {
            GPIO_PORTB_DATA_R = 0x01;
        }
        else
        {
            GPIO_PORTB_DATA_R = 0x00;
        }
    }

    // while (1); // Main loop
}
