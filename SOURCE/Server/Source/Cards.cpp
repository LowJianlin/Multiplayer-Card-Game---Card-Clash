/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Cards.cpp
 * Author(s):   Yar Xuan (100%)
 *
 * Description: Functions for loading cardlist from cardlist.json and all utility functions related to cardlist.
 ******************************************************************************/

#define _WIN32_WINNT 0x0601 // Target Windows 7 or later
// for reading json file & parsing it
#include <iostream>
#include <fstream>
#include <sstream>
#include "rapidjson/document.h"

// for RNG and shuffling the cards array
#include <random>   
#include <algorithm>
#include <vector>

#include "Cards.h"
#include "Server.h"
#include "Player.h"

const int NORMAL_CARDS_COUNT = 25; // id 0 is joker, 1-24 number cards
const int POWER_CARDS_COUNT = 14;

// initialize externs declared in Cards.h
std::vector<Card> cardlist{};
std::map<int, std::vector<Card>> powerDecks;

void LoadCardList() {
	std::cout << "[DEBUG] Loading cardlist.json" << std::endl;
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
	std::cout << "[DEBUG] loaded cardlist.json" << std::endl;
}

void FreeCardlist() {
	cardlist.clear();
	std::vector<Card>().swap(cardlist); // Force deallocation

	for (auto& pair : powerDecks) {
        pair.second.clear();
        std::vector<Card>().swap(pair.second); // Force deallocation of the vector
    }
    powerDecks.clear(); // Clear the map itself
}

void distributeCards(Player& currentPlayer, Player* otherPlayer) {
	std::cout << "[DEBUG] Distributing cards" << std::endl;
	// create an array of normal cards, copied from original cardlist to avoid modifying the original
	std::vector<Card> cardsCopy(cardlist.begin(), cardlist.begin() + NORMAL_CARDS_COUNT);

	// shuffle the normal cards within the copy
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(cardsCopy.begin(), cardsCopy.end(), g);

	std::vector<std::string> firstPlayerHand{}, secondPlayerHand{};

	// distribute first half of shuffled cards (12 cards) to 1st player, 2nd half (13 cards) to 2nd player
	for (int i = 0; i < NORMAL_CARDS_COUNT / 2; i++) {
		firstPlayerHand.push_back(std::to_string(cardsCopy[i].id));
	}
	for (int i = NORMAL_CARDS_COUNT / 2; i < NORMAL_CARDS_COUNT; i++) {
		secondPlayerHand.push_back(std::to_string(cardsCopy[i].id));
	}

	currentPlayer.setCardHand(currentPlayer.getStartsFirst() ? firstPlayerHand : secondPlayerHand);
	otherPlayer->setCardHand(otherPlayer->getStartsFirst() ? firstPlayerHand : secondPlayerHand);
}

void populatePowerDeck(int roomId) {
	std::cout << "[DEBUG] Populating power deck for room ID: " << std::to_string(roomId) << std::endl;
	
	// create an array of normal cards, copied from original cardlist to avoid modifying the original
	std::vector<Card> cardsCopy(cardlist.begin() + NORMAL_CARDS_COUNT, cardlist.end());
	// empty power deck
	std::vector<Card> powerCardDeck;

	for (auto& c : cardsCopy) {
		if (c.type == "power") {
			powerCardDeck.push_back(c);
		}
	}

	powerDecks[roomId] = powerCardDeck; 
}

int drawPowerCard(int roomId) {
	std::cout << "[DEBUG] Player picked a card from power deck!" << std::endl;
	std::vector<Card>& deck = powerDecks[roomId];

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, static_cast<int>(deck.size()) - 1);

	int randomIndex = dis(gen);
	int powerCardID = deck[randomIndex].id;

	// remove card from deck
	deck.erase(deck.begin() + randomIndex);

	return powerCardID;
}

std::string concatWithSpace(const std::vector<std::string>& vec) {
	std::string result;
	for (const auto& str : vec) {
		if (!result.empty()) {
			result += " "; // Add a space between elements
		}
		result += str;
	}
	return result;
}

Card* findCardById(int targetId) {
	auto it = std::find_if(cardlist.begin(), cardlist.end(), [targetId](const Card& card) {
		return card.id == targetId;
		});

	if (it != cardlist.end()) {
		return &(*it); // Return a pointer to the found Card
	}
	return nullptr; // Return nullptr if not found
}