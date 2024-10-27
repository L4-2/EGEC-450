// Timer1A.h
// Runs on Tiva-C

// Adapted from SysTick.h from the book:
/* "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
*/

#ifndef __TIMER1A_H__
#define __TIMER1A_H__

// Set clock freq. so Timer0A_Wait10ms delays for exactly 10 ms if clock is not 80 MHz
void Timer1A_Init( uint32_t clkFreq );

// Timer1 handler
void Timer1A_Handler(void);

// Time delay using busy wait
// The delay parameter is in units of the core clock (units of 12.5 nsec for 80 MHz clock)
void Timer1A_Wait( uint32_t delay );

// Time delay using busy wait
// This assumes 80 MHz system clock
void Timer1A_Wait1ms( uint32_t delay );

#endif
