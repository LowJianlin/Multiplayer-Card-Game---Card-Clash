/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Gameboard.h
 * Author(s):   Edthan Yeo (25%) (draw power card hand, draw power card deck, game end screen, sort/shuffle cards)
 *              Xiong Yang (25%) (card selection, play sets of 4, play power card, turn information text, draw discard pile, skip turn)
 *              Yar Xuan (25%) (turn timer, card drag & drop to re-arrange, power card effects, waiting room, pick card from opponent)
 *              Low Jianlin (25%) (draw own and opponent card hands and power cards, main gameboard UI)
 *
 * Description: Header file for gameboard game state.
 ******************************************************************************/

#ifndef GAMEBOARD_H // Include guard to prevent multiple inclusions
#define GAMEBOARD_H

#include "Cardlist.h" // Assuming Player is defined in Cardlist.h

// Function declarations for the Gameboard
void Gameboard_Load();
void Gameboard_Initialize();
void Gameboard_Update();
void Gameboard_Draw();
void Gameboard_Free();
void Gameboard_Unload();

// Extern declarations to make currentPlayer and otherPlayer accessible globally
extern Player currentPlayer;
extern Player otherPlayer;
extern bool turnPlayed, waitingRoom;	
extern bool revealHand;
extern bool givingCard;

// Extern declaration for messages
extern std::string errorMessage, infoMessage;

#endif