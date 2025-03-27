/*
 * SSEG.h
 *
 *  Created on: Mar 24, 2024
 *      Author: domin
 */

#ifndef SSEG_H_
#define SSEG_H_

//Global Variables
const static uint8_t digitPattern[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99,
                                             0x92, 0x82, 0xF8, 0x80, 0x90};
volatile uint8_t digitPattern_count; //Used to be just int i;

void SSEG1(uint32_t Thread_Id);
void SSEG2(uint32_t Time_Slice);
void sevenseg_init(void);
void SSI2_write(uint8_t data, uint8_t csMask);


#endif /* SSEG_H_ */
