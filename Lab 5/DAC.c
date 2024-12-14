#include <stdint.h>
#include "tm4c123gh6pm.h"

// DAC_Init initializes the DAC by setting up the GPIO pins
void DAC_Init(void) {
    // Enable the clock for Port D
    SYSCTL_RCGCGPIO_R |= 0x08;
    // Allow time for the clock to start
    while ((SYSCTL_PRGPIO_R & 0x08) == 0) {};
    // Set PD0-PD3 as digital output
    GPIO_PORTD_DIR_R |= 0x0F;
    GPIO_PORTD_DEN_R |= 0x0F;
}

// DAC_Out outputs a digital value to the DAC
void DAC_Out(uint8_t value) {
    // Write the value to PD0-PD3
    GPIO_PORTD_DATA_R = (GPIO_PORTD_DATA_R & ~0x0F) | (value & 0x0F);
}

