/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Server.cpp
 * Author(s):   Edthan Yeo (10%) (coinflip)
                Xiong Yang (20%) (win logic,end turn,skip turn,swap power card)
                Yar Xuan (20%) (sync clients' card decks, draw power card, swap hands, give card)
                Low Jianlin (50%) (server-client connection, setup action handler)
 *
 * Description: Server functions, including action handler to handle messages from clients.
 ******************************************************************************/

#define _WIN32_WINNT 0x0601 // Target Windows 7 or later

#include <asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <sstream>
#include <memory> // For std::shared_ptr
#include <algorithm>
#include <unordered_map>

#include "Cards.h"
#include "Server.h"
#include "Player.h"

int serverPort = 1234;

using namespace asio;
using ip::tcp;
using SocketPtr = std::shared_ptr<tcp::socket>;

tcp::acceptor* serverAcceptor = nullptr;

// Handler Function action
using ActionHandler = std::function<void(const std::string&, std::shared_ptr<tcp::socket>)>;
std::unordered_map<std::string, ActionHandler> actionHandlers;

void send_msg(tcp::socket& socket, const std::string& message, std::error_code error) {
    const std::string msg = message + "\n";
    asio::write(socket, asio::buffer(msg), error);

    if (!error) {
        std::cout << "Server sent: " << message << std::endl;
    }
    else {
        std::cout << "Server send (" << message << ") failed: " << error.message() << std::endl;
    }
}

// Initialise action handlers
static void InitialiseActionHandlers() {
    actionHandlers["card_hand"] = [](const std::string& message, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                std::string cardsStr = message.substr(message.find(":") + 1);
                player.setCardHandStr(cardsStr);

                Player& opponent = player.getOpponent();

                // Update the opponent with the player's new card hand
                send_msg(*opponent.getSocket(), "opponent_cards:" + cardsStr, error);
                std::cout << "[DEBUG] Notifying Player ID: " << opponent.getID() << " of Player " << player.getID() << "'s new hand." << std::endl;

                // Case 1: Either player has exactly one card left
                if (player.getCardHand().size() == 1 || opponent.getCardHand().size() == 1) {
                    std::string lastCard = player.getCardHand()[0];

                    if (lastCard == "0") {
                        // Player loses if they have the Joker
                        send_msg(*player.getSocket(), "lose", error);
                        send_msg(*opponent.getSocket(), "win", error);
                        std::cout << "[DEBUG] Player ID: " << player.getID() << " lost the game by being left with the Joker!" << std::endl;
                    }
                    else {
                        // Player wins if they have a non-Joker card
                        send_msg(*player.getSocket(), "win", error);
                        send_msg(*opponent.getSocket(), "lose", error);
                        std::cout << "[DEBUG] Player ID: " << player.getID() << " won the game by being left with one non-Joker card!" << std::endl;
                    }

                    // Send game over messages
                    send_msg(*player.getSocket(), "game_over", error);
                    send_msg(*opponent.getSocket(), "game_over", error);
                    return;
                }

                // Case 2: If a player has exactly 3 cards and one is the Joker, they lose immediately
                if (player.getCardHand().size() == 3) {
                    bool hasJoker = false;
                    for (const auto& card : player.getCardHand()) {
                        if (card == "0") {
                            hasJoker = true;
                            break;
                        }
                    }

                    if (hasJoker) {
                        // Player loses with 3 cards including a Joker
                        send_msg(*player.getSocket(), "lose", error);
                        send_msg(*opponent.getSocket(), "win", error);
                        std::cout << "[DEBUG] Player ID: " << player.getID() << " lost with 3 cards, including a Joker!" << std::endl;

                        // Send game over messages
                        send_msg(*player.getSocket(), "game_over", error);
                        send_msg(*opponent.getSocket(), "game_over", error);
                        return;
                    }
                }

                break;
            }
        }
    };

    actionHandlers["picked_card"] = [](const std::string& message, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                // add card to player deck
                int cardId = std::stoi(message.substr(message.find(":") + 1));
                player.addCard(cardId);

                // remove card from opponent deck
                Player& opponent = player.getOpponent();
                opponent.removeCard(cardId);

                std::cout << "[DEBUG] Player card hand size: " << player.getCardHand().size() << std::endl;
                std::cout << "[DEBUG] Player card hand contents: ";
                for (const auto& card : player.getCardHand()) {
                    std::cout << card << " ";
                }
                std::cout << std::endl;

                // Check if the opponent is left with only 1 card
                if (opponent.getCardHand().size() == 1) {
                    if (opponent.getCardHand()[0] == "0") {
                        // Opponent loses if left with the Joker
                        send_msg(*opponent.getSocket(), "lose", error);
                        send_msg(*sock, "win", error);
                    }
                    else {
                        // Opponent wins if left with a non-Joker card
                        send_msg(*opponent.getSocket(), "win", error);
                        send_msg(*sock, "lose", error);
                    }
                    // Send game over state to both players
                    send_msg(*sock, "game_over", error);
                    send_msg(*opponent.getSocket(), "game_over", error);
                    return;
                }

                // update opponent side
                send_msg(*opponent.getSocket(), "card_hand:" + concatWithSpace(opponent.getCardHand()), error);
                send_msg(*opponent.getSocket(), "opponent_cards:" + concatWithSpace(player.getCardHand()), error);

                // update current player side
                send_msg(*sock, "opponent_cards:" + concatWithSpace(opponent.getCardHand()), error);
                break;
            }
        }
        };

    actionHandlers["draw_power_card"] = [](const std::string& /*message*/, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                int roomID = player.getRoomID();
                int powerCardID = drawPowerCard(roomID);
                player.addCard(powerCardID);

                // update current player side
                send_msg(*sock, "card_hand:" + concatWithSpace(player.getCardHand()), error);
                std::cout << "[DEBUG] Gave Player ID: " << player.getID() << "Power Card ID: " << powerCardID << std::endl;
                
                // update both clients of power deck size
                size_t deckSize = powerDecks[roomID].size();
                std::cout << deckSize << std::endl;
                send_msg(*sock, "power_deck:" + std::to_string(deckSize), error);
                send_msg(*player.getOpponent().getSocket(), "power_deck:" + std::to_string(deckSize), error);

                // NEW: Update opponent's view of your hand
                send_msg(*player.getOpponent().getSocket(),
                    "opponent_cards:" + concatWithSpace(player.getCardHand()),
                    error);
            }
        }
    };

    actionHandlers["played_numbers"] = [](const std::string& message, SocketPtr senderSocket) {
        std::cout << "testing" << std::endl;
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                std::string cardsStr = message.substr(message.find(":") + 1);
                // update opponent discard pile
                send_msg(*player.getOpponent().getSocket(), "discard_pile:" + cardsStr, error);

                break;
            }
        }
    };
    
    actionHandlers["played_power"] = [](const std::string& message, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                // remove played power card
                int cardId = std::stoi(message.substr(message.find(":") + 1));
                player.removeCard(cardId);

                // update this player's card hand
                send_msg(*sock, "card_hand:" + concatWithSpace(player.getCardHand()), error);

                // update opponent to wait for player to finish power card effect
                Player& opponent = player.getOpponent();
                if (cardId >= POWER_TYPES::GIVE_CARD && cardId < POWER_TYPES::SWAP_POWERS) {
                    send_msg(*opponent.getSocket(), "give_card", error);
                    std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " that their opponent played give_card." << std::endl;
                }
                
                // update opponent discard pile
                send_msg(*opponent.getSocket(), "discard_pile:" + std::to_string(cardId), error);

                break;
            }
        }
    };

    actionHandlers["swap_hands"] = [](const std::string& message, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                // remove played power card
                int cardId = std::stoi(message.substr(message.find(":") + 1));
                player.removeCard(cardId);

                // swap their hands excluding power cards
                Player& opponent = player.getOpponent();
                std::vector<std::string> playerNewHand{}, oppNewHand{};

                for (std::string id : player.getCardHand()) {
                    Card* card = findCardById(std::stoi(id));
                    if (card->type == "power") {
                        playerNewHand.push_back(id);
                    }
                    else if (card->type == "number") {
                        oppNewHand.push_back(id);
                    }
                }

                for (std::string id : opponent.getCardHand()) {
                    Card* card = findCardById(std::stoi(id));
                    if (card->type == "power") {
                        oppNewHand.push_back(id);
                    }
                    else if (card->type == "number") {
                        playerNewHand.push_back(id);
                    }
                }

                player.setCardHand(playerNewHand);
                opponent.setCardHand(oppNewHand);

                // update opponent side
                send_msg(*opponent.getSocket(), "swap_hands", error);
                std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " that their opponent played swap_hands." << std::endl;
                send_msg(*opponent.getSocket(), "card_hand:" + concatWithSpace(opponent.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " of their new hand." << std::endl;
                send_msg(*opponent.getSocket(), "opponent_cards:" + concatWithSpace(player.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " of their opponent's new hand." << std::endl;
                // update opponent discard pile
                send_msg(*opponent.getSocket(), "discard_pile:" + std::to_string(cardId), error);

                // update current player side
                send_msg(*sock, "opponent_cards:" + concatWithSpace(opponent.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << player.getID() << " of Player " << opponent.getID() << "'s new hand." << std::endl;
                send_msg(*sock, "card_hand:" + concatWithSpace(player.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " of their new hand." << std::endl;
                break;
            }
        }
    };

    actionHandlers["give_card"] = [](const std::string& message, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                // remove card to be given
                int cardId = std::stoi(message.substr(message.find(":") + 1));
                player.removeCard(cardId);

                // add card to opponent hand
                Player& opponent = player.getOpponent();
                opponent.addCard(cardId);

                // update opponent side
                send_msg(*opponent.getSocket(), "card_hand:" + concatWithSpace(opponent.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " of their new hand." << std::endl;
                send_msg(*opponent.getSocket(), "opponent_cards:" + concatWithSpace(player.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " of their opponent's new hand." << std::endl;

                // update current player side
                send_msg(*sock, "opponent_cards:" + concatWithSpace(opponent.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << player.getID() << " of Player " << opponent.getID() << "'s new hand." << std::endl;
                send_msg(*sock, "card_hand:" + concatWithSpace(player.getCardHand()), error);
                std::cout << "[DEBUG] Notified Player ID: " << opponent.getID() << " of their new hand." << std::endl;

                break;
            }
        }
    };

    actionHandlers["swap_power"] = [](const std::string& message, SocketPtr senderSocket) {
        std::error_code error;

        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                Player& opponent = player.getOpponent();

                std::vector<std::string> playerNumbers;
                std::vector<std::string> playerPowers;

                // Separate current player's cards into number and power cards
                for (const std::string& id : player.getCardHand()) {
                    Card* card = findCardById(std::stoi(id));
                    if (card && card->type == "number") {
                        playerNumbers.push_back(id);
                    }
                    else if (card && card->type == "power") {
                        // Remove the currently used swap card (comes from message)
                        int usedCardId = std::stoi(message.substr(message.find(":") + 1));
                        if (std::stoi(id) != usedCardId) {
                            playerPowers.push_back(id);
                        }
                    }
                }

                std::vector<std::string> opponentNumbers;
                std::vector<std::string> opponentPowers;

                // Separate opponent's cards into number and power cards
                for (const std::string& id : opponent.getCardHand()) {
                    Card* card = findCardById(std::stoi(id));
                    if (card && card->type == "number") {
                        opponentNumbers.push_back(id);
                    }
                    else if (card && card->type == "power") {
                        opponentPowers.push_back(id);
                    }
                }

                // Swap power cards between players
                std::vector<std::string> newPlayerHand = playerNumbers;
                newPlayerHand.insert(newPlayerHand.end(), opponentPowers.begin(), opponentPowers.end());

                std::vector<std::string> newOpponentHand = opponentNumbers;
                newOpponentHand.insert(newOpponentHand.end(), playerPowers.begin(), playerPowers.end());

                // Set the new hands
                player.setCardHand(newPlayerHand);
                opponent.setCardHand(newOpponentHand);

                // Send updated hands
                send_msg(*sock, "card_hand:" + concatWithSpace(newPlayerHand), error);
                send_msg(*opponent.getSocket(), "card_hand:" + concatWithSpace(newOpponentHand), error);

                // Inform opponent of the discarded swap card
                int usedCardId = std::stoi(message.substr(message.find(":") + 1));
                send_msg(*opponent.getSocket(), "discard_pile:" + std::to_string(usedCardId), error);

                break;
            }
        }
    };

    actionHandlers["end_turn"] = [](const std::string& /*message*/, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                Player& opponent = player.getOpponent();
                 // BEFORE starting the opponent's turn, check if they win
                if (player.getCardHand().size() == 2) {
                    std::cout << "[DEBUG] Opponent (Player ID: " << player.getID()
                        << ") wins with 2 cards on their turn!" << std::endl;

                    send_msg(*player.getSocket(), "win", error);
                    send_msg(*opponent.getSocket(), "lose", error);
                    send_msg(*player.getSocket(), "game_over", error);
                    send_msg(*opponent.getSocket(), "game_over", error);
                    return;
                }
                send_msg(*player.getOpponent().getSocket(), "start_turn", error);
                std::cout << "[DEBUG] Notifying Player ID: " << player.getID() << " to start their turn." << std::endl;
                break; // Ensure only one "start_turn" is sent
            }
        }
     };

    actionHandlers["skip_turn"] = [](const std::string& /*message*/, SocketPtr senderSocket) {
        std::error_code error;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                Player& opponent = player.getOpponent();

                //Check if opponent wins before continuing
                if (opponent.getCardHand().size() == 2) {
                    std::cout << "[DEBUG] Opponent (Player ID: " << opponent.getID()
                        << ") wins with 2 cards on their turn!" << std::endl;

                    send_msg(*opponent.getSocket(), "win", error);
                    send_msg(*sock, "lose", error);
                    send_msg(*opponent.getSocket(), "game_over", error);
                    send_msg(*sock, "game_over", error);
                    return;  //Stop execution to prevent continuing the game
                }
                send_msg(*player.getOpponent().getSocket(), "skip_turn", error);
                std::cout << "[DEBUG] Sent 'skip_turn' to Player ID: " << player.getID() << std::endl;
                break;
            }
        }
     };

    actionHandlers["coinflip"] = [](const std::string& message, SocketPtr senderSocket) {
        std::error_code error;
        std::cout << "[DEBUG] Server received coinflip message: " << message << std::endl;
        for (auto& [sock, player] : players) {
            if (sock == senderSocket) {
                send_msg(*sock, message, error);
                send_msg(*player.getOpponent().getSocket(), message, error);

                if (!error) {
                    std::cout << "[DEBUG] Successfully forwarded coinflip message to client: " << message << std::endl;
                }
                else {
                    std::cerr << "[ERROR] Failed to send message: " << error.message() << std::endl;
                }
                break;
            }
        }
    };
}

// Parse and handle actions dynamically
void HandleIncomingMessages(const std::string& message, SocketPtr senderSocket) {
    std::string action, details;

    // Parse action and details with ':' as a separator
    size_t colonPos = message.find(':');
    if (colonPos != std::string::npos) {
        action = message.substr(0, colonPos); // Action is before the colon
        details = message.substr(colonPos + 1); // Details are after the colon
    }
    else {
        action = message; // If no colon, treat the whole message as the action
    }

    std::cout << "[DEBUG] Parsed action: " << action << ", details: " << details << std::endl;

    if (actionHandlers.find(action) != actionHandlers.end()) {
        actionHandlers[action](message, senderSocket);
    }
    else {
        std::cout << "Unknown action: " << action << std::endl;
    }
}

static void handle_client(SocketPtr socket) {
    try {
        asio::streambuf receive_buffer;

        while (true) {
            // Read until newline
            asio::read_until(*socket, receive_buffer, '\n');

            // Extract the message from the buffer
            std::istream stream(&receive_buffer);
            std::string message;
            std::getline(stream, message);

            std::cout << "Received: " << message << std::endl;

            // Handle the incoming message
            HandleIncomingMessages(message, socket);
        }
    }
    catch (const std::exception& e) {
        std::cout << "Client disconnected: " << e.what() << std::endl;
        players.erase(socket); // Remove the disconnected player
    }
    catch (...) {
        std::cout << "Client disconnected due to an unknown error." << std::endl;
        players.erase(socket); // Remove the disconnected player
    }
}

static void StartServer(int port) {
    asio::io_context ioContext;
    serverAcceptor = new tcp::acceptor(ioContext, tcp::endpoint(tcp::v4(), static_cast<asio::ip::port_type>(port)));

    std::cout << "Server started on port " << port << std::endl;

    InitialiseActionHandlers();
    LoadCardList();

    while (true) {
        auto socket = std::make_shared<tcp::socket>(ioContext);
        serverAcceptor->accept(*socket);

        Player newPlayer;
        std::error_code error;

        std::cout << "New player connected! ID: " << newPlayer.getID() << std::endl;
        send_msg(*socket, "PlayerID: " + std::to_string(newPlayer.getID()), error);
        
        newPlayer.assignRoomID(socket);
        players.insert({ socket, newPlayer });

        std::thread(handle_client, socket).detach();
    }
    ioContext.run();
}


int main() {
    StartServer(serverPort);
    // free resources
    FreeCardlist();
    roomIDs.clear();
	std::vector<int>().swap(roomIDs); // Force deallocation
}