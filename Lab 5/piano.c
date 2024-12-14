#include <stdint.h>
#include "tm4c123gh6pm.h"

// Piano_Init initializes the GPIO pins for the piano keys
void Piano_Init(void) {
    // Enable the clock for Port F
    SYSCTL_RCGCGPIO_R |= 0x20;
    // Allow time for the clock to start
    while ((SYSCTL_PRGPIO_R & 0x20) == 0) {};
    // Unlock PF0 for use
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R |= 0x07;
    // Set PF0, PF1, and PF2 as input
    GPIO_PORTF_DIR_R &= ~0x07;
    // Enable digital function for PF0, PF1, and PF2
    GPIO_PORTF_DEN_R |= 0x07;
    // Enable pull-up resistors on PF0, PF1, and PF2
    GPIO_PORTF_PUR_R |= 0x07;
}

// Piano_Out returns a logical key code for the pattern of switches that are pressed
uint8_t Piano_Out(void) {
    // Read the state of PF0, PF1, and PF2
    return (GPIO_PORTF_DATA_R & 0x07);
}
