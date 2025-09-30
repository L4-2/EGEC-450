#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

// Shift Register Connections

#define SER 0x01   // PD0
#define RCLK 0x02  // PD1
#define SRCLK 0x04 // PD2
#define SRCLR 0x08 // PD3

// input variables

uint32_t input;

// State definitions

struct State
{
  uint32_t output; // Traffic light control output

  uint32_t time; // Time to wait in 10ms units

  const struct State* next[8]; // Next state based on input
};

typedef const struct State SType;

// Led output definitions

#define GoS 0x4C
#define WaitS 0x4A
#define GoW 0x61
#define WaitW 0x51
#define pedWalk 0x89
#define pedNoWalk 0x49
#define pedHurry1 0x49
#define pedHurry2 0x09

// State identifiers

#define goS &FSM[0]
#define waitS &FSM[1]
#define goW &FSM[2]
#define waitW &FSM[3]
#define pedRequest1GoS &FSM[4]
#define pedRequest2GoS &FSM[5]
#define pedRequest1WaitS &FSM[6]
#define pedRequest2WaitS &FSM[7]
#define pedRequest1GoW &FSM[8]
#define pedRequest2GoW &FSM[9]
#define pedRequest1WaitW &FSM[10]
#define pedRequest2WaitW &FSM[11]
#define pedWaitS &FSM[12]
#define pedWaitW &FSM[13]
#define pedWalkS &FSM[14]
#define pedWalkW &FSM[15]
#define pedHurryS1 &FSM[16]
#define pedHurryS2 &FSM[17]
#define pedHurryS3 &FSM[18]
#define pedHurryS4 &FSM[19]
#define pedHurryS5 &FSM[20]
#define pedHurryS6 &FSM[21]
#define pedNoWalkS &FSM[22]
#define pedHurryW1 &FSM[23]
#define pedHurryW2 &FSM[24]
#define pedHurryW3 &FSM[25]
#define pedHurryW4 &FSM[26]
#define pedHurryW5 &FSM[27]
#define pedHurryW6 &FSM[28]
#define pedNoWalkW &FSM[29]

// Finite State Machine definition, [000, 001, 010, 011, 100, 101, 110, 111] for next states, where MSB is the pedestrian button and LSB is West button
SType FSM[30] = {
    {GoS, 250, {goS, waitS, goS, waitS, pedRequest1GoS, pedRequest1WaitS, pedRequest1GoS, pedRequest1WaitS}},           // goS state
    {WaitS, 50, {goW, goW, goW, goW, pedRequest1GoW, pedRequest1GoW, pedRequest1GoW, pedRequest1GoW}},                  // waitS state
    {GoW, 250, {goW, goW, waitW, waitW, pedRequest1GoW, pedRequest1GoW, pedRequest1WaitW, pedRequest1WaitW}},           // goW state
    {WaitW, 50, {goS, goS, goS, goS, pedRequest1GoS, pedRequest1GoS, pedRequest1GoS, pedRequest1GoS}},                  // waitW
    {GoS, 300, {goS, waitS, goS, waitS, pedRequest2GoS, pedRequest2WaitS, pedRequest2GoS, pedRequest2WaitS}},           // pedRequest1GoS
    {GoS, 300, {pedWaitS, pedWaitS, pedWaitS, pedWaitS, pedWaitS, pedWaitS, pedWaitS, pedWaitS}},                       // pedRequest2GoS
    {WaitS, 50, {goW, goW, goW, goW, pedRequest2GoW, pedRequest2GoW, pedRequest2GoW, pedRequest2GoW}},                  // pedRequest1WaitS
    {WaitS, 50, {pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS}},                      // pedRequest2WaitS
    {GoW, 300, {goW, goW, waitW, waitW, pedRequest2GoW, pedRequest2GoW, pedRequest2WaitW, pedRequest2WaitW}},           // pedRequest1GoW
    {GoW, 300, {pedWaitW, pedWaitW, pedWaitW, pedWaitW, pedWaitW, pedWaitW, pedWaitW, pedWaitW}},                       // pedRequest2GoW
    {WaitW, 50, {goS, goS, goS, goS, pedRequest2GoS, pedRequest2GoS, pedRequest2GoS, pedRequest2GoS}},                  // pedRequest1WaitW
    {WaitW, 50, {pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW}},                      // pedRequest2WaitW
    {WaitS, 50, {pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS, pedWalkS}},                      // pedWaitS
    {WaitW, 50, {pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW, pedWalkW}},                      // pedWaitW
    {pedWalk, 600, {pedHurryS1, pedHurryS1, pedHurryS1, pedHurryS1, pedHurryS1, pedHurryS1, pedHurryS1, pedHurryS1}},   // pedWalkS
    {pedWalk, 600, {pedHurryW1, pedHurryW1, pedHurryW1, pedHurryW1, pedHurryW1, pedHurryW1, pedHurryW1, pedHurryW1}},   // pedWalkW
    {pedHurry1, 100, {pedHurryS2, pedHurryS2, pedHurryS2, pedHurryS2, pedHurryS2, pedHurryS2, pedHurryS2, pedHurryS2}}, // pedHurryS1
    {pedHurry2, 100, {pedHurryS3, pedHurryS3, pedHurryS3, pedHurryS3, pedHurryS3, pedHurryS3, pedHurryS3, pedHurryS3}}, // pedHurryS2
    {pedHurry1, 100, {pedHurryS4, pedHurryS4, pedHurryS4, pedHurryS4, pedHurryS4, pedHurryS4, pedHurryS4, pedHurryS4}}, // pedHurryS3
    {pedHurry2, 100, {pedHurryS5, pedHurryS5, pedHurryS5, pedHurryS5, pedHurryS5, pedHurryS5, pedHurryS5, pedHurryS5}}, // pedHurryS4
    {pedHurry1, 100, {pedHurryS6, pedHurryS6, pedHurryS6, pedHurryS6, pedHurryS6, pedHurryS6, pedHurryS6, pedHurryS6}}, // pedHurryS5
    {pedHurry2, 100, {pedNoWalkS, pedNoWalkS, pedNoWalkS, pedNoWalkS, pedNoWalkS, pedNoWalkS, pedNoWalkS, pedNoWalkS}}, // pedHurryS6
    {pedNoWalk, 300, {goS, goS, goS, goS, goS, goS, goS, goS}},                                                         // pedNoWalkS
    {pedHurry1, 100, {pedHurryW2, pedHurryW2, pedHurryW2, pedHurryW2, pedHurryW2, pedHurryW2, pedHurryW2, pedHurryW2}}, // pedHurryW1
    {pedHurry2, 100, {pedHurryW3, pedHurryW3, pedHurryW3, pedHurryW3, pedHurryW3, pedHurryW3, pedHurryW3, pedHurryW3}}, // pedHurryW2
    {pedHurry1, 100, {pedHurryW4, pedHurryW4, pedHurryW4, pedHurryW4, pedHurryW4, pedHurryW4, pedHurryW4, pedHurryW4}}, // pedHurryW3
    {pedHurry2, 100, {pedHurryW5, pedHurryW5, pedHurryW5, pedHurryW5, pedHurryW5, pedHurryW5, pedHurryW5, pedHurryW5}}, // pedHurryW4
    {pedHurry1, 100, {pedHurryW6, pedHurryW6, pedHurryW6, pedHurryW6, pedHurryW6, pedHurryW6, pedHurryW6, pedHurryW6}}, // pedHurryW5
    {pedHurry2, 100, {pedNoWalkW, pedNoWalkW, pedNoWalkW, pedNoWalkW, pedNoWalkW, pedNoWalkW, pedNoWalkW, pedNoWalkW}}, // pedHurryW6
    {pedNoWalk, 300, {goW, goW, goW, goW, goW, goW, goW, goW}}                                                          // pedNoWalkW
};

// State pointer
SType *Pt;

// Setup for Port D to enable PD0, PD1, PD2, and PD3 as outputs
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

// Setup for Port F to enable PF1, PF2, and PF3 as outputs
void PortF_Init(void)
{
  volatile unsigned long delay;

  SYSCTL_RCGCGPIO_R |= 0x20; // 1) activate clock for Port F

  delay = SYSCTL_RCGCGPIO_R; // allow time for clock to start

  GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) unlock GPIO Port F

  GPIO_PORTF_CR_R = 0x0E; // allow changes to PF1, PF2, and PF3

  GPIO_PORTF_AMSEL_R &= ~0x0E; // 3) disable analog on PF1, PF2, and PF3

  GPIO_PORTF_PCTL_R &= ~0x0000FFF0; // 4) PCTL GPIO on PF1, PF2, and PF3

  GPIO_PORTF_DIR_R |= 0x0E; // 5) PF1, PF2, and PF3 out

  GPIO_PORTF_AFSEL_R &= ~0x0E; // 6) disable alt funct on PF1, PF2, and PF3

  GPIO_PORTF_DEN_R |= 0x0E; // 7) enable digital I/O on PF1, PF2, and PF3
}

// Shift data to the shift register
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

// Latch the data to the shift register
void ShiftRegister_LatchData(void)
{
  GPIO_PORTD_DATA_R |= RCLK;  // set PD1 (RCLK) high
  GPIO_PORTD_DATA_R &= ~RCLK; // set PD1 (RCLK) low
}

// Clear the shift register
void ShiftRegister_Clear(void)
{
  GPIO_PORTD_DATA_R &= ~SRCLR; // set PD3 (~SRCLR) low to clear the shift register
  GPIO_PORTD_DATA_R |= SRCLR;  // set PD3 (~SRCLR) high to enable the shift register
}

// Set the RGB LED on Port F
void SetRGBLED(uint8_t data)
{
  // Extract the two leftmost bits
  uint8_t rgb = (data & 0xC0) >> 6;

  // Clear the RGB LED bits
  GPIO_PORTF_DATA_R &= ~0x0E;

  // Set the RGB LED bits based on the extracted value
  GPIO_PORTF_DATA_R |= (rgb << 1);
}

void main(void)
{

  PLL_Init(Bus80MHz); // set system clock to 80 MHz

  SysTick_Init(); // initialize SysTick timer

  PortD_Init(); // initialize Port D

  PortA_Init(); // initialize Port B

  PortF_Init(); // initialize Port F

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

    // set the RGB LED
    SetRGBLED(Pt->output);

    // wait for the specified time
    SysTick_Wait10ms(Pt->time);

    // take input from the buttons
    input = (GPIO_PORTA_DATA_R & 0xE0) >> 5; // Read PA5, PA6, and PA7 as a 3-bit input

    // transition to the next state based on the input
    Pt = Pt->next[input];
  }
}
