/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        SocketUtils.cpp
 * Author(s):   Edthan Yeo (10%) (handle coinflip message)
                Xiong Yang (20%) (handle discard pile message, skip turn, start/end turn,game_start)
                Yar Xuan (20%) (sync clients' card decks, sync power card deck size, give card)
                Low Jianlin (50%) (server connection, read and parse messages from server socket)
 *
 * Description: Client side utility functions to connect/disconnect with server and 
                handle messages from server socket.
 ******************************************************************************/

#define _WIN32_WINNT 0x0601 // Target Windows 7 or later

#include <asio.hpp>
#include <string>
#include <iostream>
#include "Gameboard.h"
#include "SocketUtils.h"
#include "Cardlist.h"
#include "GameStateManager.h"
#include "GameUtils.h"

bool newGameStarting = false;

using namespace asio;
using ip::tcp;
using std::string;
int currentPlayerID = 0; // Initialize to 0 or an invalid ID

string serverIP = "127.0.0.1";
int serverPort = 1234;

asio::io_context ioContext;
tcp::socket gameSocket(ioContext);
std::thread networkThread;
std::mutex networkMutex;
std::queue<std::string> serverMessages;

void send_msg(tcp::socket& socket, const string& message, std::error_code error) {
    const string msg = message + "\n";
    asio::write(socket, asio::buffer(msg), error);

    if (!error) {
        std::cout << "Client sent: " << message << std::endl;
    }
    else {
        std::cout << "Client send (" << message << ") failed: " << error.message() << std::endl;
    }
}

void async_read_message(tcp::socket& socket) {
    static asio::streambuf receive_buffer;

    asio::async_read_until(socket, receive_buffer, '\n',
        [&socket](std::error_code ec, std::size_t length) {
            (void)length; // Mark length as intentionally unused
            if (!ec) {
                std::istream stream(&receive_buffer);
                std::string message;
                std::getline(stream, message);

                if (!message.empty()) {
                    std::lock_guard<std::mutex> lock(networkMutex);
                    serverMessages.push(message);
                    std::cout << "[DEBUG] Message queued from server: " << message << std::endl;
                }
                else {
                    std::cerr << "[ERROR] Received empty message from server!" << std::endl;
                }
                async_read_message(socket);
            }
            else {
                std::cerr << "[ERROR] Failed to read message: " << ec.message() << std::endl;
            }
        });
}

void ConnectToServer() {
    try {
        // Reset IO context and socket for reconnection
        ioContext.restart();  // Reset the io_context so it can be reused
        gameSocket = asio::ip::tcp::socket(ioContext);  // Recreate the socket

        tcp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve(serverIP, std::to_string(serverPort));

        std::error_code ec;
        asio::connect(gameSocket, endpoints, ec);

        if (ec) {
            std::cerr << "Failed to connect to server: " << ec.message() << std::endl;
            return;
        }

        std::cout << "Connected to server!" << std::endl;

        // start reading messages from server asynchronously
        async_read_message(gameSocket);

        networkThread = std::thread([]() {
            try {
                ioContext.run();
            }
            catch (const std::exception& e) {
                std::cout << "Network error" << e.what() << std::endl;
            }
        });
    }
    catch (const std::exception& e) {
        std::cerr << "Exception while connecting: " << e.what() << std::endl;
    }
}

void ProcessServerMessages() {
    std::lock_guard<std::mutex> lock(networkMutex);

    while (!serverMessages.empty()) {
        std::string message = serverMessages.front();
        serverMessages.pop();

        std::cout << "[DEBUG] Processing server message: " << message << std::endl;

        if (currentPlayer.turn_phase == "game_over") {
            // Allow setup messages even if game was over
            if (!(message.rfind("game_start", 0) == 0 ||
                message.rfind("card_hand", 0) == 0 ||
                message.rfind("opponent_cards", 0) == 0 ||
                message.rfind("power_deck", 0) == 0 ||
                message.rfind("power_cards", 0) == 0 ||
                message.rfind("turn_start", 0) == 0 ||
                message.rfind("player_turn", 0) == 0 ||
                message.rfind("coinflip", 0) == 0 ||
                message.rfind("PlayerID", 0) == 0)) {

                std::cout << "[DEBUG] Ignoring message: '" << message << "' because game is already over." << std::endl;
                continue;
            }
        }

        // Parse and set the player ID
        if (message.find("PlayerID:") != std::string::npos) {
            int id = std::stoi(message.substr(message.find(":") + 1));
            currentPlayer.setID(id);
            std::cout << "[DEBUG] Player ID set to: " << currentPlayer.getID() << std::endl;
            continue;
        }

        // Handle coin flip messages
        if (message.find("coinflip:player") != std::string::npos) {
            int id = std::stoi(message.substr(message.find(":") + 7));
            currentPlayer.is_player_turn = (currentPlayer.getID() == id);
            errorMessage = currentPlayer.is_player_turn ? "" : "Opponent's Turn";
            std::cout << "[DEBUG] Coin flip result: "
                << (currentPlayer.is_player_turn ? "Your Turn" : "Opponent's Turn")
                << std::endl;
            continue;
        }

        // Handle power card deck size update
        if (message.find("power_deck:") != std::string::npos) {
            int size = std::stoi(message.substr(message.find(":") + 1));
            powerDeckSize = size;
            if (size == 0) {
                std::cout << "[INFO]: Power cards deck has run out" << std::endl;
            }
            continue;
        }

        // Handle discard pile update
        if (message.find("discard_pile:") != std::string::npos) {
            std::cout << "[INFO]: Update discard pile" << std::endl;
            std::string cardsStr = message.substr(message.find(":") + 1);
            updateDiscardPile(cardsStr);
            continue;
        }

        // Handle card_deck messages
        if (message.find("card_hand") != std::string::npos) {
            std::string cardsStr = message.substr(message.find(":") + 1);
            currentPlayer.setCardHand(cardsStr);
            std::cout << "[DEBUG] Player card deck set to: " << cardsStr << std::endl;
            continue;
        }

        // Handle opponent_cards message
        if (message.find("opponent_cards:") != std::string::npos) {
            std::string cardsStr = message.substr(message.find(":") + 1);
            otherPlayer.setCardHand(cardsStr);
            std::cout << "[DEBUG] Opponent card deck set to: " << cardsStr << std::endl;
            continue;
        }

        // Handle give_card message (opponent has played give card)
        if (message == "give_card") {
            std::cout << "[INFO]: Opponent played give card" << std::endl;
            infoMessage = "Opponent is choosing a card to give to you...";
            continue;
        }

        // Handle start game
        if (message == "game_start") {
            std::cout << "[INFO]: Game Start" << std::endl;

            newGameStarting = true; // ✅ Mark that we are resetting state
            waitingRoom = false;

            // Clear everything
            turnPlayed = false;
            revealHand = false;
            givingCard = false;
            infoMessage.clear();
            errorMessage.clear();
            currentPlayer.is_winner = false;
            otherPlayer.is_winner = false;
            currentPlayer.turn_phase = "";
            otherPlayer.turn_phase = "";

            if (currentPlayer.is_player_turn) {
                currentPlayer.turn_phase = "draw_card";
                AEGetTime(&turnStartTime);
                std::cout << "[DEBUG] Your turn starts now." << std::endl;
            }

            continue;
        }

        // Handle start of turn
        if (message == "start_turn") {
            if (!currentPlayer.is_player_turn) {
                currentPlayer.is_player_turn = true;
                currentPlayer.turn_phase = "draw_card";
                turnPlayed = false; // Reset turnPlayed for the new turn
                selectedCards.clear();
                errorMessage = "";
                AEGetTime(&turnStartTime);

                std::cout << "[DEBUG] Received 'start_turn'. It is now your turn." << std::endl;
                std::cout << "[DEBUG] Current player turn phase: draw_card" << std::endl;
            }
            else {
                std::cout << "[WARNING] Received 'start_turn' but it's already the player's turn." << std::endl;
            }
            continue;
        }

        // Handle end of turn (for possible future use)
        if (message == "end_turn") {
            currentPlayer.is_player_turn = false;
            currentPlayer.turn_phase = "";
            errorMessage = "Opponent's Turn";
            std::cout << "[DEBUG] Received 'end_turn'. Waiting for opponent." << std::endl;
            continue;
        }

        // Handle skip turn
        if (message == "skip_turn") {
            currentPlayer.is_player_turn = true;
            currentPlayer.turn_phase = "draw_card";
            turnPlayed = false;
            selectedCards.clear();
            AEGetTime(&turnStartTime);

            errorMessage = "Opponent skipped their turn";
            std::cout << "[DEBUG] Received 'skip_turn'. It is now your turn." << std::endl;
            std::cout << "[DEBUG] Current player turn phase: draw_card" << std::endl;
            continue;
        }

        // Handle win/lose message for both players
        if (message == "win" || message == "lose") {
            currentPlayer.turn_phase = "game_over";
            currentPlayer.is_winner = (message == "win");

            otherPlayer.turn_phase = "game_over";
            otherPlayer.is_winner = (message == "lose");  // If one wins, the other loses

            // Debug log
            std::cout << "[DEBUG] Player received '" << message << "' message." << std::endl;
            std::cout << "[DEBUG] is_winner flag set to: " << currentPlayer.is_winner << std::endl;

            return;
        }


        // Handle game over message
        if (message == "game_over") {
            // Always set the turn phase to "game_over"
            currentPlayer.turn_phase = "game_over";
            continue;
        }

        // Unknown message handling
        std::cout << "[WARNING] Unhandled server message: " << message << std::endl;
    }
}

void DisconnectFromServer() {
    try {
        gameSocket.close();
        if (networkThread.joinable()) {
            networkThread.join();
        }
        std::cout << "Disconnected from server gracefully." << std::endl;
    }
    catch (...) {
        std::cerr << "Error disconnecting from server." << std::endl;
    }
}