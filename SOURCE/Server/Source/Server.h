/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Server.h
 * Author(s):   Edthan Yeo (10%) (coinflip)
                Xiong Yang (20%) (win logic,end turn,skip turn,swap power card)
                Yar Xuan (20%) (sync clients' card decks, draw power card, swap hands, give card)
                Low Jianlin (50%) (server-client connection, setup action handler)
 *
 * Description: Header file for Server.cpp.
 ******************************************************************************/

#ifndef SERVER_H // Include guard to prevent multiple inclusions
#define SERVER_H

#include <asio.hpp>
#include <iostream>
#include <vector>
#include <functional>
#include <memory> // For std::shared_ptr

using namespace asio;
using ip::tcp;
using SocketPtr = std::shared_ptr<tcp::socket>;

extern void send_msg(tcp::socket& socket, const std::string& message, std::error_code error);

static void InitialiseActionHandlers();
static void HandleIncomingMessages(const std::string& message, SocketPtr senderSocket);
static void handle_client(SocketPtr socket);
static void StartServer(int port);

#endif