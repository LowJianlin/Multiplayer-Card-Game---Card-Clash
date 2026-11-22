# Card Clash

A 2-player online card game built in C++ by a team of four, as a first-year project at DigiPen Institute of Technology Singapore (CSD1451, Spring 2025).

## About

Card Clash is a turn-based card game where two players compete to be the first to empty their hand. Each turn, you draw a card from your opponent's hand or from the power card deck, then play a power card or a set of four matching number cards.

The game runs on a **client–server model** over TCP. A dedicated server pairs players into game rooms, deals the cards, keeps track of each player's hand, and keeps both clients in sync as the game goes on.

## My Contributions

I built the **networking layer** that the game runs on, on both the server and the client:

**Server**
- **TCP server:** wrote the server using [ASIO](https://think-async.com/Asio/). It accepts incoming players, gives each one a player ID, and handles each client on its own thread.
- **Message protocol:** designed a simple text-based protocol, where each message is one line in the form `action:details` (for example `picked_card:12` or `end_turn`). This made messages easy to read and debug from the console.
- **Action handler system:** built a dispatcher that maps each action name to a handler function. Adding a new game action, such as a new power card, only meant registering a new handler. My teammates built the game logic (turns, power cards, win conditions) on top of this system.

**Client**
- **Connecting to the server:** wrote the client's connect and disconnect logic, including resetting the connection so players can start a new game without restarting the client.
- **Non-blocking message handling:** the client reads server messages asynchronously on a separate network thread and puts them in a mutex-protected queue. The game loop empties the queue once per frame, so network delays never freeze the game and game state is only changed from the main thread.

**Other**
- Built the main menu screen.
- Helped debug connection and sync issues across the team.

## How It Works

1. A client connects to the server and receives a player ID.
2. The server places the player in a room. When a second player joins, the room is full and the game begins.
3. The server flips a coin to decide who goes first, deals both hands, and sets up the power card deck for that room.
4. During the game, each client sends its actions to the server. The server updates the game state and sends the changes to both players, so the two clients always show the same game.
5. On each client, incoming messages are queued by a background network thread and applied by the game loop every frame.

## Tech Stack

- **Language:** C++17
- **Engine:** DigiPen Alpha Engine
- **Networking:** ASIO (TCP sockets, synchronous on the server, asynchronous on the client)
- **Data:** RapidJSON (card definitions are loaded from a JSON file)
- **Tools:** Visual Studio 2022, Git

## How to Play

**Goal:** be the first player to empty your hand.

**Phase 1: Draw**
- Left-click a card on your opponent's side to take it, **or**
- Left-click the power card deck to draw a power card

**Phase 2: Play**
- Left-click a power card, or four matching number cards, to select them
- Right-click to play your selection
- Press **Space** to skip if you have nothing to play

During your turn you can also drag and drop cards to rearrange your hand, or use the **Shuffle** and **Sort** buttons on the right.

## Running Locally

**Requirements:** Windows, Visual Studio 2022

Testing on one computer needs one server and two clients.

1. Clone this repository and open the `.sln` file in the `SOURCE` folder in Visual Studio.
2. Right-click the solution, choose **Configure Startup Projects**, then select **Multiple startup projects**.
3. Set both **Server** and **CardClash** to *Start*, and order them so **Server** runs first.
4. Press **F5** (Start Debugging) to launch the server and the first client.
5. Open a second client by running `CardClash.exe` from `bin/Debug-x64` (this folder is created when you build the project).

> The server listens on port `1234`. Clients connect to `127.0.0.1` (localhost) by default. To play across two computers, change the IP address in `CardClash/Source/SocketUtils.cpp`.

## What I'd Improve

- **Server thread safety:** each client connection runs on its own thread, but they all share the same player data without locking. I'd add a mutex, as the client already does with its message queue, or move the server to ASIO's asynchronous model with a single event loop.
- **Error handling:** send errors are only logged right now. I'd pass them back to the caller so the server can respond properly when a client drops mid-game.
- **Configuration:** the server IP and port are hardcoded. I'd move them into a config file.

## Team: 9~bit

Low Jianlin, Yar Xuan, Edthan Yeo, Xiong Yang

## Acknowledgements

Instructors: Cheng Ding Xiang, Gerald Wong, Soroor Faradounbeh

Created at DigiPen Institute of Technology Singapore.
All content © 2025 DigiPen Institute of Technology Singapore. All rights reserved.
