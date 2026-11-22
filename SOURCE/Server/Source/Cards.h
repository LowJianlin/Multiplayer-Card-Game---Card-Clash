/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Cards.h
 * Author(s):   Yar Xuan (100%)
 *
 * Description: Header file for Cards.cpp.
 ******************************************************************************/

#ifndef CARDS_H // Include guard to prevent multiple inclusions
#define CARDS_H

#include <vector>
#include <string>
#include "Server.h"
#include "Player.h"

enum POWER_TYPES
{
    SWAP_HANDS = 25,
    REVEAL_HAND = 27,
    GIVE_CARD = 31,
    SWAP_POWERS = 35,
    END = 38
};

struct Card {
	int id;
	std::string label, type, description;
};

extern std::vector<Card> cardlist;
extern std::map<int, std::vector<Card>> powerDecks;

void LoadCardList();
void FreeCardlist();
void distributeCards(Player& currentPlayer, Player* otherPlayer);
void populatePowerDeck(int roomId);
int drawPowerCard(int roomId);
extern std::string concatWithSpace(const std::vector<std::string>& vec); //concat Player::card_hand into one string to send to clients
Card* findCardById(int targetId);

#endif