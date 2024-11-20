/*
  size is 1*16
  if do not need to read busy, then you can tie R/W=ground
  ground = pin 1    Vss
  power  = pin 2    Vdd   +3.3V or +5V (VBUS) depending on the device
  ground = pin 3    Vlc   grounded for highest contrast
  PE0   = pin 4    RS    (1 for data, 0 for control/status)
  ground = pin 5    R/W   (1 for read, 0 for write)
  PC6   = pin 6    E     (enable)
  PA2   = pin 11   DB4   (4-bit data)
  PA3   = pin 12   DB5
  PA4   = pin 13   DB6
  PA5   = pin 14   DB7
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/

#include <stdint.h>
#include "LCD.h"
#include "TimerA0.h"
#include "tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical(void);     // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// Macros
#define BusFreq 80             // assuming a 80 MHz system clock
#define T6us 6 * BusFreq       // 6us
#define T40us 40 * BusFreq     // 40us
#define T160us 160 * BusFreq   // 160us
#define T1ms 1000 * BusFreq    // 1ms
#define T1600us 1600 * BusFreq // 1.60ms
#define T5ms 5000 * BusFreq    // 5ms
#define T15ms 15000 * BusFreq  // 15ms

// Global Vars
uint8_t LCD_RS, LCD_E; // LCD Enable and Register Select

/**************** Private Functions ****************/

void Out_RS_E()
{
    GPIO_PORTE_DATA_R = (GPIO_PORTE_DATA_R & ~0x01) | LCD_RS;
    GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~0x40) | (LCD_E << 6);
}

void SendPulse()
{
    Out_RS_E();
    TimerA0_Wait(T6us); // wait 6us
    LCD_E = 1;          // E=1, R/W=0, RS=1
    Out_RS_E();
    TimerA0_Wait(T6us); // wait 6us
    LCD_E = 0;          // E=0, R/W=0, RS=1
    Out_RS_E();
}

void SendChar()
{
    LCD_E = 0;
    LCD_RS = 1; // E=0, R/W=0, RS=1
    SendPulse();
    TimerA0_Wait(T1600us); // wait 1.6ms
}

void SendCmd()
{
    LCD_E = 0;
    LCD_RS = 0; // E=0, R/W=0, RS=0
    SendPulse();
    TimerA0_Wait(T40us); // wait 40us
}

/**************** Public Functions ****************/
// Clear the LCD
// Inputs: none
// Outputs: none
void LCD_Clear()
{
    LCD_OutCmd(0x01); // Clear Display
    LCD_OutCmd(0x80); // Move cursor back to 1st position
}

// Initialize LCD
// Inputs: none
// Outputs: none
void LCD_Init()
{
    SYSCTL_RCGC2_R |= 0x00000015; // 1) activate clock for Ports A, C, and E
    while ((SYSCTL_PRGPIO_R & 0x015) != 0x015)
    {
    }; // ready?
    GPIO_PORTA_AMSEL_R &= ~0x3C;      // 3) disable analog function on PA5-2
    GPIO_PORTC_AMSEL_R &= ~0x40;      //    disable analog function on PC6
    GPIO_PORTE_AMSEL_R &= ~0x01;      //    disable analog function on PE0
    GPIO_PORTA_PCTL_R &= ~0x00FFFF00; // 4) configure PA5-2 as GPIO
    GPIO_PORTC_PCTL_R &= ~0x0F000000; //    configure PC6 as GPIO
    GPIO_PORTE_PCTL_R &= ~0x0000000F; //    configure PE0 as GPIO
    GPIO_PORTA_DIR_R |= 0x3C;         // 5) set direction register
    GPIO_PORTC_DIR_R |= 0x40;
    GPIO_PORTE_DIR_R |= 0x01;
    GPIO_PORTA_AFSEL_R &= ~0x3C; // 6) regular port function
    GPIO_PORTC_AFSEL_R &= ~0x40;
    GPIO_PORTE_AFSEL_R &= ~0x01;
    GPIO_PORTA_DEN_R |= 0x3C; // 7) enable digital port
    GPIO_PORTC_DEN_R |= 0x40;
    GPIO_PORTE_DEN_R |= 0x01;
    GPIO_PORTA_DR8R_R |= 0x3C; // enable 8 mA drive
    GPIO_PORTC_DR8R_R |= 0x40;
    GPIO_PORTE_DR8R_R |= 0x01;

    LCD_E = 0;
    LCD_RS = 1;
    Out_RS_E();
    TimerA0_Wait(T15ms);  // Wait >15 ms after power is applied
    LCD_OutCmd(0x30);     // command 0x30 = Wake up
    TimerA0_Wait(T5ms);   // must wait 5ms, busy flag not available
    LCD_OutCmd(0x30);     // command 0x30 = Wake up #2
    TimerA0_Wait(T160us); // must wait 160us, busy flag not available
    LCD_OutCmd(0x30);     // command 0x30 = Wake up #3
    TimerA0_Wait(T160us); // must wait 160us, busy flag not available
    LCD_OutCmd(0x28);     // Function set: 4-bit/2-line
    LCD_Clear();
    LCD_OutCmd(0x10); // Set cursor
    // LCD_OutCmd(0x0C);         // Display ON; Cursor ON
    LCD_OutCmd(0x06); // Entry mode set
}

// Output a character to the LCD
// Inputs: letter is ASCII character, 0 to 0x7F
// Outputs: none
void LCD_OutChar(char letter)
{
    unsigned char let_low = (0x0F & letter) << 2;
    unsigned char let_high = (letter >> 2) & 0x3C;
    long intstatus = StartCritical();

    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | let_high;
    SendChar();
    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | let_low;
    SendChar();
    EndCritical(intstatus);
    TimerA0_Wait(T1ms); // wait 1ms
}

// Output a command to the LCD
// Inputs: 8-bit command
// Outputs: none
void LCD_OutCmd(unsigned char command)
{
    unsigned char com_low = (0x0F & command) << 2;
    unsigned char com_high = (command >> 2) & 0x3C;
    long intstatus = StartCritical();

    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | com_high;
    SendCmd();
    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | com_low;
    SendCmd();
    EndCritical(intstatus);
    TimerA0_Wait(T1ms); // wait 1ms
}

//------------LCD_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString(char *ptr)
{
    while (*ptr)
    {
        LCD_OutChar(*ptr);
        ptr++;
    }
}

//-----------------------LCD_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec(uint32_t n)
{
    // This function uses recursion to convert decimal number
    //   of unspecified length as an ASCII string
    if (n >= 10)
    {
        LCD_OutUDec(n / 10);
        n = n % 10;
    }

    LCD_OutChar(n + '0');
}

//-----------------------LCD_OutUHex-----------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex(uint32_t number)
{
    // This function uses recursion to convert the number of
    //   unspecified length as an ASCII string
}

// -----------------------LCD_OutUFix----------------------
// Output characters to LCD display in fixed-point format
// unsigned decimal, resolution 0.001, range 0.000 to 9.999
// Inputs:  an unsigned 32-bit number
// Outputs: none
// E.g., 0,    then output "0.000 "
//       3,    then output "0.003 "
//       89,   then output "0.089 "
//       123,  then output "0.123 "
//       9999, then output "9.999 "
//       9999, then output "*.*** "
void LCD_OutUFix(uint32_t number)
{
    // This function uses recursion to convert the number of
    //   unspecified length as an ASCII string
    if (number > 9999)
    {

        // values greater than 9999
        LCD_OutString("*.***");
    }
    else
    {
        // set a temporary array to store the number passed in
        int temp[5];

        uint16_t i;
        
        // store every digit of the number in the array
        for (i = 0; i < 4; i++)
        {
            temp[i] = number % 10;
            number /= 10;
        }

        // output the 1000s place value
        LCD_OutUDec(temp[3]);

        // output a decimal point
        LCD_OutChar('.');

        // output the 100s place value
        LCD_OutUDec(temp[2]);

        // output the 10s place value
        LCD_OutUDec(temp[1]);

        // output the 1s place value
        LCD_OutUDec(temp[0]);
    }
}
