#ifndef DAC_H
#define DAC_H

#include <stdint.h>

// Initializes the DAC by setting up the GPIO pins
void DAC_Init(void);

// Outputs a digital value to the DAC
void DAC_Out(uint8_t value);

#endif // DAC_H
