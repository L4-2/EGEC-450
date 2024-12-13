#ifndef DAC_H
#define DAC_H

#include <stdint.h>

// Initialize the DAC
void DAC_Init(void);

// Output to DAC
void DAC_Out(uint8_t data);

#endif // DAC_H