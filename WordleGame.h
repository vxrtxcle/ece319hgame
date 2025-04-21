// WordleGame.h
#ifndef WORDLEGAME_H_
#define WORDLEGAME_H_

#include <cstdint>
#include <stdint.h>
#include "Lab9HMain.h"



extern uint8_t currentRow; // Current guess row (0-5)
extern char currentGuess[6]; // Current word being guessed (5 letters + null terminator)
extern uint16_t score; // Player's score
extern uint16_t maxScore;
extern bool gameRunning;
extern bool gameWon;

// WordleGame.h
extern char savedGuesses[6][5]; // Array to store past guesses


// Cursor image array (15x3 pixels)


const uint16_t CursorBox[] = {
 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF,
 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF
};

const uint16_t Confetti[] = {
 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

// Initialize game data structures
void WordleGame_Init(void);

// Start a new game with random word
void startNewGame(void);

// Check if current word is complete (all 5 letters filled)
bool isWordComplete(void);

// Evaluate current guess against target word
void evaluateGuess(void);

// Draw the game board on the display
void drawGameBoard(void);

// Update the display with latest game state
void updateGameDisplay(void);

// Draw letter box at specified position with appropriate color
void drawLetterBox(uint8_t row, uint8_t col);

// Display end game screen (win/lose)
void displayEndGameScreen(void);

// Language selection screen
void selectLanguage(void);

#endif