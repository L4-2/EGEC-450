#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "DAC.h"
#include "Sound.h"
#include "Timer0A.h"
#include "PLL.h"
#include "SysTickInts.h"

#define SINE_TABLE_SIZE 32

volatile uint32_t Note;

// #define NOTE_C 4778 // 261.63 Hz
// #define NOTE_D 4257 // 293.66 Hz
// #define NOTE_E 3792 // 329.63 Hz
// #define NOTE_F 3579 // 349.23 Hz
// #define NOTE_G 3189 // 392.00 Hz
// #define NOTE_A 2841 // 440.00 Hz
// #define NOTE_B 2531 // 493.88 Hz

// Initialize the sound driver
void Sound_Init(void)
{
    DAC_Init();
    Timer0A_Init(80000000);
}

// Play a specific note
void Sound_Play(uint32_t note)
{
    Note = note;
}
