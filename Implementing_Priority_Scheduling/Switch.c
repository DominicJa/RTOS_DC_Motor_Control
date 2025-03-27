// Switch.c
// Runs on Tiva-C

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Switch.h"
#include "Timer0A.h"

void Switch_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x08; // activate clock for Port D
    //GPIO_PORTD_LOCK_R = 0x4C4F434B; // unlock GPIO Port D
    GPIO_PORTD_AMSEL_R &= ~0x0D; // disable analong function on PD3-0 (NEW)
    GPIO_PORTD_PCTL_R &= ~0x0000FF0F; // configure PD3-0 as GPIO (NEW)
    //GPIO_PORTD_CR_R = 0x0D; // allow changes to PD3-0
    GPIO_PORTD_DIR_R &= ~0x0D; // make PD3-0 in
    GPIO_PORTD_AFSEL_R &= ~0x0D; // disable alt funct on PD3-0
    GPIO_PORTD_DEN_R |= 0x0D; // enable digistal I/O on PD3-0
    //GPIO_PORTD_PUR_R |= 0x0F; // pullup on PD3-0
    GPIO_PORTD_IS_R &= ~0x0D; // PD3-0 are edge-sensitive
    GPIO_PORTD_IBE_R &= ~0x0D; // PD3-0 are single edge
    GPIO_PORTD_IEV_R |= 0x0D; // PD3-0 rising edge triggered
    GPIO_PORTD_ICR_R = 0x0D; // clear flags
    GPIO_PORTD_IM_R |= 0x0D; // arm interrupts on PD3-0
    NVIC_PRI0_R = (NVIC_PRI0_R&0x00FFFFFF) | 0x20000000; // priority 1
    NVIC_EN0_R = 0x08; // enable interrupt 3 in NVIC
}

void GPIOPortD_Handler(void){
    Timer0A_Wait1ms(25); // Wait for switch to stabilize (aka wait for debouncing)

    if(GPIO_PORTD_RIS_R&0x08){ // poll PD3 (aka SW2 check)
        GPIO_PORTD_ICR_R = 0x08; // acknowledge flag3

        //Speed up by 10 RPM (Use global variable for desired speed and update it here)
        if((Desired_RPM <= 140) && (Desired_RPM >= 0)){
            Desired_RPM += 10;
        }
        //Add a mailbox here to set the flag, which will tell the task a new speed has been requested
        Mailbox_Desired_Flag = 1;
        Mailbox_ActualLCD_Flag = 1;
    }
    if(GPIO_PORTD_RIS_R&0x04){ // poll PD2 (aka SW3 check)
        GPIO_PORTD_ICR_R = 0x04; // acknowledge flag2

        //Slow down by 10 RPM (Use global variable for desired speed and update it here)
        if(Desired_RPM >= 10){
           Desired_RPM -= 10;
        }
        //Add a mailbox here to set the flag, which will tell the task a new speed has been requested
        Mailbox_Desired_Flag = 1;
        Mailbox_ActualLCD_Flag = 1;
    }
    if(GPIO_PORTD_RIS_R&0x01){ // poll PD0 (aka SW5 check)
        GPIO_PORTD_ICR_R = 0x01; // acknowledge flag0

        //Set speed to 0 RPM (Use global variable for desired speed and update it here)
        Desired_RPM = 0;
        //Add a mailbox here to set the flag, which will tell the task a new speed has been requested
        Mailbox_Desired_Flag = 1;
        Mailbox_ActualLCD_Flag = 1;
    }
}

