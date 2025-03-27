// Timer2A.h
// Runs on Tiva-C

// Adapted from Program 8.2 from the book:
/* "Embedded Systems: Introduction to ARM Cortex-M Microcontrollers",
   ISBN: 978-1477508992, Jonathan Valvano, copyright (c) 2013
   Volume 1, Program 9.8
*/

#ifndef __TIMER2A_H__
#define __TIMER2A_H__

//Global Variables
volatile uint32_t Last_CC_Count;
volatile uint32_t Current_CC_Count;
volatile uint32_t CC_Difference;
volatile uint8_t Mailbox_Actual_Flag;
uint32_t mutex;

void EnableInterrupts();
void Timer2A_Init(); // Using PB0 for input capture (T2CCP0)

#endif
