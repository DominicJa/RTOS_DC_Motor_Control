// Adapted from Professor Mazidi's code at microdigitaled.com
//   cited at EduBase-V2 trainer website

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "SSI2.h"

// SPI functions for Tiva-C SSI2 module on EduBase-V2 board
// Pinout: MOSI - PB7
//         MISO - PB6 (not used)
//         SCLK - PB4
//         CS on PortC

// enable SSI2 and associated GPIO pins
// note: you must initialize your CS pin separately
void SSI2_init(void) {
  SYSCTL_RCGCSSI_R |= 0x04;    // enable clock to SSI2
  SYSCTL_RCGCGPIO_R |= 0x02;   // enable clock to GPIOB

  // PORTB 7, 4 for SSI2 TX and SCLK
  GPIO_PORTB_AMSEL_R &= ~0x90;      // turn off analog of PORTB 7, 4
  GPIO_PORTB_AFSEL_R |= 0x90;       // PORTB 7, 4 for alternate function
  GPIO_PORTB_PCTL_R &= ~0xF00F0000; // clear functions for PORTB 7, 4
  GPIO_PORTB_PCTL_R |= 0x20020000;  // PORTB 7, 4 for SSI2 function
  GPIO_PORTB_DEN_R |= 0x90;         // PORTB 7, 4 as digital pins

  SSI2_CR1_R = 0;              // make it master
  SSI2_CC_R = 0;               // use system clock
  SSI2_CPSR_R = 16;            // clock prescaler divide by 16 gets 1 MHz clock
  SSI2_CR0_R = 0x0007;         // clock rate div by 1, phase/polarity 0 0, mode freescale, data size 8
  SSI2_CR1_R = 2;              // enable SSI2

  return;
}

// enables chip select (using mask), writes one byte to SSI2,
// waits for transmit to complete, and deasserts chip select (using mask)
void SSI2_write( uint8_t data, uint8_t csMask ) {
  GPIO_PORTC_DATA_R &= ~csMask;  // assert chip select
  SSI2_DR_R = data;              // write data
  while (SSI2_SR_R & 0x10) {}    // wait for transmit done
  GPIO_PORTC_DATA_R |= csMask;   // deassert chip select
}
