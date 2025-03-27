// Timer1A.h
// Runs on Tiva-C

// Adapted from SysTick.h from the book:
/* "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
*/

#ifndef __TIMER1A_H__
#define __TIMER1A_H__

// Set clock freq. so Timer1A_Wait10ms delays for exactly 10 ms if clock is not 80 MHz
void Timer1A_Init( uint32_t clkFreq );

#endif
