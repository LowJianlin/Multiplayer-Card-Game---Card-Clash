/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Player.h
 * Author(s):   Yar Xuan (90%) (Player class functions)
 *              Edthan Yeo (10%) (coinflip in assignRoomID)
 *
 * Description: Player class definition.
 ******************************************************************************/

#ifndef PLAYER_H // Include guard to prevent multiple inclusions
#define PLAYER_H

#include <asio.hpp>
#include <vector>
#include <map>
#include <memory> // For std::shared_ptr

using SocketPtr = std::shared_ptr<asio::ip::tcp::socket>;
extern std::vector<int> roomIDs;

class Player {
private:
    static int nextID;  // Static variable to track the next available ID
    int playerID;       // Unique ID for this player
    int roomID;
    std::vector<std::string> card_hand; // list of card IDs
    std::vector<std::string> power_cards;

    bool is_player_turn = false, room_full = false, starts_first = false;

public:
    Player();
    int getID() const;
    int getRoomID() const;
    bool getRoomFull() const;
    std::vector<std::string> getCardHand() const;
    void setTurn(bool turn);
    void setRoomFull(bool val);
    bool getTurn() const;
    bool getStartsFirst() const;
    void setStartsFirst(bool val);
    SocketPtr getSocket() const;
    void addCard(int id);
    void removeCard(int id);
    void setCardHandStr(std::string newHand);
    void setCardHand(std::vector<std::string>);
    Player& getOpponent();
    void assignRoomID(SocketPtr socket);

    std::vector<std::string> getPowerHand() const;
};

extern std::map<SocketPtr, Player> players;

#endif