/* Adapted from Professor Mazidi's code at microdigitaled.com
 *   cited at EduBase-V2 trainer website
 *
 * Serial LCD Driver Functions
 *
 * The LCD controller of Wytec EduPad board is connected to
 * a shift register on SSI2 of Tiva LaunchPad.  PORTC 6 is used
 * for chip select of the shift register.
 *
 * The connections between the shift register and LCD are
 * Bit 0 - RS (register select)
 * Bit 1 - E (enable)
 * Bit 2 -
 * Bit 4 - Data 4
 * Bit 5 - Data 5
 * Bit 6 - Data 6
 * Bit 7 - Data 7
 *
 * Built and tested with Keil MDK-ARM v5.24a and TM4C_DFP v1.1.0
 */

#include <stdint.h>
#include "Timer0A.h"
#include "SSI2.h"
#include "LCD.h"
#include "tm4c123gh6pm.h"
#include <stdio.h>

void DisableInterrupts(void);    // Disable interrupts
void EnableInterrupts(void);     // Enable interrupts
uint32_t StartCritical (void);   // previous I bit, disable interrupts
void EndCritical( uint32_t sr ); // restore I bit to previous value
void WaitForInterrupt(void);     // low power mode

// Macros
#define RS 1    // BIT0 mask for reg select
#define EN 2    // BIT1 mask for E

/**************** Private Functions ****************/

// LCD's SPI chip select is at PC6 (mask of 0x40 for SSI2_Write)
void LCD_nibble_write( uint8_t data, uint8_t control) {
  data &= 0xF0;       // clear lower nibble for control
  control &= 0x0F;    // clear upper nibble for data
  SSI2_write( (data | control), 0x40 );       // RS = 0, R/W = 0
  SSI2_write( (data | control | EN), 0x40 );  // pulse E
  //delayMs(0);
  SSI2_write( data, 0x40 );
  return;
}

/**************** Public Functions ****************/

// Clear the LCD
// Inputs: none
// Outputs: none
void LCD_Clear(void) {
  LCD_command(0x01);  // Clear Display
  // not necessary //LCD_command(0x80);  // Move cursor back to 1st position
}

// initialize SSI2 CS for LCD, then initialize LCD controller
// assumes Timer0A and SSI2 have already been initialized
void LCD_init(void) {
  SYSCTL_RCGCGPIO_R |= 0x04;   // enable clock to GPIOC

  // PORTC 6 for SSI2 chip select
  GPIO_PORTC_AMSEL_R &= ~0x40;      // disable analog
  GPIO_PORTC_DATA_R |= 0x40;        // set PORTC6 idle high
  GPIO_PORTC_DIR_R |= 0x40;         // set PORTC6 as output for CS
  GPIO_PORTC_DEN_R |= 0x40;         // set PORTC6 as digital pins

  Timer0A_Wait1ms(20);        // LCD controller reset sequence
  LCD_nibble_write(0x30, 0);
  Timer0A_Wait1ms(5);
  LCD_nibble_write(0x30, 0);
  Timer0A_Wait1ms(1);
  LCD_nibble_write(0x30, 0);
  Timer0A_Wait1ms(1);

  LCD_nibble_write(0x20, 0);  // use 4-bit data mode
  Timer0A_Wait1ms(1);
  LCD_command(0x28);          // set 4-bit data, 2-line, 5x7 font
  LCD_command(0x06);          // move cursor right
  LCD_command(0x01);          // clear screen, move cursor to home
  LCD_command(0x0F);          // turn on display, cursor blinking

  return;
}

// send a command to the LCD
void LCD_command( uint8_t command ) {
  uint32_t intStatus = StartCritical();
  LCD_nibble_write(command & 0xF0, 0);    // upper nibble first
  LCD_nibble_write(command << 4, 0);      // then lower nibble
  EndCritical( intStatus );

  if (command < 4)
    Timer0A_Wait1ms(2);         // command 1 and 2 needs up to 1.64ms
  else
    Timer0A_Wait1ms(1);         // all others 40 us

  return;
}

// send data (a character) to the LCD
void LCD_data( uint8_t data ) {
  uint32_t intStatus = StartCritical();
  LCD_nibble_write(data & 0xF0, RS);      // upper nibble first
  LCD_nibble_write(data << 4, RS);        // then lower nibble
  EndCritical( intStatus );

  Timer0A_Wait1ms(1);

  return;
}

//------------LCD_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString( uint8_t *ptr ) {
  return;
}

//-----------------------LCD_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec( uint32_t n ) {
  // This function uses recursion to convert decimal number
  //   of unspecified length as an ASCII string
  if(n < 10){
      uint8_t decimal_to_char = (n % 10) + '0';
      LCD_data(decimal_to_char);
      return;
  }
  LCD_OutUDec(n / 10);
  LCD_OutUDec(n % 10);
}

//-----------------------LCD_OutUHex-----------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex( uint32_t number ) {
// This function uses recursion to convert the number of
// unspecified length as an ASCII string
  return;
}

// -----------------------LCD_OutUFix----------------------
// Output characters to LCD display in fixed-point format
// unsigned decimal, resolution 0.1, range 000.0 to 999.9
// Inputs:  an unsigned 32-bit number
// Outputs: none
// E.g., 0,    then output "0.0"
//       3,    then output "0.3"
//       89,   then output "8.9"
//       123,  then output "12.3"
//       9999, then output "999.9"
//     > 9999, then output "*.***"
void LCD_OutUFix( uint32_t number ) {
  if(number < 10){// maybe condition should be if number < 10
      //send decimal point to LCD_data
      LCD_data('.');
      //send last number to LCD_data using mod 10 I think would work
      uint8_t decimal_to_char = (number % 10) + '0';
      LCD_data(decimal_to_char);
      return;
  }
  else if(number > 9999){//This should never happen but just incase
      //Just send -.--- or *.*** to the LCD. I think this is what he said to do
      //To implement this I should be able to  just hard code this and return
      LCD_data('*');
      LCD_data('.');
      LCD_data('*');
      LCD_data('*');
      LCD_data('*');
      return;
  }
  else{
  LCD_OutUDec(number / 10);
  LCD_OutUFix(number % 10);
  }
}
