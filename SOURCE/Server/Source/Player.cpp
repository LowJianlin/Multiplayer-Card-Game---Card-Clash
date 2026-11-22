/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Player.cpp
 * Author(s):   Yar Xuan (90%) (Player class functions)
 *              Edthan Yeo (10%) (coinflip in assignRoomID)
 *
 * Description: Player class functions.
 ******************************************************************************/

#define _WIN32_WINNT 0x0601 // Target Windows 7 or later

#include <asio.hpp>
#include <iostream>
#include <vector>
#include <memory> // For std::shared_ptr
#include <random> // Include for coin flip
#include <map>

#include "Server.h"
#include "Player.h"
#include "Cards.h"

using SocketPtr = std::shared_ptr<asio::ip::tcp::socket>;
std::map<std::shared_ptr<asio::ip::tcp::socket>, Player> players; // Use shared_ptr for sockets

// Initialize player ID and roomIDs array
int Player::nextID = 1;
std::vector<int> roomIDs = {};

// Player class constructors and functions
Player::Player() {
    playerID = nextID++;  // Assign and increment
    roomID = 0; // default, no room yet
}

int Player::getID() const {
    return playerID;
}

int Player::getRoomID() const {
    return roomID;
}

bool Player::getRoomFull() const {
    return room_full;
}

std::vector<std::string> Player::getCardHand() const {
    return card_hand;
}

std::vector<std::string> Player::getPowerHand() const {
    return power_cards;
}

void Player::setRoomFull(bool val) {
    room_full = val;
}

void Player::setTurn(bool turn) {
    is_player_turn = turn;
}

bool Player::getTurn() const {
    return is_player_turn;
}

bool Player::getStartsFirst() const {
    return starts_first;
}

void Player::setStartsFirst(bool val) {
    starts_first = val;
}

SocketPtr Player::getSocket() const {
    for (auto& [sock, player] : players) {
        if (player.getID() == this->getID()) {
            return sock;
        }
    }
    return nullptr;
}

void Player::addCard(int id) {
    std::vector<std::string> newHand(this->getCardHand());
    newHand.push_back(std::to_string(id));
    this->setCardHand(newHand);
}

void Player::removeCard(int id) {
    std::vector<std::string> newHand{};
    for (auto& cardId : this->getCardHand()) {
        if (cardId != std::to_string(id)) {
            newHand.push_back(cardId);
        }
    }

    this->setCardHand(newHand);
}

void Player::setCardHand(std::vector<std::string> newHand) {
    card_hand = newHand;
}

void Player::setCardHandStr(std::string cardsStr) {
    std::istringstream iss(cardsStr); // Create a string stream from the input string
    std::vector<int> cardIDs;     // Vector to store the extracted numbers
    int cardID;
    std::vector<std::string> newHand;

    // Extract numbers from the stream
    while (iss >> cardID) {
        cardIDs.push_back(cardID);
    }

    // set card objects to player card hand
    for (int id : cardIDs) {
        for (auto& card : cardlist) {
            if (id == card.id) {
                newHand.push_back(std::to_string(card.id));
                break;
            }
        }
    }
    this->setCardHand(newHand);
}

Player& Player::getOpponent() {
    for (auto& [sock, player] : players) {
        if (player.getID() != this->getID() && player.getRoomID() == this->getRoomID()) {
            return player; // Return a reference to the Player object
        }
    }
    static Player nullPlayer; // Default or "null" Player object
    return nullPlayer; // Return the default object
}

void Player::assignRoomID(SocketPtr socket) {
    std::cout << "[INFO] Assigning room ID" << std::endl;
    std::error_code error;
    Player* otherPlayer = nullptr;

    // Loop through players map, find which player room is not full
    std::cout << "[INFO] Checking for pending rooms" << std::endl;
    for (auto& [sock, player] : players) {
        //Check if the room is not full
        if (!player.room_full) {
            roomID = player.roomID;

            // Mark the room as full for both players
            player.setRoomFull(true);
            this->setRoomFull(true);

            // save the other player object
            otherPlayer = &player;
            break; // Exit the loop after assigning a room
        }
    }

    // If room is full and other player is valid
    if (room_full && otherPlayer != nullptr) {
        std::cout << "[INFO] Found a room." << std::endl;

        // Trigger coin flip only when 2 players are connected
        std::cout << "[INFO] Both players connected. Performing coinflip." << std::endl;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 1);
        bool player1Starts = dis(gen) == 1;

        std::string coinflipMessage = player1Starts ? "coinflip:player" + std::to_string(this->getID()) : "coinflip:player" + std::to_string(otherPlayer->getID());

        // update Player objects
        if (player1Starts)
            this->setStartsFirst(true);
        else
            otherPlayer->setStartsFirst(true);

        // send coinflip msg to this player and other player
        send_msg(*socket, coinflipMessage, error);
        send_msg(*(otherPlayer->getSocket()), coinflipMessage, error);

        // popularte power deck & inform players of deck size
        populatePowerDeck(roomID);
        size_t deckSize = powerDecks[roomID].size();
        send_msg(*socket, "power_deck:" + std::to_string(deckSize), error);
        send_msg(*(otherPlayer->getSocket()), "power_deck:" + std::to_string(deckSize), error);

        // distribute cards & send card hand to clients
        distributeCards(*this, otherPlayer);

        std::string currPlayerCards = concatWithSpace(card_hand), otherPlayerCards = concatWithSpace(otherPlayer->card_hand);
        send_msg(*socket, "card_hand:" + currPlayerCards, error);
        send_msg(*(otherPlayer->getSocket()), "card_hand:" + otherPlayerCards, error);

        // send opponent card hand for debugging/rendering
        send_msg(*socket, "opponent_cards:" + concatWithSpace(otherPlayer->getCardHand()) + " " + concatWithSpace(otherPlayer->getPowerHand()), error);
        send_msg(*(otherPlayer->getSocket()), "opponent_cards:" + concatWithSpace(this->getCardHand()) + " " + concatWithSpace(this->getPowerHand()), error);

        // Notify both players that the game can start
        send_msg(*socket, "game_start", error);
        send_msg(*otherPlayer->getSocket(), "game_start", error);
    }
    else {
        // If no available room was found, assign a new room
        if (!roomIDs.empty()) {
            auto maxRoomID = std::max_element(roomIDs.begin(), roomIDs.end());
            roomID = (*maxRoomID) + 1;
        }
        else {
            roomID = 1;
        }
        roomIDs.push_back(roomID);
        std::cout << "[INFO] Assigned new room ID: " << roomID << std::endl;
    }
}

