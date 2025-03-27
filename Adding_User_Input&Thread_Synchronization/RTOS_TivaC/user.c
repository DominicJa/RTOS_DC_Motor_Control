// Modified from textbook to operate using 16 MHz bus clock, changed user tasks, and using RGB LED at Port F
// TODO: Modify TIMESLICE value

//*****************************************************************************
// user.c
// Runs on LM4F120/TM4C123
// An example user program that initializes the simple operating system
//   Schedule three independent threads using preemptive round robin  
//   Each thread rapidly toggles a pin on Port D and increments its counter 
//   TIMESLICE is how long each thread runs

// Daniel Valvano
// January 29, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

   Programs 4.4 through 4.12, section 4.2

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

//#include <stdio.h>
#include <stdint.h>
#include "os.h"
#include "tm4c123gh6pm.h"
#include "SSEG.h"
#include "Timer0A.h"
#include "Timer1A.h"
#include "Timer2A.h"
#include "Timer3A.h"
#include "LCD.h"
#include "Switch.h"

#define TIMESLICE 8000000  // The thread switch time in number of SysTick counts (bus clock cycles at 8 MHz)

#define FREQUENCY 8000000.0f
#define SEC_PER_MIN 60
#define GEARBOX_RATIO 120
#define PULSE_PER_ROTATION 8

uint32_t Count1;   // number of times thread1 loops
uint32_t Count2;   // number of times thread2 loops
uint32_t Count3;   // number of times thread3 loops

uint32_t sig = 1;

int pw = 0;
uint32_t RPM = 0;

void Task1(void){
  Count1 = 0;

  // set direction
  GPIO_PORTB_DATA_R &= ~0x04;
  GPIO_PORTB_DATA_R |= 0x08;

  //OS_Wait(&sig);

  for(;;){
    Count1++;
    //GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | (0x01<<1);  // Show red (Save this line for task 3)
    //Below is my attempt at the PWM DC Motor Code

    // set direction
    //GPIO_PORTB_DATA_R &= ~0x04;
    //GPIO_PORTB_DATA_R |= 0x08;


    // reverse direction
   // GPIO_PORTB_DATA_R &= ~0x08;
    //GPIO_PORTB_DATA_R |= 0x04;


    if(Mailbox_Desired_Flag == 1){ // Check for new desired speed
        OS_Wait(&sig);
        pw = (28 * Desired_RPM) + 100; // equation for setting the pw based on desired speed
        PWM1_3_CMPB_R = pw;
        OS_Signal(&sig);
        Timer3A_Wait1ms(250); // Do I even need this?
        RPM = (SEC_PER_MIN*FREQUENCY)/(PULSE_PER_ROTATION*GEARBOX_RATIO*CC_Difference)*10;
        Mailbox_Desired_Flag = 0;
        Mailbox_Actual_Flag = 1;
    }
  }
}

void Task2(void){
  Count2 = 0;
  //OS_Signal(&sig);
  for(;;){
    Count2++;
   //GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | (0x02<<1);  // Show blue (Save this line for task 3)
    //RPM = (SEC_PER_MIN*FREQUENCY)/(PULSE_PER_ROTATION*GEARBOX_RATIO*CC_Difference)*10;
    if((Mailbox_Actual_Flag == 1) || (Mailbox_ActualLCD_Flag == 1)){
        Timer0A_Wait1ms(40);
        LCD_Clear();
        LCD_OutUDec(Desired_RPM); // Put on first line
        LCD_command(0xC0); // Cursor moved to row 2 col 1
        LCD_OutUFix(RPM); // Put on second line
        Mailbox_Actual_Flag = 0;
        Mailbox_ActualLCD_Flag = 0;
    }
  }
}

void Task3(void){
  Count3 = 0;
  uint32_t Actual_RPM = 0;
  float Expected_RPM = 0.0f;
  int8_t Diff = 0;
  int32_t Percent_Error = 0;
  uint8_t temp_desired_RPM = 0;
  for(;;){
    Count3++;
    //GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | (0x03<<1);  // Show red + blue = purple/magenta
    Actual_RPM = (SEC_PER_MIN*FREQUENCY)/(PULSE_PER_ROTATION*GEARBOX_RATIO*CC_Difference);
    Expected_RPM = (pw-100)/28;
    Diff = Expected_RPM - Actual_RPM;
    Percent_Error = (Diff/Expected_RPM)*100;

    if ((Diff > 0) && (Expected_RPM > 0)){
        if((Percent_Error > 10) && (Percent_Error < 35)){
            OS_Wait(&sig);
            Mailbox_Desired_Flag = 1;
            pw = 0;
            PWM1_3_CMPB_R = pw;
            temp_desired_RPM = Desired_RPM;
            while(Desired_RPM <= temp_desired_RPM){}
            OS_Signal(&sig);

        }
    }
  }
}

int main(void){
  OS_Init();           // initialize, disable interrupts, set PLL to 8 MHz (equivalent to PLL_Init()) checked(this means should be ready)
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20) == 0){}; // allow time for clock to stabilize
  GPIO_PORTF_DIR_R |= 0x0E;             // make PF3-1 out
  GPIO_PORTF_AFSEL_R &= ~0x0E;          // disable alt funct on PF3-1
  GPIO_PORTF_DEN_R |= 0x0E;             // enable digital I/O on PF3-1
  GPIO_PORTF_PCTL_R &= ~0x0000FFF0;     // configure PF3-1 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x0E;          // disable analog functionality on PF3-1
  OS_AddThreads(&Task1, &Task2, &Task3);

  Timer0A_Init(8000000);
  Timer1A_Init(8000000);
  Timer2A_Init();
  Timer3A_Init(8000000);
  sevenseg_init(); // This is equivalent to SSI2_init()
  LCD_init();

  //Code for PWM init is below

   SYSCTL_RCGCPWM_R |= 0x02;        // enable clock to PWM1
   SYSCTL_RCGCGPIO_R |= 0x20;       // enable clock to GPIOF
   SYSCTL_RCGCGPIO_R |= 0x02;       // enable clock to GPIOB

   //delayMs(1);                     // PWM1 seems to take a while to start
   Timer0A_Wait1ms(1);

   SYSCTL_RCC_R &= ~0x00100000;     // use system clock for PWM
   PWM1_INVERT_R  |= 0x80;           // positive pulse
   PWM1_3_CTL_R = 0;               // disable PWM1_3 during configuration
   PWM1_3_GENB_R = 0x0000080C;     // output high when load and low when match
   PWM1_3_LOAD_R = 3999;           // 4 kHz
   PWM1_3_CTL_R = 1;               // enable PWM1_3
   PWM1_ENABLE_R |= 0x80;           // enable PWM1

   GPIO_PORTF_DIR_R |= 0x08;             // set PORTF 3 pins as output (LED) pin
   GPIO_PORTF_DEN_R |= 0x08;             // set PORTF 3 pins as digital pins
   GPIO_PORTF_AFSEL_R |= 0x08;           // enable alternate function
   GPIO_PORTF_PCTL_R &= ~0x0000F000;     // clear PORTF 3 alternate function
   GPIO_PORTF_PCTL_R |= 0x00005000;      // set PORTF 3 alternate funtion to PWM

   GPIO_PORTB_DEN_R |= 0x0C;             // PORTB 3 as digital pins
   GPIO_PORTB_DIR_R |= 0x0C;             // set PORTB 3 as output
   GPIO_PORTB_DATA_R |= 0x08;            // enable PORTB 3

  //Code for PWM init ends on the line above

   Switch_Init();


  OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
  return 0;             // this never executes
}
