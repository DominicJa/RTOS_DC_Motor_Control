/* Seven-segment display counter
 *
 * This program counts number 0-3 on the seven segment display.
 * The seven segment display is driven by a shift register which is
 * connected to SSI2 in SPI mode.
 *
 * Built and tested with Keil MDK-ARM v5.28 and TM4C_DFP v1.1.0
 */

#include <stdint.h>
#include <math.h>
#include "tm4c123gh6pm.h"
#include "SSEG.h"
#include "Timer1A.h"

//void sevenseg_init(void);
//void SSI2_write(unsigned char data);

void SSEG1(uint32_t Thread_Id) {

    //(Set this function in user.c main somewhere I think) sevenseg_init();    // initialize SSI2 that connects to the shift registers
    //The seven segment digits start from the right and go left. So writing a 1 would be right most and writing a 8 would be left most. Look at binary representation
       //SSI2_write(digitPattern[digitPattern_count]);    // write digit pattern to the seven segments (First HCT595, which drives the cathodes)
       //SSI2_write((1 << digitPattern_count));           // select digit (Second HCT595, which drives the common anodes)
       //if (++digitPattern_count > 3)
           //digitPattern_count = 0;
    switch(Thread_Id){
        case 0: SSI2_write(digitPattern[0], 0x80);
                SSI2_write(0x08, 0x80);
                break;
        case 1: SSI2_write(digitPattern[1], 0x80);
                SSI2_write(0x08, 0x80);
                break;
        case 2: SSI2_write(digitPattern[2], 0x80);
                SSI2_write(0x08, 0x80);
                break;
        default: SSI2_write(0x8E, 0x80);
                 SSI2_write(0x08, 0x80);
    }
}

void SSEG2(uint32_t Time_Slice){
    uint8_t i;
    uint8_t digits[4];
    uint8_t n_digits = 0;
    uint32_t temp = Time_Slice;

    while(temp != 0){
        temp /= 10;
        n_digits++;
    }

    for(i = 0; i < n_digits; ++i){
        digits[i] = Time_Slice % 10;
        Time_Slice /= 10;
    }

    for(i = 0; i < n_digits; ++i){
        switch(digits[i]){
        case 0: SSI2_write(digitPattern[0], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 1: SSI2_write(digitPattern[1], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 2: SSI2_write(digitPattern[2], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 3: SSI2_write(digitPattern[3], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 4: SSI2_write(digitPattern[4], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 5: SSI2_write(digitPattern[5], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 6: SSI2_write(digitPattern[6], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 7: SSI2_write(digitPattern[7], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 8: SSI2_write(digitPattern[8], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        case 9: SSI2_write(digitPattern[9], 0x80);
                SSI2_write(1 << i, 0x80);
                break;
        }
    }
}

// enable SSI2 and associated GPIO pins
void sevenseg_init(void) {
    SYSCTL_RCGCGPIO_R |= 0x02;   // enable clock to GPIOB
    SYSCTL_RCGCGPIO_R |= 0x04;   // enable clock to GPIOC
    SYSCTL_RCGCSSI_R |= 0x04;    // enable clock to SSI2

    // PORTB 7, 4 for SSI2 TX and SCLK
    GPIO_PORTB_AMSEL_R &= ~0x90;      // turn off analog of PORTB 7, 4
    GPIO_PORTB_AFSEL_R |= 0x90;       // PORTB 7, 4 for alternate function
    GPIO_PORTB_PCTL_R &= ~0xF00F0000; // clear functions for PORTB 7, 4
    GPIO_PORTB_PCTL_R |= 0x20020000;  // PORTB 7, 4 for SSI2 function
    GPIO_PORTB_DEN_R |= 0x90;         // PORTB 7, 4 as digital pins

    // PORTC 7 for SSI2 slave select
    GPIO_PORTC_AMSEL_R &= ~0x80;      // disable analog of PORTC 7
    GPIO_PORTC_DATA_R |= 0x80;        // set PORTC 7 idle high
    GPIO_PORTC_DIR_R |= 0x80;         // set PORTC 7 as output for SS
    GPIO_PORTC_DEN_R |= 0x80;         // set PORTC 7 as digital pin

    SSI2_CR1_R = 0;              // turn off SSI2 during configuration
    SSI2_CC_R = 0;               // use system clock
    SSI2_CPSR_R = 16;            // clock prescaler divide by 16 gets 1 MHz clock
    SSI2_CR0_R = 0x0007;         // clock rate div by 1, phase/polarity 0 0, mode freescale, data size 8
    SSI2_CR1_R = 2;              // enable SSI2 as master
}

// This function enables slave select, writes one byte to SSI2,
// wait for transmit complete and deassert slave select.
void SSI2_write(uint8_t data, uint8_t csMask) {
    GPIO_PORTC_DATA_R &= ~csMask;       // assert slave select
    SSI2_DR_R = data;            // write data
    while (SSI2_SR_R & 0x10) {}  // wait for transmit done
    GPIO_PORTC_DATA_R |= csMask;        // deassert slave select
}
