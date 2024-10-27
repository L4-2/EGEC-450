#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include "SSI2.h"
#include "Timer0A.h"
#include "Timer1A.h"

// variables for stopwatch
volatile uint32_t milliseconds = 0;    // Stopwatch time in milliseconds
volatile uint8_t stopwatchValues[4];   // Digits to display
volatile uint8_t stopwatchRunning = 0; // Flag to indicate if the stopwatch is running
volatile uint32_t prevSwitches = 0;    // Previous switch state

// Setup for Port B to enable PB0, PB1, PB2, and PB3 as outputs for leds below 7-segment display
void PortB_Init(void)
{
  SYSCTL_RCGCGPIO_R |= 0x02; // 1) activate clock for Port B
  while ((SYSCTL_PRGPIO_R & 0x02) == 0)
  {
  };                                // allow time for clock to start
  GPIO_PORTB_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port B
  GPIO_PORTB_CR_R = 0x0F;           // allow changes to PB0, PB1, PB2, PB3
  GPIO_PORTB_AMSEL_R &= ~0x0F;      // 3) disable analog on PB0, PB1, PB2, PB3
  GPIO_PORTB_PCTL_R &= ~0x0000FFFF; // 4) PCTL GPIO on PB0, PB1, PB2, PB3
  GPIO_PORTB_DIR_R |= 0x0F;         // 5) PB0, PB1, PB2, PB3 as outputs
  GPIO_PORTB_AFSEL_R &= ~0x0F;      // 6) disable alt funct on PB0, PB1, PB2, PB3
  GPIO_PORTB_DEN_R |= 0x0F;         // 7) enable digital I/O on PB0, PB1, PB2, PB3
}

// Setup for Port A to enable PA6 and PA7 as inputs
void PortA_Init(void)
{
  volatile unsigned long delay;

  SYSCTL_RCGCGPIO_R |= 0x01;        // 1) activate clock for Port A
  delay = SYSCTL_RCGCGPIO_R;        // allow time for clock to start
  GPIO_PORTA_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port A
  GPIO_PORTA_CR_R = 0xE0;           // allow changes to PA5, PA6, and PA7
  GPIO_PORTA_AMSEL_R &= ~0xE0;      // 3) disable analog on PA5, PA6, and PA7
  GPIO_PORTA_PCTL_R &= ~0xFFF00000; // 4) PCTL GPIO on PA5, PA6, and PA7
  GPIO_PORTA_DIR_R &= ~0xE0;        // 5) PA5, PA6, and PA7 as inputs
  GPIO_PORTA_AFSEL_R &= ~0xE0;      // 6) disable alt funct on PA5, PA6, and PA7
  GPIO_PORTA_DEN_R |= 0xE0;         // 7) enable digital I/O on PA5, PA6, and PA7
}

// Setup for Port F to enable PF2, PF3, and PF4 as inputs for buttons
void PortF_Init(void)
{
  volatile unsigned long delay;

  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  delay = SYSCTL_RCGCGPIO_R;        // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1C;           // allow changes to PF2, PF3, PF4
  GPIO_PORTF_AMSEL_R &= ~0x1C;      // 3) disable analog on PF2, PF3, PF4
  GPIO_PORTF_PCTL_R &= ~0x000FFF00; // 4) PCTL GPIO on PF2, PF3, PF4
  GPIO_PORTF_DIR_R &= ~0x1C;        // 5) PF2, PF3, PF4 as inputs
  GPIO_PORTF_AFSEL_R &= ~0x1C;      // 6) disable alt funct on PF2, PF3, PF4
  GPIO_PORTF_DEN_R |= 0x1C;         // 7) enable digital I/O on PF2, PF3, PF4

  // Configure interrupts for PF2, PF3, PF4
  GPIO_PORTF_IS_R &= ~0x1C;                              // PF2, PF3, PF4 are edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x1C;                             // PF2, PF3, PF4 are not both edges
  GPIO_PORTF_IEV_R &= ~0x1C;                             // PF2, PF3, PF4 falling edge event
  GPIO_PORTF_ICR_R = 0x1C;                               // clear flags
  GPIO_PORTF_IM_R |= 0x1C;                               // arm interrupt on PF2, PF3, PF4
  NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000; // priority 5
  NVIC_EN0_R |= 0x40000000;                              // enable interrupt 30 in NVIC
}

// initialize 7-segment display
void sevenseg_init(void)
{
  SYSCTL_RCGCGPIO_R |= 0x02; // enable clock to GPIOB
  SYSCTL_RCGCGPIO_R |= 0x04; // enable clock to GPIOC
  SYSCTL_RCGCSSI_R |= 0x04;  // enable clock to SSI2

  // PORTB 7, 4 for SSI2 TX and SCLK
  GPIO_PORTB_AMSEL_R &= ~0x90;      // turn off analog of PORTB 7, 4
  GPIO_PORTB_AFSEL_R |= 0x90;       // PORTB 7, 4 for alternate function
  GPIO_PORTB_PCTL_R &= ~0xF00F0000; // clear functions for PORTB 7, 4
  GPIO_PORTB_PCTL_R |= 0x20020000;  // PORTB 7, 4 for SSI2 function
  GPIO_PORTB_DEN_R |= 0x90;         // PORTB 7, 4 as digital pins

  // PORTC 7 for SSI2 slave select
  GPIO_PORTC_AMSEL_R &= ~0x80; // disable analog of PORTC 7
  GPIO_PORTC_DATA_R |= 0x80;   // set PORTC 7 idle high
  GPIO_PORTC_DIR_R |= 0x80;    // set PORTC 7 as output for SS
  GPIO_PORTC_DEN_R |= 0x80;    // set PORTC 7 as digital pin

  SSI2_CR1_R = 0;      // turn off SSI2 during configuration
  SSI2_CC_R = 0;       // use system clock
  SSI2_CPSR_R = 16;    // clock prescaler divide by 16 gets 1 MHz clock
  SSI2_CR0_R = 0x0007; // clock rate div by 1, phase/polarity 0 0, mode freescale, data size 8
  SSI2_CR1_R = 2;      // enable SSI2 as master
}

// Interrupt handler for Port F
void GPIOPortF_Handler(void)
{

  // Read the current state of the switches
  uint32_t switches = GPIO_PORTF_DATA_R & 0x1C; // Read PF2, PF3, PF4

  // Detect edge transition from not pressed to pressed
  uint32_t pressed = switches & ~prevSwitches;

  if (pressed & 0x04) // Start/Stop
  {
    GPIO_PORTF_ICR_R = 0x04; // acknowledge flag0
    stopwatchRunning = !stopwatchRunning;
    // SysTick_Wait10ms(10); // Debounce
  }
  if (pressed & 0x08) // Increment by 1ms only if the stopwatch is stopped
  {
    uint8_t state = GPIO_PORTF_DATA_R & 0x08;
    GPIO_PORTF_ICR_R = 0x08; // acknowledge flag1
    if (!stopwatchRunning)
    {
      milliseconds++;
      // Update the display values array with new time
      uint32_t newtime = milliseconds;
      stopwatchValues[3] = newtime % 10; // Milliseconds (0.001)
      newtime /= 10;
      stopwatchValues[2] = newtime % 10; // Milliseconds (0.01)
      newtime /= 10;
      stopwatchValues[1] = newtime % 10; // Milliseconds (0.1)
      newtime /= 10;
      stopwatchValues[0] = newtime % 10; // Seconds (1.0)
      SysTick_Wait10ms(30);              // Debounce
    }
  }
  if (pressed & 0x10) // Reset
  {
    GPIO_PORTF_ICR_R = 0x10; // acknowledge flag2
    // Reset the stopwatch
    stopwatchRunning = 0;
    milliseconds = 0;
    stopwatchValues[0] = 0;
    stopwatchValues[1] = 0;
    stopwatchValues[2] = 0;
    stopwatchValues[3] = 0;
    // SysTick_Wait10ms(10); // Debounce
  }
}

void main(void)
{
  // set system clock to 80 MHz
  PLL_Init(Bus80MHz);

  // initialize SysTick timer
  SysTick_Init();

  // initialize timer1A
  Timer1A_Init(80000000);

  // initialize 7-segment display
  sevenseg_init();

  // initialize timer0A
  Timer0A_Init(80000000);

  // initialize port F
  PortF_Init();

  // initialize port B
  PortB_Init();
}
