#include <cstdlib>
#include <stdint.h>
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <Sound.h>
#include "WordleGame.h"
#include <ST7735.h>
#include "Lab9HMain.h"

const char* wordList[] = {
"WHICH", "THERE", "THEIR", "ABOUT", "WOULD", "THESE", "OTHER", "WORDS",
"COULD", "WRITE", "FIRST", "WATER", "AFTER", "WHERE", "RIGHT", "THINK",
"THREE", "YEARS", "PLACE", "SOUND", "GREAT", "AGAIN", "STILL", "EVERY",
"SMALL", "FOUND", "THOSE", "NEVER", "UNDER", "MIGHT", "WHILE", "HOUSE",
"WORLD", "BELOW", "ASKED", "GOING", "LARGE", "UNTIL", "ALONG", "SHALL",
"BEING", "OFTEN", "EARTH", "BEGAN", "SINCE", "STUDY", "NIGHT", "LIGHT",
"ABOVE", "PAPER", "PARTS", "YOUNG", "STORY", "POINT", "TIMES", "HEARD",
"WHOLE", "WHITE", "GIVEN", "MEANS", "MUSIC", "MILES", "THING", "TODAY",
"LATER", "USING", "MONEY", "LINES", "ORDER", "GROUP", "AMONG", "LEARN",
"KNOWN", "SPACE", "TABLE", "EARLY", "TREES", "SHORT", "HANDS", "STATE",
"BLACK", "SHOWN", "STOOD", "FRONT", "VOICE", "KINDS", "MAKES", "COMES",
"CLOSE", "POWER", "LIVED", "VOWEL", "TAKEN", "BUILT", "HEART", "READY",
"QUITE", "CLASS", "BRING", "ROUND", "HORSE", "SHOWS", "PIECE", "GREEN",
"STAND", "BIRDS", "START", "RIVER", "TRIED", "LEAST", "FIELD", "WHOSE",
"GIRLS", "LEAVE", "ADDED", "COLOR", "THIRD", "HOURS", "MOVED", "PLANT",
"DOING", "NAMES", "FORMS", "HEAVY", "IDEAS", "CRIED", "CHECK", "FLOOR",
"BEGIN", "WOMAN", "ALONE", "PLANE", "SPELL", "WATCH", "CARRY", "WROTE",
"CLEAR", "NAMED", "BOOKS", "CHILD", "GLASS", "HUMAN", "TAKES", "PARTY",
"BUILDS", "SEEMS", "OPENED", "WORMS", "GUESS", "ACTION", "YOUNG", "WOULD",
};

char targetWord[6]; // Current target word
uint8_t letterStatus[6][5]; // Status of each letter (0=not guessed, 1=incorrect, 2=wrong position, 3=correct)

//currentRow = 0; // Current guess row (0-5)
//currentGuess[6] = "     "; // Current word being guessed (5 letters + null terminator)
//score = 0; // Player's score
//gameRunning = true;
//gameWon = false;

uint8_t currentRow; // Current guess row (0-5)
char currentGuess[6]; // Current word being guessed (5 letters + null terminator)
uint16_t score; // Player's score
uint16_t maxScore;
bool gameRunning;
bool gameWon;

// WordleGame.h
char savedGuesses[6][5]; // Array to store past guesses


void WordleGame_Init(void) {
  // Initialize randomizer
  uint32_t seed = 1;
  uint32_t M = 1;
  M = 1664525*M+1013904223;
  seed = M;
}
char colon = ':';
char* space = "     ";
char* CHScore = "Score";
void drawGameBoard(void) {
  ST7735_FillScreen(ST7735_BLACK);
  
  // Draw score at top
  ST7735_SetCursor(1, 0);
  ST7735_OutString((char*)Phrases[score][myLanguage]);
  ST7735_OutString((char*)space);
  ST7735_OutString((char*)CHScore);
  ST7735_OutChar(colon);
  ST7735_OutUDec(maxScore);
  
  // Draw game grid (6 rows x 5 columns)
  for(int row = 0; row < 6; row++) {
    for(int col = 0; col < 5; col++) {
      drawLetterBox(row, col);
    }
  }
  
  // Draw current guess row indicator
  ST7735_DrawBitmap(5, 30 + currentRow*20, GuessMarker, 5, 5);
  // Draw the letter

}


void startNewGame(void) {
  // Reset game state
  currentRow = 0;
  gameRunning = true;
  gameWon = false;
  score = 0;
  maxScore = 0;
  
  // Clear current guess
  for(int i = 0; i < 5; i++) {
    currentGuess[i] = ' ';
  }
  
  // Reset letter statuses
  for(int i = 0; i < 6; i++) {
    for(int j = 0; j < 5; j++) {
      letterStatus[i][j] = 0;
    }
  }
  
  // Select random word
  uint32_t wordIndex = rand() % (sizeof(wordList)/sizeof(wordList[0]));
  strcpy(targetWord, wordList[wordIndex]);
  
  // Draw initial game board
  drawGameBoard();
}

bool isWordComplete(void) {
  // Check if all 5 letters have been filled
  for(int i = 0; i < 5; i++) {
    if(currentGuess[i] == ' ') return false;
  }
  return true;
}

void evaluateGuess(void) {
  // Copy current guess to current row
  bool correctLetters[5] = {false, false, false, false, false};
  bool usedLetters[5] = {false, false, false, false, false};
  score = 0;
  // First pass: check for exact matches (green)
  for(int i = 0; i < 5; i++) {
    if(currentGuess[i] == targetWord[i]) {
      letterStatus[currentRow][i] = 3; // Correct position
      correctLetters[i] = true;
      usedLetters[i] = true;
      score += 10; // Add 10 points for green match
    }
  }
  
  // Second pass: check for letters in wrong positions (yellow)
  for(int i = 0; i < 5; i++) {
    if(letterStatus[currentRow][i] == 0) { // Not yet marked
      bool found = false;
      for(int j = 0; j < 5 && !found; j++) {
        if(!usedLetters[j] && !correctLetters[j] && currentGuess[i] == targetWord[j]) {
          letterStatus[currentRow][i] = 2; // Wrong position
          usedLetters[j] = true;
          found = true;
          score += 5; // Add 5 points for yellow match
        }
      }
      
      if(!found) {
        letterStatus[currentRow][i] = 1; // Not in word
      }
    }
  }
  
  // Check if word is correct
  bool allCorrect = true;
  for(int i = 0; i < 5; i++) {
    if(letterStatus[currentRow][i] != 3) {
      allCorrect = false;
      break;
    }
  }
  if (maxScore < score)
  {
    maxScore = score;
  }
  if(allCorrect) {
    gameWon = true;
    Sound_Highpitch(); // Play victory sound
  } else if(currentRow == 5) {
    // Last guess and not correct
    Sound_Explosion(); // Play losing sound
  }
}


void drawLetterBox(uint8_t row, uint8_t col) {
  uint16_t color;
  
  // Determine box color based on letter status
  if(row < currentRow) {
    // Past guesses - colored based on evaluation
    switch(letterStatus[row][col]) {
      case 1: color = ST7735_LIGHTGREY; break;   // Not in word
      case 2: color = ST7735_YELLOW; break; // Wrong position
      case 3: color = ST7735_GREEN; break;  // Correct position
      default: color = ST7735_WHITE; break; // Not evaluated
    }
  } else {
    // Current or future guesses - white boxes
    color = ST7735_WHITE;
  }
  
  // Draw box outline
  ST7735_FillRect(20 + col*20, 30 + row*20, 15, 15, color);
  
  // Draw letter if present
  if(row < currentRow) {
    // Past guesses
    char letter[2] = {' ', 0};
    letter[0] = savedGuesses[row][col];
    ST7735_SetCursor(4 + col*3, 3 + row*2);
    ST7735_OutString(letter);
  } else if(row == currentRow) {
    // Current guess
    char letter[2] = {' ', 0};
    letter[0] = currentGuess[col];
    ST7735_SetCursor(4 + col*3.5, 3 + row*2);
    ST7735_OutString(letter);
  }
  
  // Draw cursor for current position
  if(row == currentRow && col == cursorPosition) {
    ST7735_DrawBitmap(20 + col*20, 46 + row*20, CursorBox, 15, 3);
  }
}

void updateGameDisplay(void) {
  // Redraw just what needs to be updated
  drawGameBoard();
}

void displayEndGameScreen(void) {
  ST7735_FillScreen(ST7735_BLACK);
  
  if(gameWon) {
    // Display win message and animation
    ST7735_SetCursor(3, 3);
    ST7735_OutString((char*)Phrases[3][myLanguage]);
    
    // Display celebration animation
    for(int i = 0; i < 20; i++) {
      // Draw confetti animation
      ST7735_DrawBitmap(rand()%(128), rand()%(160), Confetti, 5, 5);
      Clock_Delay1ms(100);
    }
  } else {
    // Display lose message and show correct word
    ST7735_SetCursor(3, 3);
    ST7735_OutString((char*)Phrases[1][myLanguage]);
    ST7735_SetCursor(2, 5);
    ST7735_OutString((char*)Phrases[2][myLanguage]);
    ST7735_OutString(&colon);
    ST7735_OutString((char*)targetWord);
  }
  
  // Display final score
  ST7735_SetCursor(3, 7);
  ST7735_OutString((char*)Phrases[0][myLanguage]);
  ST7735_OutString(&colon);
  ST7735_OutUDec(score);
  
  // Display restart prompt
  ST7735_SetCursor(1, 10);
  ST7735_OutString((char*)Phrases[1][myLanguage]);
}
