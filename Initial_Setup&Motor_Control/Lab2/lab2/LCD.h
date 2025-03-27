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

#ifndef __LCD_H__
#define __LCD_H__

// Clear the LCD
// Inputs: none
// Outputs: none
void LCD_Clear();

// initialize SSI2 CS for LCD, then initialize LCD controller
// assumes Timer0A and SSI2 have already been initialized
void LCD_init(void);

// send a command to the LCD
void LCD_command( uint8_t command );

// send data (a character) to the LCD
void LCD_data( uint8_t data );

//------------LCD_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString( uint8_t *ptr );

//-----------------------LCD_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec( uint32_t n );

//-----------------------LCD_OutUHex-----------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex( uint32_t number );

// -----------------------LCD_OutUFix----------------------
// Output characters to LCD display in fixed-point format
// unsigned decimal, resolution 0.1, range 000.0 to 999.9
// Inputs:  an unsigned 32-bit number
// Outputs: none
void LCD_OutUFix( uint32_t number );

#endif
