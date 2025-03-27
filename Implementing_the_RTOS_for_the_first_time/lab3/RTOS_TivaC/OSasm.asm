;/*****************************************************************************/
; OSasm.asm: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123
; A very simple real time operating system with minimal features.
; Daniel Valvano
; January 29, 2015
;
; This example accompanies the book
;  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
;  ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;
;  Programs 4.4 through 4.12, section 4.2
;
;Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
;    You may use, edit, run or distribute this file
;    as long as the above copyright notice remains
; THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
; OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; For more information about my classes, my research, and my books, see
; http://users.ece.utexas.edu/~valvano/
; */

        .thumb
        .text
        .align 2


        .global  RunPt            ; currently running thread
        .global  OS_DisableInterrupts
        .global  OS_EnableInterrupts
        .global  StartOS
        .global  SysTick_Handler
        .global Time_Slice_Counter ; global time slice varible here
        .global Global_Thread_Id ; global thread identifier number


OS_DisableInterrupts:  .asmfunc
        CPSID   I
        BX      LR
       .endasmfunc

OS_EnableInterrupts:  .asmfunc
        CPSIE   I
        BX      LR
       .endasmfunc

SysTick_Handler:  .asmfunc     ; 1) Saves R0-R3,R12,LR,PC,PSR

    CPSID   I                  ; 2) Prevent interrupt during switch

    LDR		R0, Time_Slice_CounterAddr ; May need to put this whole part after CPSID I (because increamenting counter than being interrupted sounds wrong)
	LDR R1, [R0]
	ADD R1, #1
	STR R1, [R0]

    PUSH    {R4-R11}           ; 3) Save remaining regs r4-11
    LDR     R0, RunPtAddr      ; 4) R0=pointer to RunPt, old thread
    LDR     R1, [R0]           ;    R1 = RunPt
    STR     SP, [R1]           ; 5) Save SP into TCB
    LDR     R1, [R1,#8]        ; 6) R1 = RunPt->next
    STR     R1, [R0]           ;    RunPt = R1

    LDR		R2, [R0]	       ; R2 = new RunPt
    LDR 	R4, [R2,#4]        ; R4 = value of new RunPt.id
    LDR     R3, Global_Thread_IdAddr ; R3 points to Global_Thread_IdAddr
    STR		R4, [R3]		   ; R3 (Global_Thread_Id) = R2 (Value of new RunPt.id)

    LDR     SP, [R1]           ; 7) new thread SP; SP = RunPt->sp;
    POP     {R4-R11}           ; 8) restore regs r4-11
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR
   .endasmfunc
RunPtAddr .field RunPt,32
Time_Slice_CounterAddr .field Time_Slice_Counter,32
Global_Thread_IdAddr .field Global_Thread_Id,32

StartOS:  .asmfunc
	LDR 	R0, Time_Slice_CounterAddr
	LDR		R1, [R0] 		   ; Load in value of Time_Slice_Counter to R1
	MOV 	R1, #0			   ; May not be needed because Time_Slice_Counter is 0 by default
	ADD		R1, #1
	STR		R1, [R0]

    LDR     R0, RunPtAddr      ; currently running thread
    LDR     R2, [R0]           ; R2 = value of RunPt

    LDR 	R1, [R2,#4]        ; R1 = value at RunPt.id
    LDR 	R3, Global_Thread_IdAddr ; R3 points to Global_Thread_IdAddr
    STR     R1, [R3]		   ; R3 (Global_Thread_Id) = R1 (Value of RunPt.id)

    LDR     SP, [R2]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11
    POP     {R0-R3}            ; restore regs r0-3
    POP     {R12}
    POP     {LR}               ; discard LR from initial stack
    POP     {LR}               ; start location
    POP     {R1}               ; discard PSR
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread
   .endasmfunc
   .end
