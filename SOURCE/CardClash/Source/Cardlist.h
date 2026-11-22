/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Cardlist.h
 * Author(s):   Yar Xuan (80%)
 * 				Xiong Yang (20%) (updateDiscardPile)
 * 
 * Description: Header file for Cardlist.cpp. Includes Player and Card structs.
 ******************************************************************************/
#ifndef CARDLIST_H // Include guard to prevent multiple inclusions
#define CARDLIST_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream> 
#include "AEEngine.h"

extern const int NORMAL_CARDS_COUNT, POWER_CARDS_COUNT; 

struct Card {
    int id{};
	std::string label, type, description;
    AEVec2 position{};
	bool isPlaced = false;


	// Overloading the '==' operator to compare two cards
	bool operator==(const Card& other) const {
		return id == other.id;  // Compare based on the card's id
	}
};

extern std::vector<Card> cardlist, selectedCards, discardedCards;

void Cardlist_Load();
void updateDiscardPile(std::string cardsStr);
void Cardlist_Free();

struct Player {
    std::vector<Card> card_hand{}, power_cards{};
    bool is_player_turn = false;
    std::string turn_phase = "";
    int playerID = -1;
    bool is_winner = false;
    int getID() const {
        return playerID;
    }

    void setID(int id) {
        playerID = id;
    }

    std::string getCardsStr();
    void setCardHand(std::string cardsStr);
};


extern struct Player currentPlayer, otherPlayer;

#endif