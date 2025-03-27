// Timer3A.h
// Runs on Tiva-C

#ifndef __SWITCH_H__
#define __SWITCH_H__

//Global Variables
volatile uint8_t Desired_RPM;
volatile uint8_t Mailbox_Desired_Flag;
volatile uint8_t Mailbox_ActualLCD_Flag;

void Switch_Init(void);

#endif
