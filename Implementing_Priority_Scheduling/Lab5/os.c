// os.c
// Runs on LM4F120/TM4C123
// A very simple real time operating system with minimal features.
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

#include <stdint.h>
#include "os.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

// function definitions in OSasm.s
void OS_DisableInterrupts(void); // Disable interrupts
void OS_EnableInterrupts(void);  // Enable interrupts
int32_t StartCritical(void);
void EndCritical(int32_t primask);
void StartOS(void);

#define NUMTHREADS  3        // maximum number of threads
#define STACKSIZE   100      // number of 32-bit words in stack
struct tcb{ // This is the thread control block
  int32_t *sp;       // pointer to stack (valid for threads not running
  uint32_t id; // thread identifier number
  struct tcb *next;  // linked-list pointer
  uint32_t *bi; // block identifier
  uint8_t prio;
};
typedef struct tcb tcbType;
tcbType tcbs[NUMTHREADS];
tcbType *RunPt; // Used to point to the running thread
int32_t Stacks[NUMTHREADS][STACKSIZE];

//uint32_t test = 7;


// ******** OS_Init ************
// initialize operating system, disable interrupts until OS_Launch
// initialize OS controlled I/O: systick, 16 MHz PLL
// Inputs: none
// Outputs: none
void OS_Init(void){
  OS_DisableInterrupts();
  PLL_Init(Bus8MHz);         // set processor clock to 8 MHz
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0x00FFFFFF)|0xE0000000; // priority 7 (Highest Prio for thread handler)
}

void SetInitialStack(int i){
  tcbs[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer
  Stacks[i][STACKSIZE-1] = 0x01000000;   // thumb bit
  Stacks[i][STACKSIZE-3] = 0x14141414;   // R14
  Stacks[i][STACKSIZE-4] = 0x12121212;   // R12
  Stacks[i][STACKSIZE-5] = 0x03030303;   // R3
  Stacks[i][STACKSIZE-6] = 0x02020202;   // R2
  Stacks[i][STACKSIZE-7] = 0x01010101;   // R1
  Stacks[i][STACKSIZE-8] = 0x00000000;   // R0
  Stacks[i][STACKSIZE-9] = 0x11111111;   // R11
  Stacks[i][STACKSIZE-10] = 0x10101010;  // R10
  Stacks[i][STACKSIZE-11] = 0x09090909;  // R9
  Stacks[i][STACKSIZE-12] = 0x08080808;  // R8
  Stacks[i][STACKSIZE-13] = 0x07070707;  // R7
  Stacks[i][STACKSIZE-14] = 0x06060606;  // R6
  Stacks[i][STACKSIZE-15] = 0x05050505;  // R5
  Stacks[i][STACKSIZE-16] = 0x04040404;  // R4
}

// ******** OS_AddThread ***************
// add three foreground threads to the scheduler
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads(void(*task0)(void),
                 void(*task1)(void),
                 void(*task2)(void)){ int32_t status;
  status = StartCritical();
  tcbs[0].next = &tcbs[1]; // 0 points to 1
  tcbs[1].next = &tcbs[2]; // 1 points to 2
  tcbs[2].next = &tcbs[0]; // 2 points to 0
  tcbs[0].id = 0; // id number for thread 0
  tcbs[1].id = 1; // id number for thread 1
  tcbs[2].id = 2; // id number for thread 2
  tcbs[0].bi = 0; // set block identifiers for threads to 0
  tcbs[1].bi = 0;
  tcbs[2].bi = 0;
  tcbs[0].prio = 1; // setting thread priority
  tcbs[1].prio = 1;
  tcbs[2].prio = 2;
  SetInitialStack(0); Stacks[0][STACKSIZE-2] = (int32_t)(task0); // PC
  SetInitialStack(1); Stacks[1][STACKSIZE-2] = (int32_t)(task1); // PC
  SetInitialStack(2); Stacks[2][STACKSIZE-2] = (int32_t)(task2); // PC
  RunPt = &tcbs[0];       // thread 0 will run first
  EndCritical(status);
  return 1;               // successful
}

/// ******** OS_Launch ***************
// start the scheduler, enable interrupts
// Inputs: number of bus clock cycles for each time slice
//         (maximum of 24 bits)
// Outputs: none (does not return)
void OS_Launch(uint32_t theTimeSlice){
  NVIC_ST_RELOAD_R = theTimeSlice - 1; // reload value
  NVIC_ST_CTRL_R = 0x00000007; // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}

// ******** OS_Wait ***************
void OS_Wait(uint32_t* sig){
    OS_DisableInterrupts();
    if((*sig) == 0){
        (*RunPt).bi = sig;
        OS_EnableInterrupts();
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PENDSTSET;
    }
    else{
        (*sig) = 0;
        OS_EnableInterrupts();
    }
}

// ******** OS_Signal ***************
void OS_Signal(uint32_t* sig){
    uint8_t i;
    uint32_t status;
    tcbType* counter = (*RunPt).next;
    status = StartCritical();

    (*sig) = 1;
/* Free all threads because its binary and also just because this function was called doesn't mean another thread got blocked */
    for(i = 0; i < 2; i++){
        if((*counter).bi == sig){
            (*counter).bi = 0;
            break;
        }
        counter = (*counter).next;
    }
    EndCritical(status);
}
