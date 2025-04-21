/*
 * Switch.cpp
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this
  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA27INDEX] = 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA16INDEX] = 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA15INDEX] = 0x00040081; // input, no pull
}
// return current state of switches
uint32_t Switch_In(void){
    // write this
  uint32_t data = GPIOA->DIN31_0;
  data = ((data>>15)&0x01) | ((data&((1<<28)|(1<<27)))>>25);
  return data; // return 0; //replace this your code
}
void Button_IntArm(uint8_t priority) {
  // Configure button interrupts (edge triggered)
  GPIOA->CPU_INT.IMASK |= (1<<28)|(1<<27)|(1<<15); // Enable interrupts for pins PA28, PA27, PA16
  
  // Set to edge triggered 
  GPIOA->CPU_INT.ISET |= (1<<28)|(1<<27)|(1<<15);  // Edge triggered
  
  // Clear any pending interrupts
  GPIOA->CPU_INT.ICLR |= (1<<28)|(1<<27)|(1<<15);  // Clear any pending interrupts
  
  // Set priority in NVIC
  NVIC->IP[18] = priority << 5;  // GPIOA IRQ is 18
  NVIC->ISER[0] = 1 << 18;       // Enable IRQ 18
}