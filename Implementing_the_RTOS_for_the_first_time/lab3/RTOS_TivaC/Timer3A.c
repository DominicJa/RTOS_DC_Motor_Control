// Timer3A.c
// Runs on Tiva-C

// Adapted from SysTick.c from the book:
/* "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
*/

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Timer3A.h"

static uint32_t sysClkFreq = 8000000; // Assume 8 MHz clock by default

// Set clock freq. so Timer0A_Wait10ms delays for exactly 10 ms if clock is not 8 MHz
void Timer3A_Init( uint32_t clkFreq ){
  sysClkFreq = clkFreq;
  return;
}

// Time delay using busy wait
// The delay parameter is in units of the core clock (units of 12.5 nsec for 80 MHz clock)
//   Adapted from Program 9.8 from the book:
/*   "Embedded Systems: Introduction to ARM Cortex-M Microcontrollers",
     ISBN: 978-1477508992, Jonathan Valvano, copyright (c) 2013
     Volume 1, Program 9.8
*/
void Timer3A_Wait( uint32_t delay ){

  if(delay <= 1){ return; } // Immediately return if requested delay less than one clock

  SYSCTL_RCGCTIMER_R |= 0x00000008;  // 0) Activate Timer3
  TIMER3_CTL_R &= ~0x00000001;       // 1) Disable Timer3A during setup
  TIMER3_CFG_R = 0;                  // 2) Configure for 32-bit timer mode
  TIMER3_TAMR_R = 1;                 // 3) Configure for one-shot mode
  TIMER3_TAILR_R = delay - 1;        // 4) Specify reload value
  TIMER3_TAPR_R = 0;                 // 5) No prescale
  TIMER3_IMR_R = 0;                  // 6-9) No interrupts
  TIMER3_CTL_R |= 0x00000001;        // 10) Enable Timer3A

  //while( TIMER0_TAR_R ){} // Doesn't work; Wait until timer expires (value equals 0)
  // Or, clear interrupt and wait for raw interrupt flag to be set
  TIMER3_ICR_R = 1;
  while( !(TIMER3_RIS_R & 0x1) ){}
  return;
}

// Time delay using busy wait
// This assumes 8 MHz system clock
void Timer3A_Wait1ms( uint32_t delay ){
  uint32_t i;
  for( i = 0; i < delay; i++ ){
    Timer3A_Wait(sysClkFreq/1000);  // wait 1ms
  }
  return;
}
