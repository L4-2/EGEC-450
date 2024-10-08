#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

// State definitions
struct State
{
  uint32_t TF_C;                    // Traffic light control output
  uint32_t time;                    // Time to wait in 10ms units
  const struct State *nextState[4]; // Next state based on input
};

typedef const struct State State_t;

// State identifiers
#define goS &FSM[0]
#define waitS &FSM[1]
#define goW &FSM[2]
#define waitW &FSM[3]

// Finite State Machine definition
State_t FSM[4] = {
    {0x21, 3000, {goS, waitS, goS, waitS}}, // goS state
    {0x22, 500, {goW, goW, goW, goW}},      // waitS state
    {0x0C, 3000, {goW, goW, waitW, waitW}}, // goW state
    {0x14, 500, {goS, goS, goS, goS}}       // waitW state
};

State_t *CurrentState;

void PortD_Init(void)
{
  volatile unsigned long delay;

  SYSCTL_RCGCGPIO_R |= 0x00000008; // 1) activate clock for Port D

  delay = SYSCTL_RCGCGPIO_R; // allow time for clock to start

  GPIO_PORTD_LOCK_R = 0x4C4F434B; // 2) unlock GPIO Port D

  GPIO_PORTD_CR_R = 0xFF; // allow changes to PF4-0

  // only PF0 needs to be unlocked, other bits can't be locked

  GPIO_PORTD_AMSEL_R = 0x00; // 3) disable analog on PF

  GPIO_PORTD_PCTL_R = 0x00000000; // 4) PCTL GPIO on PF4-0

  GPIO_PORTD_DIR_R = 0xFF; // 5) PF4,PF0 in, PF3-1 out

  GPIO_PORTD_AFSEL_R = 0x00; // 6) disable alt funct on PF7-0

  GPIO_PORTD_PUR_R = 0xFF; // enable pull-up on PF0 and PF4

  GPIO_PORTD_DEN_R = 0xFF; // 7) enable digital I/O on PF4-0
}

void PortB_Init(){
  volatile unsigned long delay;

  SYSCTL_RCGCGPIO_R |= 0x00000002; // 1) activate clock for Port B

  delay = SYSCTL_RCGCGPIO_R; // allow time for clock to start

  GPIO_PORTB_LOCK_R = 0x4C4F434B; // 2) unlock GPIO Port B

  GPIO_PORTB_CR_R = 0xFF; // allow changes to PF4-0

  // only PF0 needs to be unlocked, other bits can't be locked

  GPIO_PORTB_AMSEL_R = 0x00; // 3) disable analog on PF

  GPIO_PORTB_PCTL_R = 0x00000000; // 4) PCTL GPIO on PF4-0

  GPIO_PORTB_DIR_R = 0xFF; // 5) PF4,PF0 in, PF3-1 out

  GPIO_PORTB_AFSEL_R = 0x00; // 6) disable alt funct on PF7-0

  GPIO_PORTB_PUR_R = 0x00; // enable pull-up on PF0 and PF4

  GPIO_PORTB_DEN_R = 0xFF; // 7) enable digital I/O on PF4-0
}

void main(void)
{

  PLL_Init(Bus80MHz); // set system clock to 80 MHz

  SysTick_Init(); // initialize SysTick timer

  PortD_Init(); // initialize Port D

  // uint16_t i = 0;

  // for(i = 0; i < 100; i++)
  // {
  //   SysTick_Wait10ms(10);
  //   GPIO_PORTD_DATA_R ^= 0xFF;
  // }

  // infinetly cascade leds
  while (1)
  {
    GPIO_PORTD_DATA_R = 0x01;
    SysTick_Wait10ms(1);
    GPIO_PORTD_DATA_R = 0x02;
    SysTick_Wait10ms(1);
    GPIO_PORTD_DATA_R = 0x04;
    SysTick_Wait10ms(1);
    GPIO_PORTD_DATA_R = 0x08;
    SysTick_Wait10ms(1);
    GPIO_PORTD_DATA_R = 0x10;
    SysTick_Wait10ms(1);
    GPIO_PORTD_DATA_R = 0x20;
    SysTick_Wait10ms(1);
    GPIO_PORTD_DATA_R = 0x40;
    SysTick_Wait10ms(1);
    GPIO_PORTD_DATA_R = 0x80;
    SysTick_Wait10ms(1);
  }
}
