/*
 * Switch.h
 *
 *  Created on: Nov 5, 2023
 *      Author: jonat
 */

#ifndef SWITCH_H_
#define SWITCH_H_

// initialize your switches
void Switch_Init(void);

// return current state of switches
uint32_t Switch_In(void);

void Button_IntArm(uint8_t priority);

#endif /* SWITCH_H_ */