#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

// Define control pins
#define SER 0x01   // PD0
#define RCLK 0x02  // PD1
#define SRCLK 0x04 // PD2
#define SRCLR 0x08 // PD3

uint32_t previousInput = 0xFF; // Initialize to a value that is not a valid input

// input variables
uint32_t input;
uint32_t pedButtonTimer = 0;

// State definitions
struct State
{
  uint32_t output;             // Traffic light control output
  uint32_t time;               // Time to wait in 10ms units
  const struct State *next[8]; // Next state based on input
};

typedef const struct State SType;

// State identifiers
#define goS &FSM[0]
#define waitS &FSM[1]
#define goW &FSM[2]
#define waitW &FSM[3]
#define pedWait &FSM[4]
#define pedWalk &FSM[5]
#define pedDontWalk &FSM[6]

// Finite State Machine definition
SType FSM[7] = {
    {0x4C, 250, {goS, waitS, goS, waitS, pedWait, pedWait, pedWait, pedWait}},                                              // goS state
    {0x4A, 50, {goW, goW, goW, goW, pedWait, pedWait, pedWait, pedWait}},                                                   // waitS state
    {0x61, 250, {goW, goW, waitW, waitW, pedWait, pedWait, pedWait, pedWait}},                                              // goW state
    {0x51, 50, {goS, goS, goS, goS, pedWait, pedWait, pedWait, pedWait}},                                                   // waitW state
    {0x52, 300, {pedWalk, pedWalk, pedWalk, pedWalk, pedWalk, pedWalk, pedWalk, pedWalk}},                                  // pedWait state
    {0x89, 1000, {pedDontWalk, pedDontWalk, pedDontWalk, pedDontWalk, pedDontWalk, pedDontWalk, pedDontWalk, pedDontWalk}}, // pedWalk state
    {0x51, 100, {goS, goS, goS, goS, goS, goS, goS, goS}}                                                                   // pedDontWalk state
};

// State pointer
SType *Pt;

void PortD_Init(void)
{
  volatile unsigned long delay;

  SYSCTL_RCGCGPIO_R |= 0x00000008; // 1) activate clock for Port D

  delay = SYSCTL_RCGCGPIO_R; // allow time for clock to start

  GPIO_PORTD_LOCK_R = 0x4C4F434B; // 2) unlock GPIO Port D

  GPIO_PORTD_CR_R = 0x0F; // allow changes to PD0, PD1, PD2, PD3

  GPIO_PORTD_AMSEL_R &= ~0x0F; // 3) disable analog on PD0, PD1, PD2, PD3

  GPIO_PORTD_PCTL_R &= ~0x0F; // 4) PCTL GPIO on PD0, PD1, PD2, PD3

  GPIO_PORTD_DIR_R |= 0x0F; // 5) PD0, PD1, PD2, PD3 out

  GPIO_PORTD_AFSEL_R &= ~0x0F; // 6) disable alt funct on PD0, PD1, PD2, PD3

  GPIO_PORTD_DEN_R |= 0x0F; // 7) enable digital I/O on PD0, PD1, PD2, PD3
}

// Setup for Port A to enable PA6 and PA7 as inputs
void PortA_Init(void)
{
  volatile unsigned long delay;

  SYSCTL_RCGCGPIO_R |= 0x01; // 1) activate clock for Port A

  delay = SYSCTL_RCGCGPIO_R; // allow time for clock to start

  GPIO_PORTA_LOCK_R = 0x4C4F434B; // 2) unlock GPIO Port A

  GPIO_PORTA_CR_R = 0xE0; // allow changes to PA5, PA6, and PA7

  GPIO_PORTA_AMSEL_R &= ~0xE0; // 3) disable analog on PA5, PA6, and PA7

  GPIO_PORTA_PCTL_R &= ~0xFFF00000; // 4) PCTL GPIO on PA5, PA6, and PA7

  GPIO_PORTA_DIR_R &= ~0xE0; // 5) PA5, PA6, and PA7 as inputs

  GPIO_PORTA_AFSEL_R &= ~0xE0; // 6) disable alt funct on PA5, PA6, and PA7

  GPIO_PORTA_DEN_R |= 0xE0; // 7) enable digital I/O on PA5, PA6, and PA7
}

void ShiftRegister_ShiftData(uint8_t data)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    if (data & 0x80) // check the MSB
    {
      GPIO_PORTD_DATA_R |= SER; // set PD0 (SER) high
    }
    else
    {
      GPIO_PORTD_DATA_R &= ~SER; // set PD0 (SER) low
    }
    data <<= 1; // shift data left

    GPIO_PORTD_DATA_R |= SRCLK;  // set PD2 (SRCLK) high
    GPIO_PORTD_DATA_R &= ~SRCLK; // set PD2 (SRCLK) low
  }
}

void ShiftRegister_LatchData(void)
{
  GPIO_PORTD_DATA_R |= RCLK;  // set PD1 (RCLK) high
  GPIO_PORTD_DATA_R &= ~RCLK; // set PD1 (RCLK) low
}

void ShiftRegister_Clear(void)
{
  GPIO_PORTD_DATA_R &= ~SRCLR; // set PD3 (~SRCLR) low to clear the shift register
  GPIO_PORTD_DATA_R |= SRCLR;  // set PD3 (~SRCLR) high to enable the shift register
}

void main(void)
{

  PLL_Init(Bus80MHz); // set system clock to 80 MHz

  SysTick_Init(); // initialize SysTick timer

  PortD_Init(); // initialize Port D

  PortA_Init(); // initialize Port B

  // clear the shift register first
  ShiftRegister_Clear();

  // set the initial state
  Pt = goS;

  // PD6 and PD7 are the buttons
  while (1)
  {
    // output pattern to leds
    ShiftRegister_ShiftData(Pt->output);

    // latch the data to the leds
    ShiftRegister_LatchData();

    // wait for the specified time
    SysTick_Wait10ms(Pt->time);

    // take input from the buttons
    input = (GPIO_PORTA_DATA_R & 0xE0) >> 5; // Read PA5, PA6, and PA7 as a 3-bit input

    // Check if the pedestrian button (PA7) is held for more than 2 seconds
    if (input & 0x04) // Check if PA7 is high
    {
      pedButtonTimer++;
      if (pedButtonTimer >= 400) // 200 * 10ms = 2000ms = 2 seconds
      {
        input = 0x04;       // Set input to trigger pedestrian walk cycle
        pedButtonTimer = 0; // Reset the timer
      }
    }
    else
    {
      pedButtonTimer = 0; // Reset the timer if the button is not held
    }

    // transition to the next state based on the input
    Pt = Pt->next[input];
  }
}
