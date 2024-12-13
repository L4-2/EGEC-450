#include <stdint.h>
#include "DAC.h"
#include "tm4c123gh6pm.h"

// Initialize the DAC
void DAC_Init(void) {
    // Enable clock for Port D
    SYSCTL_RCGCGPIO_R |= 0x08;
    // Allow time for clock to stabilize
    while((SYSCTL_PRGPIO_R & 0x08) == 0) {};
    // Set PD0-PD3 as output
    GPIO_PORTD_DIR_R |= 0x0F;
    // Enable digital function for PD0-PD3
    GPIO_PORTD_DEN_R |= 0x0F;
}

// Output to DAC
void DAC_Out(uint8_t data) {
    // Mask the data to only use the lower 4 bits
    data &= 0x0F;
    // Output the data to PD0-PD3
    GPIO_PORTD_DATA_R = (GPIO_PORTD_DATA_R & ~0x0F) | data;
}