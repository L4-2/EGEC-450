#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>


// Initialize the sound driver
void Sound_Init(void);

// Play a specific note
void Sound_Play(uint32_t note);

#endif // SOUND_H
