/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        SocketUtils.h
 * Author(s):   Edthan Yeo (10%) (handle coinflip message)
                Xiong Yang (20%) (handle discard pile message, skip turn, start/end turn)
                Yar Xuan (20%) (sync clients' card decks, sync power card deck size, give card)
                Low Jianlin (50%) (server connection, read and parse messages from server socket)
 *
 * Description: Header file for SocketUtils.cpp.
 ******************************************************************************/

#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <asio.hpp>
#include <string>
#include <queue>

extern std::string serverIP;
extern int serverPort;

extern asio::io_context ioContext;
extern asio::ip::tcp::socket gameSocket;
extern std::thread networkThread;
extern std::mutex networkMutex;
extern std::queue<std::string> serverMessages;

extern void send_msg(asio::ip::tcp::socket & socket, const std::string& message, std::error_code error);
void async_read_message(asio::ip::tcp::socket& socket);
extern void ConnectToServer();
extern void ProcessServerMessages();
extern void DisconnectFromServer();

#endif // SOCKET_UTILS_H