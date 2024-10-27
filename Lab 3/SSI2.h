// Adapted from Professor Mazidi's code at microdigitaled.com
//   cited at EduBase-V2 trainer website

#ifndef __SSI2_H__
#define __SSI2_H__

// SPI functions for Tiva-C SSI2 module on EduBase-V2 board
// Pinout: MOSI - PB7
//         MISO - PB6 (not used)
//         SCLK - PB4
//         CS on PortC

// enable SSI2 and associated GPIO pins
// note: you must initialize your CS pin separately
void SSI2_init(void);

// enables chip select (using mask), writes one byte to SSI2,
// waits for transmit to complete, and deasserts chip select (using mask)
void SSI2_write( uint8_t data, uint8_t csMask );


// helper function to display a digit on the 7-segment display
void displayDigit( uint8_t digit, uint8_t display, uint8_t decimal );

#endif
