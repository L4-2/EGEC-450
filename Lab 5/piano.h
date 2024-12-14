#ifndef PIANO_H
#define PIANO_H

#include <stdint.h>

// Initializes the PIANO by setting up the GPIO pins
void Piano_Init(void);

// Outputs a digital value to the PIANO
uint8_t Piano_Out(void);

#endif // PIANO_H
