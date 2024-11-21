// SysTickInts.c
// Edited to run on Tiva-C
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// June 1, 2015

/* This example accompanies the books
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1 Program 9.7

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "ADCSWTrigger.h"
#include "tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

extern volatile uint16_t mailbox;

volatile uint32_t ADCvalue;

// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns (assuming 80 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
volatile uint32_t Counts;
uint32_t wait_per;

void 	SysTick_Init(uint32_t period) {
	long sr = StartCritical();
	wait_per = period;

	ADC0_InitSWTriggerSeq3_Ch8();        // initialize ADC sample PE5/A8

	Counts = 0;

	NVIC_ST_CTRL_R = 0;                  // disable SysTick during setup
	NVIC_ST_RELOAD_R = period - 1;       // maximum reload value
	NVIC_ST_CURRENT_R = 0;               // any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000;	// priority 2
	NVIC_ST_CTRL_R = 0x00000007;         // enable SysTick with interrupts

	EnableInterrupts();

	EndCritical(sr);
}

void SysTick_Handler() {
	ADCvalue = ADC0_InSeq3();

	// toggle mailbox variable from 0 to 1 or 1 to 0
	mailbox ^= 1;

}

uint32_t SysTick_Mailbox() {
	return ADCvalue;
}
