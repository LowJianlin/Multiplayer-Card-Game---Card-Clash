/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        GameUtils.h
 * Author(s):   Edthan Yeo (80%) (draw power card hand, draw power card deck, sort/shuffle cards, game end screen)
 *              Xiong Yang (20%) (playCards, draw discard pile)
 *
 * Description: Header file for GameUtils.cpp.
 ******************************************************************************/

#ifndef GAMEUTILS_H // Include guard to prevent multiple inclusions
#define GAMEUTILS_H

#include "AEEngine.h"
#include "Cardlist.h" 
#include <vector>

extern AEGfxVertexList* rectMesh;
extern AEMtx33 cardScale, cardTranslate;
extern AEVec2 cardSize;
extern s8 font;
extern AEVec2 powerDeckPosition, discardedDeckPosition;
extern bool isPowerDeckHovered;
extern int powerDeckSize;
extern f64 turnStartTime;

extern void playCards(std::vector<Card>& cardsToPlay);
extern void drawCard(Card card, bool hideCard = false);
extern void drawPowerDeck();
extern void drawDiscardPile();
bool ifClickFunction(AEVec2 buttonPos, float width, float height);

extern std::vector<Card> selectedCards;

// Texture pointers for power cards
extern AEGfxTexture* giveCardTexture;
extern AEGfxTexture* revealHandTexture;
extern AEGfxTexture* swapHandsTexture;
extern AEGfxTexture* reshuffleTexture;

extern AEGfxTexture* JokerCardTexture;
extern AEGfxTexture* No1CardTexture;
extern AEGfxTexture* No2CardTexture;
extern AEGfxTexture* No3CardTexture;
extern AEGfxTexture* No4CardTexture;
extern AEGfxTexture* No5CardTexture;
extern AEGfxTexture* No6CardTexture;
extern AEGfxTexture* OpponentBack;

// Function declarations
void LoadPowerCardTextures();
void DrawPowerCard(std::string& label, float x, float y);
void DrawOppPowerCard(float x, float y);
void FreePowerCardTextures();

extern AEGfxTexture* winScreenTexture;
extern AEGfxTexture* loseScreenTexture;
void LoadGameEndTextures();
void FreeGameEndTextures();

void LoadCardTextures();
void FreeCardTextures();

void SortPlayerHand();
void ShufflePlayerHand();

#endif