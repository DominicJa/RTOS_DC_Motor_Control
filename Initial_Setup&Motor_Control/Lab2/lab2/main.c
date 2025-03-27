/* This program controls the rotational speed and direction of
 * a DC motor.
 *
 * The Wytec EduPad board has a quad half-H-bridge motor driver,
 * which is capable of driving two DC motors or a stepper motor
 * with up to 15V and 1.2A continuous current.
 *
 * The enable pins of the half-bridges are connected to PORTB 3-0.
 * The PWM inputs are connected to PORTF 2 and PORTF 3.  These
 * pins are shared with the LEDs.  The PORTB pins are connected to
 * the LEDs on the EduBase board.  The PORTF pins are connected to
 * the LEDs on the Tiva LaunchPad.
 *
 * The four output pins of the motor driver device are connected to
 * the blue terminals marked M1, M2, M3, and M4.  Use of external
 * power supply for motor is recommended.  External power should be
 * connected to the terminal T3 and the jumper J4 should be at EXT.
 *
 * This program uses PORTB 3, 2 for direction control and PORTF 3 for
 * PWM control.  The outputs are terminals M3 and M4.
 * The PWM is generated by PWM1_3B.
 *
 * Built and tested with Keil MDK-ARM v5.24a and TM4C_DFP v1.1.0
 */
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "Timer0A.h"
#include "Timer2A.h"
#include "SSI2.h"
#include "LCD.h"

//#define RPM1_Period 656000.0f// Change to #define RPM1_Period 656000.0f (The value needs to be float for the full calculation to work)
#define FREQUENCY 8000000.0f
#define SEC_PER_MIN 60
#define GEARBOX_RATIO 120
#define PULSE_PER_ROTATION 8

void delayMs(int n);

int main(void) {
    //Varible to hold value of Rpm calculation
    uint32_t RPM = 0;
    //Function calls for various setups
    PLL_Init(Bus8MHz);
    Timer0A_Init(8000000);
    Timer2A_Init();
    SSI2_init();
    LCD_init();
    int pw = 0;

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

    while(1) {
        // set direction
        GPIO_PORTB_DATA_R &= ~0x04;
        GPIO_PORTB_DATA_R |= 0x08;

        //NOTE: MAX speed (160 rpm) seems to have a period of around 4000/4100 (CC_Difference)
        //THOUGHT PROCESS: 160 rpm is around 2.6666667 rotations per second (rps) so normally
        //you would divide the 160 rpm by the 2.666667 which would roughly give you 60 rpm which is 1 rps.
        //This number I think is useful because if you can find the period (CC_Difference) of 60 rpm
        //all you would need to do is multiply that period by 60 to get the period of 1 rpm
        //Then you can create the rpm formula as follows: CC_Difference_of_1rpm/CC_Difference = rpm to display
        //For example if CC_Difference of 1rpm was 1000 and the CC_Difference was 50 that would equal 20 rpm.
        //NOTE: Using the Thoughts above if 160 rpm has a period of 4000 than we would multiply this by 2.666667 to get
        //period of 60 rpm which is 10667 (CC_Difference) multiply that by 60 to get the period of 1 rpm which would be 640000

        //I should next test if 10667 is the period of 60 rpm
        //Idea 1: This can be done in a few ways I could use a voltage divder to send 3v into the board and because 60 rpm is max speed at
        //this voltage I could see what period shows on the LCD and if the calculations are correct
        //Idea 2: I could just try to formula as is for now and test if the numbers make sense when displayed on LCD
        //(for now whole numbers are okay than I can implement the fixed notation after using the slides)
        //656000

        //Formula works
        //To use pw to chage the rpm to your liking just assume that 3999 is 160 RPM.
        //So you can get 60 RPM by do 60/160 = 0.375. Than use this 0.375 and multiply it by the 3999 to get 1499
        //Which gives the 60 rpm
        //Tested trying to get 60 rpm using the equation above. I got 56 rpm which is a 7 percent error percent
        //I used the equation: Percentage Erorr = ((Estimated Number – Actual Number)/ Actual number) x 100.

        // ramp up speed
        for (pw = 100; pw < 3999; pw += 20) {//Max PW 3999 (Which is MAX speed) Start from 100
            PWM1_3_CMPB_R = pw;
            Timer0A_Wait1ms(100);
            LCD_Clear();
            RPM = (SEC_PER_MIN*FREQUENCY)/(PULSE_PER_ROTATION*GEARBOX_RATIO*CC_Difference)*10;
            LCD_OutUFix(RPM);
        }
        // ramp down speed
        for (pw = 3940; pw >100; pw -= 20) {//Min PW 100(Which is pretty much min speed) Start from 3900
            PWM1_3_CMPB_R = pw;
            Timer0A_Wait1ms(100);
            LCD_Clear();
            RPM = (SEC_PER_MIN*FREQUENCY)/(PULSE_PER_ROTATION*GEARBOX_RATIO*CC_Difference)*10;
            LCD_OutUFix(RPM);
        }

        // reverse direction
        GPIO_PORTB_DATA_R &= ~0x08;
        GPIO_PORTB_DATA_R |= 0x04;
        // ramp up speed
        for (pw = 100; pw < 3999; pw += 20) {//Max PW 3999 (Which is MAX speed) Start from 100
            PWM1_3_CMPB_R = pw;
            Timer0A_Wait1ms(100);
            LCD_Clear();
            RPM = (SEC_PER_MIN*FREQUENCY)/(PULSE_PER_ROTATION*GEARBOX_RATIO*CC_Difference)*10;
            LCD_OutUFix(RPM);
        }
        // ramp down speed
        for (pw = 3940; pw >100; pw -= 20) {//Min PW 100(Which is pretty much min speed) Start from 3900
            PWM1_3_CMPB_R = pw;
            Timer0A_Wait1ms(100);
            LCD_Clear();
            RPM = (SEC_PER_MIN*FREQUENCY)/(PULSE_PER_ROTATION*GEARBOX_RATIO*CC_Difference)*10;
            LCD_OutUFix(RPM);
        }
    }
}

/* delay n milliseconds (50 MHz CPU clock) */
void delayMs(int n) {
    int i, j;
    for(i = 0 ; i< n; i++)
        for(j = 0; j < 6265; j++)
            {}  /* do nothing for 1 ms */
}
