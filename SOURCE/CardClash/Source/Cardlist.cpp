/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Cardlist.cpp
 * Author(s):   Yar Xuan (80%)
 * 				Xiong Yang (20%) (updateDiscardPile)
 *
 * Description: Functions for loading cardlist from cardlist.json and all utility functions related to cardlist and Player struct. 
 ******************************************************************************/
 
 // for reading json file & parsing it
#include <iostream>
#include <fstream>
#include <sstream>
#include "rapidjson/document.h"

#include <vector>
#include "Cardlist.h"

// initialize extern variables declared in Cardlist.h
const int NORMAL_CARDS_COUNT = 25; // id 0 is joker, 1-24 number cards
const int POWER_CARDS_COUNT = 14; 
std::vector<Card> cardlist, selectedCards, discardedCards;
struct Player currentPlayer, otherPlayer;

void Cardlist_Load() {
	using namespace std;
	using namespace rapidjson;

	// read cardlist json file into json_str as a string
	ifstream file("Input/cardlist.json");
	ostringstream tmp;
	tmp << file.rdbuf();
	string json = tmp.str();
	const char* json_str = json.c_str();

	// create Document object to parse json string
	Document d;
	d.Parse(json_str);
	const Value& cards = d["cards"];

	// loop through cards array and add to cardlist array
	for (SizeType i = 0; i < cards.Size(); i++) {
		const Value& card = cards[i];
		cardlist.push_back({
			card["id"].GetInt(),
			std::string(card["label"].GetString()),
			std::string(card["type"].GetString()),
			std::string(card["description"].GetString())
		});
	}
}

void updateDiscardPile(std::string cardsStr) {
	std::istringstream iss(cardsStr); // Create a string stream from the input string
	std::vector<int> cardIDs{};     // Vector to store the extracted numbers
	int cardID{};

	// Extract numbers from the stream
	while (iss >> cardID) {
		cardIDs.push_back(cardID);
	}

	// Add each card to discard pile
	for (int id : cardIDs) {
		auto it = std::find_if(cardlist.begin(), cardlist.end(), [id](const Card& card) {
			return card.id == id;
		});

		if (it != cardlist.end()) {
			discardedCards.push_back(*it);
		}
	}
}

void Cardlist_Free() {
	cardlist.clear();
	std::vector<Card>().swap(cardlist); // Force deallocation
}

std::string Player::getCardsStr() {
	std::string result;
	for (const auto& card : card_hand) {
		if (!result.empty()) {
			result += " "; // Add a space between elements
		}
		result += std::to_string(card.id);
	}

	for (const auto& card : power_cards) {
		if (!result.empty()) {
			result += " "; // Add a space between elements
		}
		result += std::to_string(card.id);
	}

	return result;
}

void Player::setCardHand(std::string cardsStr) {
	std::istringstream iss(cardsStr); // Create a string stream from the input string
	std::vector<int> cardIDs{};     // Vector to store the extracted numbers
	int cardID{};

	// Extract numbers from the stream
	while (iss >> cardID) {
		cardIDs.push_back(cardID);
	}

	// set card objects to player card hand
	std::vector<Card> newHand{}, newPowerHand{};

	for (int id : cardIDs) {
		for (auto& card : cardlist) {
			if (id == card.id) {
				if (card.type == "number") {
					newHand.push_back(card);
				}
				else if (card.type == "power") {
					newPowerHand.push_back(card);
				}
				break;
			}
		}
	}

	card_hand = newHand;
	power_cards = newPowerHand;
}