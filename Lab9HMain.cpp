// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Your name
// Last Modified: 12/26/2024

// Wordle.cpp - Main game implementation for MSPM0
#include <stdint.h>
#include <stdio.h>
#include <cstdint>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "WordleGame.h" // Custom game logic header

// Global variables for game state
SlidePot Cursor(1700, 0); // Slide pot for cursor movement
uint8_t cursorPosition = 0; // Current cursor position (0-4)

bool semaphore = false; // For synchronization between ISRs and main loop
//Language_t currentLanguage = English; // Default language

extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);
extern "C" void GPIOA_IRQHandler(void);
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

SlidePot Sensor(1500,0); // copy calibration from Lab 7
void sleep(uint32_t milliseconds) {
    // Number of iterations per millisecond (calibrated for ~80 MHz)
    const uint32_t iterations_per_ms = 1000;

    for (uint32_t i = 0; i < milliseconds; ++i) {
        for (volatile uint32_t j = 0; j < iterations_per_ms; ++j) {
            // Do nothing (just waste time)
        }
    }
}
// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    
    // Sample slide pot to get cursor position
    int currpos = Cursor.In();
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    uint32_t newPos = Cursor.Convert(currpos) / 300; // 1500/5 = 300 (5 positions)
    
    

    if(newPos > 4) {newPos = 4;}
    
    // Only update if position changed
    if(newPos != cursorPosition) {
      cursorPosition = newPos;
      semaphore = true; // Set semaphore for main to update display
    }
    uint32_t pa27in = GPIOA->DIN27_24;
    uint32_t pa28in = GPIOA->DIN31_28;
    uint32_t pa15in = GPIOA->DIN15_12;
    uint32_t status = GPIOA->CPU_INT.IIDX;
  
  if(pa27in == 16777216) { // Forward button pressed
    // Select next letter (A→B→C...)
    if(currentGuess[cursorPosition] == ' ' || currentGuess[cursorPosition] == 'Z') {
      currentGuess[cursorPosition] = 'A';
      drawLetterBox(currentRow, cursorPosition);
      sleep(1000);
    } else {
      currentGuess[cursorPosition]++;
      drawLetterBox(currentRow, cursorPosition);
      sleep(1000);
    }
    semaphore = true;
  }
  else if(pa28in == 1) { // Backward button pressed
    // Select previous letter (Z→Y→X...)
    if(currentGuess[cursorPosition] == ' ' || currentGuess[cursorPosition] == 'A') {
      currentGuess[cursorPosition] = 'Z';
      drawLetterBox(currentRow, cursorPosition);
      sleep(1000);
    } else {
      currentGuess[cursorPosition]--;
      drawLetterBox(currentRow, cursorPosition);
      sleep(1000);
    }
    semaphore = true;
  }
  else if(pa15in == 16777216) { // Submit button pressed
    if(isWordComplete()) {
      evaluateGuess();
      
      // Move to next row or end game
      currentRow++;
      if(currentRow >= 6 || gameWon) {
        gameRunning = false;
      } else {
        // Reset current guess for next row
        for(int i = 0; i < 5; i++) {
          currentGuess[i] = ' ';
        }
      }
      semaphore = true;
    }
  }
  
  // Clear interrupt flag
  GPIOA->CPU_INT.IIDX = (1 << status);
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}

void GPIOA_IRQHandler(void) {
  uint32_t status = GPIOA->CPU_INT.IIDX;
  
  if(status == PA27INDEX) { // Forward button pressed
    // Select next letter (A→B→C...)
    if(currentGuess[cursorPosition] == ' ' || currentGuess[cursorPosition] == 'Z') {
      currentGuess[cursorPosition] = 'A';
    } else {
      currentGuess[cursorPosition]++;
    }
    semaphore = true;
  }
  else if(status == PA28INDEX) { // Backward button pressed
    // Select previous letter (Z→Y→X...)
    if(currentGuess[cursorPosition] == ' ' || currentGuess[cursorPosition] == 'A') {
      currentGuess[cursorPosition] = 'Z';
    } else {
      currentGuess[cursorPosition]--;
    }
    semaphore = true;
  }
  else if(status == PA16INDEX) { // Submit button pressed
    if(isWordComplete()) {
      evaluateGuess();
      
      // Move to next row or end game
      currentRow++;
      if(currentRow >= 6 || gameWon) {
        gameRunning = false;
      } else {
        // Reset current guess for next row
        for(int i = 0; i < 5; i++) {
          currentGuess[i] = ' ';
        }
      }
      semaphore = true;
    }
  }
  
  // Clear interrupt flag
  GPIOA->CPU_INT.IIDX = (1 << status);
}



uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

//typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
//typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
// use main1 to observe special characters
/*int main(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(int myPhrase=0; myPhrase<= 2; myPhrase++){
    for(int myL=0; myL<= 3; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}*/
void waitforbutton(void) {
  uint8_t butSel = 0;
  while (butSel == 0) {
    if (Switch_In() & 0x01) { 
        butSel = 1; // Submit
    } else if (Switch_In() & 0x08) {
        butSel = 2; // Reverse
    } else if (Switch_In() & 0x04) {
        butSel = 3; //Forward
    }
  }
  switch (butSel) {
    case 1:
    if(isWordComplete()) {
      evaluateGuess();
      
      // Move to next row or end game
      currentRow++;
      if(currentRow >= 6 || gameWon) {
        gameRunning = false;
      } else {
        // Reset current guess for next row
        for(int i = 0; i < 5; i++) {
          currentGuess[i] = ' ';
        }
      }
      semaphore = false;
    }
      drawLetterBox(currentRow, cursorPosition);
      break;
    case 2:
    if(currentGuess[cursorPosition] == ' ' || currentGuess[cursorPosition] == 'A') {
      currentGuess[cursorPosition] = 'Z';
      drawLetterBox(currentRow, cursorPosition);
      break;
    } else {
      currentGuess[cursorPosition]--;
      drawLetterBox(currentRow, cursorPosition);
      break;
    }
    semaphore = false;
      
      break;
    case 3:
    if(currentGuess[cursorPosition] == ' ' || currentGuess[cursorPosition] == 'Z') {
      currentGuess[cursorPosition] = 'A';
      drawLetterBox(currentRow, cursorPosition);
      break;
    } else {
      currentGuess[cursorPosition]++;
      drawLetterBox(currentRow, cursorPosition);
      break;
    }
    semaphore = false;
      break;
    default:
      // Handle invalid input
      break;
  }
}
void selectLanguage(void) {
  // Display language selection menu on the ST7735 screen
  char selectlang[] = "Select Language:";
  ST7735_OutString(selectlang);
  ST7735_OutChar(13);
  char eng[] = "1. English";
  ST7735_OutString(eng);
  ST7735_OutChar(13);
  char span[] = "2. Spanish";
  ST7735_OutString(span);
  ST7735_OutChar(13);
  char port[] = "3. Portuguese";
  ST7735_OutString(port);
  ST7735_OutChar(13);
  char fre[] = "4. French";
  ST7735_OutString(fre);

  // Wait for user input (e.g., button press or joystick selection)
  uint8_t languageSelection = 0;
  while (languageSelection == 0) {
    // Read user input (e.g., button press or joystick selection)
    if (Switch_In() & 0x01) { // Suppose button press sets the least significant bit
        languageSelection = 1; // English
    } else if (Switch_In() & 0x08) {
        languageSelection = 2; // Spanish
    } else if (Switch_In() & 0x04) {
        languageSelection = 3; // Portuguese
    }
  }

  // Update the myLanguage variable based on the user's selection
  switch (languageSelection) {
    case 1:
      myLanguage = English;
      break;
    case 2:
      myLanguage = Spanish;
      break;
    case 3:
      myLanguage = Portuguese;
      break;
    default:
      // Handle invalid input
      break;
  }


  
}

int main(void) {
  __disable_irq();
  PLL_Init(); // Set bus speed (80MHz)
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(ST7735_BLACK);
  
  // Initialize hardware interfaces
  Cursor.Init(); // Initialize slide pot
  Switch_Init(); // Initialize buttons
  LED_Init();    // Initialize LEDs
  Sound_Init();  // Initialize sound system
  WordleGame_Init(); // Initialize game data
  
  // Set up interrupts
  SysTick_Init();
  SysTick_IntArm(7272, 1); // 11kHz for sound (highest priority)
  TimerG12_Init();
  TimerG12_IntArm(2666667, 0);
  //TIMG12_Init(2666667, 1); // 30Hz for slide pot sampling (priority 1)
  Button_IntArm(2); // Edge triggered for button press (priority 2)
  
  // Show language selection screen
  selectLanguage();
  
  
  // Initialize new game
  startNewGame();
  
  __enable_irq();
  
  while(1) {
    if(semaphore) {
      semaphore = false; // Clear semaphore
      
      // Update display
      updateGameDisplay();
      
      // Check for end game condition
      if(!gameRunning) {
        displayEndGameScreen();
        
        // Wait for button press to restart
        while(!Switch_In());
        startNewGame();
      }
    }
  }
}

// use main2 to observe graphics
/*
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  for(uint32_t t=500;t>0;t=t-5){
    SmallFont_OutVertical(t,104,6); // top left
    Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString((char *)"GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString((char *)"Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString((char *)"Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  while(1){
  }
}
*/

/*
// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
    // write code to test switches and LEDs

  }
}
*/

/*
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}
// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
*/

/*
int main5(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  
  // initialize all data structures
  __enable_irq();

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
}
*/