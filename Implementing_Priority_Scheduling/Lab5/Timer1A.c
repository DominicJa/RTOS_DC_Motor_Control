// Timer1A.c
// Runs on Tiva-C

// Adapted from SysTick.c from the book:
/* "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
*/

#include <stdint.h>
#include <Timer1A.h>
#include "tm4c123gh6pm.h"
#include "SSEG.h"
#include "os.h"

static uint32_t sysClkFreq = 8000000; // Assume 8 MHz clock by default

// Set clock freq. so Timer1A_Wait10ms delays for exactly 10 ms if clock is not 80 MHz
void Timer1A_Init( uint32_t clkFreq ){
  sysClkFreq = clkFreq;

  //Sets up Timer1A for periodic interrupts

  SYSCTL_RCGCTIMER_R |= 0x00000002; // 0) Activate Timer1
  TIMER1_CTL_R &= ~0x00000001;      // 1) Disable Timer1A during setup
  TIMER1_CFG_R = 0x0;               // 2) Configure for 32-bit timer mode
  TIMER1_TAMR_R = 0x2;              // 3) Configure for Periodic mode
  TIMER1_TAILR_R = 0x3E80;          // 5) Specify reload value (Using 8000 because I want a count down of 1ms)
  TIMER1_TAPR_R = 0;                // N/A) No prescalez
  TIMER1_IMR_R |= TIMER_IMR_TATOIM; // 6) Enable Time-Out interrupt
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// N/A) Clear Timer1A Time_Out RAW Interrupt
  TIMER1_CTL_R |= TIMER_CTL_TAEN;   // 7) Enable Timer1A
  NVIC_PRI5_R = (NVIC_PRI5_R & 0xFFFF0FFF) | 0xFFFF2FFF; //Timer2A = Priority of 1
  NVIC_EN0_R = 0x00200000;          // Enable interrupt 21 in NVIC

  return;
}


//Set up the Interrupt handler/systick handler for periodic interrupts
void Timer1A_Handler(){
    TIMER1_ICR_R = TIMER_ICR_TATOCINT; // Acknowledge Timer1A Timeout
    SSEG1(Global_Thread_Id);
    SSEG2(Time_Slice_Counter);
    return;
}
