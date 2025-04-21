// Sound.cpp
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

static const uint8_t *SoundPtr;     // Pointer to current sound data
static uint32_t SoundIndex;         // Current position in sound array
static uint32_t SoundLength;        // Total length of current sound
static uint8_t SoundPlaying = 0;    // Flag indicating if sound is active

void SysTick_IntArm(uint32_t period, uint32_t priority){
  // write this
  SysTick->LOAD = period - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = 0x00000007; 
  NVIC->IP[15] = priority << 5; 
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5 bit DAC
void Sound_Init(void){
// write this
  DAC5_Init();
}
extern "C" void SysTick_Handler(void);
/*
void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
    // output one value to DAC if a sound is active

}
*/

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
// write this
  SoundPtr = pt;           // Set pointer to sound data
  SoundLength = count;     // Set total length
  SoundIndex = 0;          // Start at beginning of sound
  SoundPlaying = 1;        // Mark sound as active
}

void Sound_Shoot(void){
// write this
  Sound_Start( shoot, 4080);
}
void Sound_Killed(void){
// write this

}

void Sound_Fastinvader1(void){

}
void Sound_Fastinvader2(void){

}
void Sound_Fastinvader3(void){

}
void Sound_Fastinvader4(void){

}

void Sound_Highpitch(void) {
  // Victory sound when player wins
  Sound_Start(highpitch, 4000);
}

void Sound_Explosion(void) {
  // Losing sound when player fails
  Sound_Start(explosion, 4000);
}

//******* Sound_Update ************
// Called at 11kHz from SysTick_Handler
// Outputs the next sample of the current sound to the DAC
// If the sound has completed, it stops the output
// Input: none
// Output: none
void Sound_Update(void) {
  if(SoundPlaying) {
    // Output current sound sample to the 5-bit DAC
    DAC5_Out(SoundPtr[SoundIndex]);
    
    // Move to next sample
    SoundIndex++;
    
    // Check if we've reached the end of the sound
    if(SoundIndex >= SoundLength) {
      // Sound is complete, stop playing
      SoundPlaying = 0;
      // Output zero (silence) to DAC
      DAC5_Out(0);
    }
  } else {
    // No active sound, output silence
    DAC5_Out(0);
  }
}


// SysTick handler for sound output
void SysTick_Handler(void) {
  // Output one value to DAC if a sound is active
  Sound_Update();
}