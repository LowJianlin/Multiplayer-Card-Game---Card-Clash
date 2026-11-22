/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Gameboard.cpp
 * Author(s):   Edthan Yeo (25%) (draw power card hand, draw power card deck, game end screen, sort/shuffle cards)
 *              Xiong Yang (25%) (card selection, play sets of 4, play power card, turn information text, draw discard pile, skip turn)
 *              Yar Xuan (25%) (turn timer, card drag & drop to re-arrange, power card effects, waiting room, pick card from opponent)
 *              Low Jianlin (25%) (draw own and opponent card hands and power cards, main gameboard UI)
 *
 * Description: Gameboard game state.
 ******************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00

#include <winsock2.h>
#include <Windows.h>
#include "AEEngine.h"
#include "Cardlist.h"
#include <iostream>
#include <algorithm>
#include <random>
#include "SocketUtils.h"
#include "GameUtils.h"
#include "Gameboard.h"
#include "GameStateManager.h"

// constants
static f64 TURN_TIMER = 45.0f; // 45 seconds
static f64 REVEAL_HAND_TIMER = 5.0f; // 5 seconds

static AEMtx33 playareaScale = { 0 };
static AEMtx33 OpponentAreaTranslate = { 0 };
static AEMtx33 UserAreaTranslate = { 0 };
static AEVec2 p1card = { -750.0f, -380.0f };
static AEVec2 p2card = { -750.0f, 300.0f };
static s32 mousePosX = 0, mousePosY = 0;
static float powerCardSpacing = 100.0f, numberCardSpacing = 92.0f;

static AEVec2 sortButtonPos = { 650.0f, -180.0f }; // Adjust position as needed
static float sortButtonWidth = 100.0f, sortButtonHeight = 60.0f;

static AEVec2 shuffleButtonPos = { 650.0f, -120.0f }; // Below Sort button
static float shuffleButtonWidth = 100.0f, shuffleButtonHeight = 60.0f;

static int draggedCardIndex = -1;
bool givingCard = false, revealHand = false, isPowerDeckEmpty = false;
static f64 revealHandStartTime, currentTime;

// extern variables declared in GameBoard.h
std::string errorMessage = "", infoMessage = "";
bool turnPlayed = false;
bool waitingRoom = true;

void Gameboard_Load() {
	font = AEGfxCreateFont("./Assets/Fonts/ABeeZee-Regular.ttf", 25);
    LoadPowerCardTextures();
    LoadCardTextures();
    LoadGameEndTextures();
}

void Gameboard_Initialize() {
    waitingRoom = true;
    ConnectToServer();

    // Pointer to rectangle mesh
    AEGfxMeshStart();

    AEGfxTriAdd(
        0.0f, 0.85f, 0xFFFFFFFF, 0.0f, 0.0f,    // Top-left
        1.0f, 0.85f, 0xFFFFFFFF, 1.0f, 0.0f,    // Top-right
        0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f);    // Bottom-left

    AEGfxTriAdd(
        0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,        // Bottom-left
        1.0f, 0.85f, 0xFFFFFFFF, 1.0f, 0.0f,    // Top-right
        1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 1.0f);    // Bottom-right

    rectMesh = AEGfxMeshEnd();

    // Scale matrix for play area
    AEMtx33Scale(&playareaScale, 1600.0f, 250.0f);
    // Translation matrix for opponent play area
    AEMtx33Trans(&OpponentAreaTranslate, -(1600.0f / 2), 250.0f);
    // Translation matrix for user play area
    AEMtx33Trans(&UserAreaTranslate, -(1600.0f / 2), -450.0f);

    // Scale matrix for P1 card
    AEMtx33Scale(&cardScale, cardSize.x, cardSize.y);
}


void Gameboard_Update() {

    if (AEInputCheckCurr(AEVK_ESCAPE)) {
        next = GS_MAINMENU;
    }
    ProcessServerMessages();

    if (waitingRoom) {
        return;
    }

    size_t p1CardsNum = currentPlayer.card_hand.size();
    size_t oppCardsNum = otherPlayer.card_hand.size();
    AEInputGetCursorPosition(&mousePosX, &mousePosY);
    isPowerDeckEmpty = powerDeckSize == 0;
    
    // update currentTime to compare with turnStartTime, or revealHandStartTime
    AEGetTime(&currentTime);

    // Calculate self and opponent cards positions
    for (size_t i = 0; i < p1CardsNum; i++) {
        if (i != draggedCardIndex) {
            currentPlayer.card_hand[i].position = { p1card.x + (numberCardSpacing * i), p1card.y };
        }
    }

    for (size_t i = 0; i < oppCardsNum; i++) {
        otherPlayer.card_hand[i].position = { p2card.x + (numberCardSpacing * i), p2card.y };
    }

    // Prevent actions if not the player's turn
    if (!currentPlayer.is_player_turn) {
        errorMessage = "Opponent's Turn";
        return;
    }

    // Detect left-click to select/deselect cards
    if (AEInputCheckTriggered(AEVK_LBUTTON)) {
        for (size_t i = 0; i < p1CardsNum; i++) {
            f32 cardLeftX = 800.0f + currentPlayer.card_hand[i].position.x;
            f32 cardRightX = cardLeftX + 75.0f;
            f32 cardTopY = 450.0f - currentPlayer.card_hand[i].position.y - cardSize.y;
            f32 cardBottomY = 450.0f - currentPlayer.card_hand[i].position.y;

            // Check if the mouse is on the card
            if (mousePosX >= cardLeftX && mousePosX <= cardRightX &&
                mousePosY >= cardTopY && mousePosY <= cardBottomY) {

                auto it = std::find(selectedCards.begin(), selectedCards.end(), currentPlayer.card_hand[i]);

                if (givingCard) {
                    // if no card selected yet, add to selectedCards
                    if (selectedCards.size() == 0 && it == selectedCards.end()) {
                        selectedCards.push_back(currentPlayer.card_hand[i]);
                    }
                    // if card is already selected, de-select
                    else if (it != selectedCards.end()) {
                        selectedCards.erase(it);
                    }
                    // if player clicked on another card, set selected to this
                    else {
                        selectedCards[0] = currentPlayer.card_hand[i];
                    }
                }
                else {
                    // Add card to selection if not already selected and fewer than 4 selected
                    if (selectedCards.size() < 4 && it == selectedCards.end()) {
                        selectedCards.push_back(currentPlayer.card_hand[i]);
                    }
                    // Deselect card if already selected
                    else if (it != selectedCards.end()) {
                        selectedCards.erase(it);
                    }
                }
            }
        }
        // prevent player from selecting power card if choosing card to give 
        if (!givingCard) {
            for (size_t i = 0; i < currentPlayer.power_cards.size(); i++) {
                Card& powerCard = currentPlayer.power_cards[i];
                f32 leftX = 800.0f + powerCard.position.x;
                f32 rightX = leftX + cardSize.x;
                f32 bottomY = 450.0f - powerCard.position.y;
                f32 topY = bottomY - cardSize.y;

                if (mousePosX >= leftX && mousePosX <= rightX && mousePosY >= topY && mousePosY <= bottomY) {
                    auto it = std::find(selectedCards.begin(), selectedCards.end(), powerCard);

                    // Add card to selection if not already selected and fewer than 4 selected
                    if (selectedCards.size() < 4 && it == selectedCards.end()) {
                        selectedCards.push_back(powerCard);
                    }
                    // Deselect card if already selected
                    else if (it != selectedCards.end()) {
                        selectedCards.erase(it);
                    }
                }
            }
        }
    }

    // Handle card dragging
    if (AEInputCheckCurr(AEVK_LBUTTON)) {
        // Check if cursor is within window bounds
        if (mousePosX >= 0 && mousePosX <= 1600 && mousePosY >= 0 && mousePosY <= 900) {

            // Check if cursor is within a card
            f32 cardBottomY = 450.0f - p1card.y;
            f32 cardTopY = cardBottomY - 110.0f;
            if (mousePosY >= cardTopY && mousePosY <= cardBottomY) {
                for (size_t i = 0; i < p1CardsNum; i++) {
                    f32 cardLeftX = 800.0f + currentPlayer.card_hand[i].position.x;
                    f32 cardRightX = cardLeftX + 75.0f;
                    if (mousePosX >= cardLeftX && mousePosX <= cardRightX) {
                        draggedCardIndex = (draggedCardIndex == -1) ? static_cast<int>(i) : draggedCardIndex;
                    }
                }
            }

            // Update dragged card position
            if (draggedCardIndex != -1) {
                currentPlayer.card_hand[draggedCardIndex].position.x = static_cast<f32>(mousePosX - 800 - (cardSize.x / 2));
                currentPlayer.card_hand[draggedCardIndex].position.y = static_cast<f32>(450 - mousePosY - (cardSize.y / 2));
            }
        }
    }

    // Handle card drop
    if (AEInputCheckReleased(AEVK_LBUTTON) && draggedCardIndex != -1) {
        for (size_t i = 0; i < p1CardsNum; i++) {
            if (i != draggedCardIndex && AETestRectToRect(&currentPlayer.card_hand[draggedCardIndex].position, cardSize.x, cardSize.y,
                &currentPlayer.card_hand[i].position, cardSize.x, cardSize.y)) {
                std::swap(currentPlayer.card_hand[draggedCardIndex], currentPlayer.card_hand[i]);
                // update player hand on server side & opponent side
                std::error_code ec;
                send_msg(gameSocket, "card_hand:" + currentPlayer.getCardsStr(), ec);
            }
        }
        draggedCardIndex = -1;
    }

    // Handle player turn first phase (draw card from power deck or opponent's hand)
    if (currentPlayer.turn_phase == "draw_card") {
        // Check if mouse is hovering over power deck
        f32 deckLeftX = 800.0f + powerDeckPosition.x;
        f32 deckRightX = deckLeftX + cardSize.x;
        f32 deckBottomY = 450.0f - powerDeckPosition.y;
        f32 deckTopY = deckBottomY - cardSize.y;

        isPowerDeckHovered = !isPowerDeckEmpty && 
            (mousePosX >= deckLeftX && mousePosX <= deckRightX &&
            mousePosY >= deckTopY && mousePosY <= deckBottomY);

        // Handle clicking on power deck
        if (isPowerDeckHovered && AEInputCheckTriggered(AEVK_LBUTTON) && currentPlayer.power_cards.size() < 3) {
            // ask server for a power card
            std::error_code error;
            send_msg(gameSocket, "draw_power_card", error);
            // move on to next turn phase
            currentPlayer.turn_phase = "play_cards";
            isPowerDeckHovered = false;
            std::cout << "[DEBUG] Current player turn phase: play_cards" << std::endl;
        }
        // Handle clicking on opponent cards
        else if (AEInputCheckTriggered(AEVK_LBUTTON)) {
            for (size_t i = 0; i < oppCardsNum; i++) {
                f32 cardLeftX = 800.0f + otherPlayer.card_hand[i].position.x;
                f32 cardRightX = cardLeftX + 75.0f;
                f32 cardTopY = 450.0f - otherPlayer.card_hand[i].position.y - cardSize.y;
                f32 cardBottomY = 450.0f - otherPlayer.card_hand[i].position.y;

                // Check if the mouse is on the card
                if (mousePosX >= cardLeftX && mousePosX <= cardRightX &&
                    mousePosY >= cardTopY && mousePosY <= cardBottomY) {
                    std::cout << "[DEBUG] Player picked a card from opponent's deck!" << std::endl;
                    // add card to current player's deck
                    currentPlayer.card_hand.push_back(otherPlayer.card_hand[i]);

                    // notify server to update opponent side
                    std::error_code error;
                    send_msg(gameSocket, "picked_card: " + std::to_string(otherPlayer.card_hand[i].id), error);

                    // move on to next turn phase
                    currentPlayer.turn_phase = "play_cards";
                    std::cout << "[DEBUG] Current player turn phase: play_cards" << std::endl;
                }
            }
        }
    }

    // Handle player turn 2nd phase (play set of 4 or power card)
    if (currentPlayer.turn_phase == "play_cards" && AEInputCheckTriggered(AEVK_RBUTTON)) {
        std::error_code ec;
        if (selectedCards.size() == 4) {
            std::string firstLabel = selectedCards[0].label;
            bool sameSet = std::all_of(selectedCards.begin(), selectedCards.end(),
                [&firstLabel](const Card& card) { return card.label == firstLabel; });

            if (sameSet && selectedCards[0].type == "number") {
                std::cout << "Playing normal cards..." << std::endl;
                playCards(selectedCards);
                errorMessage = "";
                turnPlayed = true;

                // Send updated card hand to server
                send_msg(gameSocket, "card_hand:" + currentPlayer.getCardsStr(), ec);
                
                // Update both clients discard pile
                std::string cardsStr;
                for (const auto& c : selectedCards) {
                    if (!cardsStr.empty()) {
                        cardsStr += " "; // Add a space between elements
                    }
                    cardsStr += std::to_string(c.id);
                    discardedCards.push_back(c);
                }

                send_msg(gameSocket, "played_numbers:" + cardsStr, ec);
                selectedCards.clear();
            }
            else {
                errorMessage = "Invalid selection! Cards must be of the same type.";
            }
        }
        else if (selectedCards.size() == 1) {
            // Check is power card
            if (selectedCards[0].type == "power") {
                std::cout << "Playing a power card..." << std::endl;
                // Played Swap Hands
                if (selectedCards[0].label == "Swap Hands") {
                    std::cout << "[DEBUG] Played Swap Hands" << std::endl;
                    // send updated card hand to server
                    send_msg(gameSocket, "swap_hands:" + std::to_string(selectedCards[0].id), ec);
                }
                // Played Reveal Hand
                else if (selectedCards[0].label == "Reveal Hand") {
                    std::cout << "[DEBUG] Played Reveal Hand" << std::endl;
                    revealHand = true;
                    AEGetTime(&revealHandStartTime);
                    // notify server to remove this power card first
                    send_msg(gameSocket, "played_power:" + std::to_string(selectedCards[0].id), ec);
                }
                // Played Give Card
                else if (selectedCards[0].label == "Give Card") {
                    std::cout << "[DEBUG] Played Give Card" << std::endl;
                    givingCard = true;
                    // notify server to remove this power card first
                    send_msg(gameSocket, "played_power:" + std::to_string(selectedCards[0].id), ec);
                }
                // Played Swap Power Cards
                else if (selectedCards[0].label == "Swap Power Cards") {
                    std::cout << "[DEBUG] Played Reshuffle Power Cards" << std::endl;
                    send_msg(gameSocket, "swap_power:" + std::to_string(selectedCards[0].id), ec);
                }

                discardedCards.push_back(selectedCards[0]);
                currentPlayer.power_cards.erase(currentPlayer.power_cards.begin());
                playCards(selectedCards);
                selectedCards.clear();
                errorMessage = "";

                // don't end turn yet if player is doing power card effect
                if (!givingCard && !revealHand) {
                    turnPlayed = true;
                }
            }
            else if (givingCard) {
                std::cout << "Player is giving card: " << std::to_string(selectedCards[0].id) << " to opponent." << std::endl;
                send_msg(gameSocket, "give_card:" + std::to_string(selectedCards[0].id), ec);
                turnPlayed = true;
                givingCard = false;
            }
            else {
                std::cout << "Invalid selection for power card." << std::endl;
                errorMessage = "Invalid selection! Only power cards can be played individually.";
            }
        }
        else if (selectedCards.size() > 0 && selectedCards.size() < 4) {
            errorMessage = "You must select 4 cards to play!";
            std::cout << "Invalid selection! Not enough cards selected for normal play." << std::endl;
        }
    }

    if (currentPlayer.turn_phase == "draw_card") {
        infoMessage = "Draw a card from your opponent or from the power deck!";
    }
    else if (currentPlayer.turn_phase == "play_cards") {
        infoMessage = "Play a set of 4 or a power card! Press space to skip this turn.";
    }
    if (givingCard) {
        infoMessage = "Choose 1 card to give to your opponent...";
    }

    // count and display reveal hand time
    if (revealHand) {
        f64 timeElapsed = currentTime - revealHandStartTime;
        infoMessage = "Opponent's hand will be hidden in " + std::to_string(static_cast<int>(REVEAL_HAND_TIMER - timeElapsed)+1) + "...";

        if (timeElapsed >= REVEAL_HAND_TIMER) {
            revealHand = false;
            turnPlayed = true;
        }
    }
    // End the turn if the player has already played
    if (turnPlayed) {
        std::cout << "Turn already played. Preventing further actions." << std::endl;
        currentPlayer.is_player_turn = false;
        infoMessage = "";

        // Send "end_turn" message to server
        std::error_code ec;
        send_msg(gameSocket, "end_turn", ec);
        selectedCards.clear();
        return;
    }

    // press space to skip
    if (currentPlayer.turn_phase == "play_cards" && AEInputCheckTriggered(AEVK_SPACE) && !givingCard && !revealHand) {
        // Send a turn skip message to the server
        std::error_code ec;
        send_msg(gameSocket, "skip_turn", ec);
        infoMessage = "";

        // Update local turn status
        currentPlayer.is_player_turn = false;
        otherPlayer.is_player_turn = true;
        turnPlayed = true;
        selectedCards.clear();
    }

    // Check if the player clicks the "Sort" button
    if (ifClickFunction(sortButtonPos, sortButtonWidth, sortButtonHeight)) {
        std::cout << "[DEBUG] Sort button clicked!" << std::endl;
        SortPlayerHand();

        // update player hand on server side & opponent side
        std::error_code ec;
        send_msg(gameSocket, "card_hand:" + currentPlayer.getCardsStr(), ec);
    }
    // Check if the player clicks the "Shuffle" button
    if (ifClickFunction(shuffleButtonPos, shuffleButtonWidth, shuffleButtonHeight)) {
        std::cout << "[DEBUG] Shuffle button clicked!" << std::endl;
        ShufflePlayerHand();

        // update player hand on server side & opponent side
        std::error_code ec;
        send_msg(gameSocket, "card_hand:" + currentPlayer.getCardsStr(), ec);
    }
}

void Gameboard_Draw() {
	AEGfxSetBackgroundColor(0.490f, 0.510f, 0.722f);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

	// Draw Opponent Area
	{
		AEMtx33 transform = { 0 };
		AEMtx33Concat(&transform, &OpponentAreaTranslate, &playareaScale);
		AEGfxSetTransform(transform.m);
		AEGfxSetColorToMultiply(0.165f, 0.196f, 0.294f, 1.0f);
		AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
	}

	// Draw User Area
	{
		AEMtx33 transform = { 0 };
		AEMtx33Concat(&transform, &UserAreaTranslate, &playareaScale);
		AEGfxSetTransform(transform.m);
		AEGfxSetColorToMultiply(0.165f, 0.196f, 0.294f, 1.0f);
		AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
        //AEGfxPrint(font, "Cards", powerDeckPosition.x / 800,
            //(powerDeckPosition.y + 40) / 450, 0.9f, 0.0f, 0.0f, 0.0f, 1.0f);
	}

    // Display waiting message
    if (waitingRoom) {
        AEGfxPrint(font, "Waiting for Player2...", -0.15f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    // Display info message (above card hand)
    AEGfxPrint(font, infoMessage.c_str(), -((infoMessage.size() * 0.012f) / 2.0f), -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	// Draw current player and opponent card hands
	{
		size_t p1CardsNum = currentPlayer.card_hand.size();
		for (size_t i = 0; i < p1CardsNum; i++) {
            if (i != draggedCardIndex) {
				Card card = currentPlayer.card_hand[i];
				drawCard(card);
            }
        }

        size_t oppCardsNum = otherPlayer.card_hand.size();
        for (size_t i = 0; i < oppCardsNum; i++) {
            Card card = otherPlayer.card_hand[i];
            bool hideCard = revealHand ? false : true;
            drawCard(card, hideCard);
        }

		// draw dragged card after the rest of the cards so it overlaps on top
        if (draggedCardIndex != -1) {
            drawCard(currentPlayer.card_hand[draggedCardIndex]);
        }
	}

    // Draw power card deck
	drawPowerDeck();

    // Draw player's power cards
    for (size_t i = 0; i < currentPlayer.power_cards.size(); i++) {
		Card& card = currentPlayer.power_cards[i];
        
        bool selected = false;
        for (auto& c : selectedCards) {
            if (c.id == card.id) {
                selected = true;
                break;
            }
        }

        // if power card is selected, draw it 15px upwards to indicate selected
		card.position = { -850.0f + powerCardSpacing * (static_cast<float>(i) + 1), selected ? -185.0f : -200.0f };
        DrawPowerCard(card.label, card.position.x, card.position.y);
	}

    // Draw Opponent's Power Cards
    for (size_t i = 0; i < otherPlayer.power_cards.size(); i++) {
        Card& card = otherPlayer.power_cards[i];

        card.position = { 870.0f - (cardSize.x + powerCardSpacing * (static_cast<float>(i) + 1)), 120.0f };
        DrawOppPowerCard(card.position.x, card.position.y);
    }


    // Draw power card deck
    drawDiscardPile();

    // Draw played/discarded cards
    for (size_t i = 0; i < discardedCards.size(); i++) {
        Card& card = discardedCards[i];

        if (!card.isPlaced) {
            float offsetX = static_cast<float>(rand() % 11 - 5);
            float offsetY = static_cast<float>(rand() % 11 - 5);

            card.position = { discardedDeckPosition.x + offsetX,
                              discardedDeckPosition.y + offsetY };

            card.isPlaced = true;
        }
        if (card.type == "power") {
            DrawPowerCard(card.label, card.position.x, card.position.y);
        }
        else if (card.type == "number") {
            drawCard(card);
        }
    }
 
    // Draw Sort Button
    // Transform for button placement
    AEMtx33 sortButtonScale, sortButtonTranslate, sortButtonTransform;

    // Scale and move the button
    AEMtx33Scale(&sortButtonScale, sortButtonWidth, sortButtonHeight);
    AEMtx33Trans(&sortButtonTranslate, sortButtonPos.x, sortButtonPos.y);
    AEMtx33Concat(&sortButtonTransform, &sortButtonTranslate, &sortButtonScale);

    AEGfxSetTransform(sortButtonTransform.m);

    // Button color (Set it to a very distinct color to test)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
    AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);

    // Draw the "Sort" text on the button
    AEGfxPrint(font, "Sort", (sortButtonPos.x + 22) / 800.0f,
        (sortButtonPos.y + 17) / 450.0f, 1.2f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draw Shuffle Button
    // Transform for shuffle button placement
    AEMtx33 shuffleButtonScale, shuffleButtonTranslate, shuffleButtonTransform;

    AEMtx33Scale(&shuffleButtonScale, shuffleButtonWidth, shuffleButtonHeight);
    AEMtx33Trans(&shuffleButtonTranslate, shuffleButtonPos.x, shuffleButtonPos.y);
    AEMtx33Concat(&shuffleButtonTransform, &shuffleButtonTranslate, &shuffleButtonScale);

    AEGfxSetTransform(shuffleButtonTransform.m);

    // Button color (Orange)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
    AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);

    // Draw the "Shuffle" text on the button
    AEGfxPrint(font, "Shuffle", (shuffleButtonPos.x ) / 800.0f,
        (shuffleButtonPos.y + 17) / 450.0f, 1.2f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Display Turn Information
    if (currentPlayer.is_player_turn && !turnPlayed) {
        f64 timeElapsed = currentTime - turnStartTime;
        //std::cout << "\nCURRENT TIME: " + std::to_string(static_cast<int>(currentTime)) << std::endl;
        std::string turnTimer = "Your turn ends in " + std::to_string(static_cast<int>(TURN_TIMER - timeElapsed) + 1) + 's';
        AEGfxPrint(font, turnTimer.c_str(), -0.95f, 0.45f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f);

        if (timeElapsed >= TURN_TIMER) {
            turnPlayed = true;
            return;
        }
    }

    // Display any additional error messages
    if (!errorMessage.empty()) {
        f32 msgY = errorMessage == "Opponent's Turn" ? 0.45f : 0.35f;
        AEGfxPrint(font, errorMessage.c_str(), -0.95f, msgY, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    }

    // Handle Game Over Screen
    if (currentPlayer.turn_phase == "game_over") {
        AEGfxTexture* gameEndTexture = currentPlayer.is_winner ? winScreenTexture : loseScreenTexture;

        // Draw Game Over Screen
        if (gameEndTexture) {
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetTransparency(1.0f);
            AEGfxTextureSet(gameEndTexture, 0, 0);

            // Transformations for centering the image
            AEMtx33 scaleMatrix, translateMatrix, transform;
            AEMtx33Scale(&scaleMatrix, 1600.0f, 1100.0f);  // Full-screen scaling
            AEMtx33Trans(&translateMatrix, -800.0f, -450.0f);
            AEMtx33Concat(&transform, &translateMatrix, &scaleMatrix);

            AEGfxSetTransform(transform.m);
            AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
        }

        // Allow pressing Enter or Space to transition to credits
        if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE)) {
            next = GS_CREDITS;
        }
        return;
    }
}

void Gameboard_Free() {
    if (rectMesh) {
        AEGfxMeshFree(rectMesh);
        rectMesh = nullptr; // Prevent accidental use after free
    }
    
    if (font) {
        AEGfxDestroyFont(font);
        font = 0; // Reset font to prevent dangling reference
    }

    FreePowerCardTextures();
    FreeCardTextures();
    FreeGameEndTextures();

    // Clear selected cards and discard pile to avoid memory leaks
    selectedCards.clear();
    std::vector<Card>().swap(selectedCards); // Force deallocation

    discardedCards.clear();
    std::vector<Card>().swap(discardedCards);

    // Clear error and info messagesS
    errorMessage.clear();
    infoMessage.clear();

    currentPlayer.card_hand.clear();
    currentPlayer.power_cards.clear();
    otherPlayer.card_hand.clear();
    otherPlayer.power_cards.clear();

    std::vector<Card>().swap(currentPlayer.card_hand);
    std::vector<Card>().swap(currentPlayer.power_cards);
    std::vector<Card>().swap(otherPlayer.card_hand);
    std::vector<Card>().swap(otherPlayer.power_cards);
}


void Gameboard_Unload() {
	printf("\ngame board unload\n");
    DisconnectFromServer();
}