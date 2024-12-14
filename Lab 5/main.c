// #include <stdio.h>
// #include <stdint.h>
// #include <math.h>
// #include "PLL.h"
// #include "DAC.h"
// #include "tm4c123gh6pm.h"
// #include "Piano.h"

// #define PI 3.14159265
// #define SINE_TABLE_SIZE 32

// void Delay(uint32_t delay);

// // Sine wave lookup table
// const uint8_t sineTable[SINE_TABLE_SIZE] = {
//   8, 9, 11, 12, 13, 14, 15, 15, 15, 14, 13, 12, 11, 9, 8, 6,
//   4, 3, 2, 1, 0, 0, 0, 1, 2, 3, 4, 6, 8, 9, 11, 12
// };

// int main(void)
// {
//   uint8_t index = 0;
//   uint8_t key;

//   PLL_Init(Bus80MHz);  // set system clock to 80 MHz
//   DAC_Init();
//   Piano_Init();

//   while(1) {
//     key = Piano_Out();  // Read the button states

//     switch(key) {
//       case 0x01:  // PF0 pressed
//         Delay(1000);  // Higher frequency
//         break;
//       case 0x02:  // PF1 pressed
//         Delay(2000);  // Medium frequency
//         break;
//       case 0x04:  // PF2 pressed
//         Delay(3000);  // Lower frequency
//         break;
//       default:  // No button pressed or multiple buttons pressed
//         Delay(5000);  // Lowest frequency or no sound
//         break;
//     }

//     DAC_Out(sineTable[index]);
//     index = (index + 1) % SINE_TABLE_SIZE;  // Increment and wrap around the index
//   }
// }

// void Delay(uint32_t delay) {
//   volatile unsigned long i;
//   for (i = 0; i < delay; i++);  // Simple delay loop
// }

#include <stdio.h>
#include <stdint.h>
#include "PLL.h"
#include "DAC.h"
#include "tm4c123gh6pm.h"
#include "Piano.h"
#include "Sound.h"
#include "Timer0A.h"

uint32_t volatile Test;

// Note frequencies
#define NOTE_C 9400 // 261.63 Hz
#define NOTE_D 4257 // 293.66 Hz
#define NOTE_E 3792 // 329.63 Hz
#define NOTE_F 3579 // 349.23 Hz
#define NOTE_G 3189 // 392.00 Hz
#define NOTE_A 2841 // 440.00 Hz
#define NOTE_B 2531 // 493.88 Hz

int main(void)
{
  uint8_t key;

  PLL_Init(Bus80MHz); // set system clock to 80 MHz
  Sound_Init();       // Initialize the sound driver
  Piano_Init();       // Initialize the piano keys

  while (1)
  {
    key = Piano_Out(); // Read the button states

    switch (key)
    {
    case 0x01: // PF0 pressed
      Sound_Play(NOTE_C);
      break;
    case 0x02: // PF1 pressed
      Sound_Play(NOTE_E);
      break;
    case 0x04: // PF2 pressed
      Sound_Play(NOTE_G);
      break;
    default:               // No button pressed or multiple buttons pressed
      Sound_Play(8000000); // Stop sound
      break;
    }
  }
}
