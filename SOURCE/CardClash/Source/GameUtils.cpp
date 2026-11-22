/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        GameUtils.cpp
 * Author(s):   Edthan Yeo (80%) (draw power card hand, draw power card deck, sort/shuffle cards, game end screen)
 *              Xiong Yang (20%) (playCards, draw discard pile)
 *
 * Description: Utility functions used mainly in Gameboard.cpp, includes in-game logic functions and graphics functions.
 ******************************************************************************/

#include "AEEngine.h"
#include "Cardlist.h"
#include <algorithm>
#include <random>

AEGfxVertexList* rectMesh = 0;
AEMtx33 cardScale = { 0 }, cardTranslate = { 0 };
AEVec2 cardSize = { 75.0f, 120.0f };
s8 font;
AEVec2 powerDeckPosition = { -130.0f, -50.0f };
AEVec2 discardedDeckPosition = { 70.0f, -50.0f }; // Position of discarded card pile
AEVec2 gameEndRect = { -0.5, -1 };
bool isPowerDeckHovered = false;
int powerDeckSize = 0;
f64 turnStartTime; // extern this, used in SocketUtils to start turn timer

void playCards(std::vector<Card>& cardsToPlay) {
    // Play the cards (game-specific logic)
    for (Card& card : cardsToPlay) {
        // For example, remove cards from hand and update gameplay
        auto it = std::find(currentPlayer.card_hand.begin(), currentPlayer.card_hand.end(), card);
        if (it != currentPlayer.card_hand.end()) {
            currentPlayer.card_hand.erase(it);
        }
    }
}

// Texture pointers
AEGfxTexture* JokerCardTexture = nullptr;
AEGfxTexture* No1CardTexture = nullptr;
AEGfxTexture* No2CardTexture = nullptr;
AEGfxTexture* No3CardTexture = nullptr;
AEGfxTexture* No4CardTexture = nullptr;
AEGfxTexture* No5CardTexture = nullptr;
AEGfxTexture* No6CardTexture = nullptr;
AEGfxTexture* OpponentBack = nullptr;

// Load textures for normal cards
void LoadCardTextures() {
	JokerCardTexture = AEGfxTextureLoad("Assets/joker_card.png");
	No1CardTexture = AEGfxTextureLoad("Assets/1-card.png");
	No2CardTexture = AEGfxTextureLoad("Assets/2-card.png");
	No3CardTexture = AEGfxTextureLoad("Assets/3-card.png");
	No4CardTexture = AEGfxTextureLoad("Assets/4-card.png");
	No5CardTexture = AEGfxTextureLoad("Assets/5-card.png");
	No6CardTexture = AEGfxTextureLoad("Assets/6-card.png");
	OpponentBack = AEGfxTextureLoad("Assets/opponent_back.png");

}

// Free textures when the game ends
void FreeCardTextures() {
	AEGfxTextureUnload(JokerCardTexture);
	AEGfxTextureUnload(No1CardTexture);
	AEGfxTextureUnload(No2CardTexture);
	AEGfxTextureUnload(No3CardTexture);
	AEGfxTextureUnload(No4CardTexture);
	AEGfxTextureUnload(No5CardTexture);
	AEGfxTextureUnload(No6CardTexture);
	AEGfxTextureUnload(OpponentBack);
}


void drawCard(Card card, bool hideCard = false) {
	AEGfxTexture* textureToDraw = nullptr;

	if (hideCard) {
		textureToDraw = OpponentBack; // Use opponent's hidden card texture
	}
	else {
		// ? Ensure correct texture is applied
		switch (card.label[0]) {
		case 'J': textureToDraw = JokerCardTexture; break;
		case '1': textureToDraw = No1CardTexture; break;
		case '2': textureToDraw = No2CardTexture; break;
		case '3': textureToDraw = No3CardTexture; break;
		case '4': textureToDraw = No4CardTexture; break;
		case '5': textureToDraw = No5CardTexture; break;
		case '6': textureToDraw = No6CardTexture; break;
		default: textureToDraw = nullptr; break;
		}
	}

	if (textureToDraw) {
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

		// ? Modify color when selected (apply a green tint)
		if (std::find(selectedCards.begin(), selectedCards.end(), card) != selectedCards.end()) {
			AEGfxSetColorToMultiply(0.5f, 1.0f, 0.5f, 0.8f);  // Green tint
		}
		else {
			AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);  // Normal color
		}

		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);
		AEGfxTextureSet(textureToDraw, 0, 0);

		// ? Restore the **original transformation logic**
		AEMtx33 localcardTranslate, transform;
		AEMtx33Trans(&localcardTranslate, card.position.x, card.position.y);
		AEMtx33Concat(&transform, &localcardTranslate, &cardScale); // ? Ensures correct placement
		AEGfxSetTransform(transform.m);

		// ? Draw the card
		AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	}

	// ? Ensure card labels are correctly displayed
	if (!hideCard) {
		AEGfxPrint(font, card.label.c_str(), (card.position.x + 15) / 800,
			(card.position.y + 50) / 450, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void drawPowerDeck() {
	// Translation matrix for power deck
	AEMtx33 powerDeckTranslate = { 0 };
	AEMtx33Trans(&powerDeckTranslate, powerDeckPosition.x, powerDeckPosition.y);

	AEMtx33 transform = { 0 };
	AEMtx33Concat(&transform, &powerDeckTranslate, &cardScale);
	AEGfxSetTransform(transform.m);

	// Change color if hovered
	if (isPowerDeckHovered) {
		AEGfxSetColorToMultiply(0.8f, 0.8f, 1.0f, 1.0f);
	}
	else {
		AEGfxSetColorToMultiply(0.6f, 0.6f, 1.0f, 1.0f);
	}

	AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);

	// Draw "Power Cards" text on the deck
	// Print "Power" on one line
	AEGfxPrint(font, "Power", powerDeckPosition.x / 800.0f,
		(powerDeckPosition.y + 80.0f) / 450.0f, 0.9f, 0.0f, 0.0f, 0.0f, 1.0f);

	// Print "Cards" below it
	AEGfxPrint(font, "Cards", powerDeckPosition.x / 800.0f,
		(powerDeckPosition.y + 40.0f) / 450.0f, 0.9f, 0.0f, 0.0f, 0.0f, 1.0f);

	// Print deck size to the right
	AEGfxPrint(font, std::to_string(powerDeckSize).c_str(), (powerDeckPosition.x + 90.0f) / 800.0f, (powerDeckPosition.y + 40.0f) / 450.0f, 0.9f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void drawDiscardPile() {
	// Translation matrix for discarded power deck
	AEMtx33 discardPileTranslate = { 0 };
	AEMtx33Trans(&discardPileTranslate, discardedDeckPosition.x, discardedDeckPosition.y);

	AEMtx33 transform = { 0 };
	AEMtx33Concat(&transform, &discardPileTranslate, &cardScale);
	AEGfxSetTransform(transform.m);
	AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
	AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);

	// Draw "Discard Pile" text
	AEGfxPrint(font, "Discard", discardedDeckPosition.x / 800,
		(discardedDeckPosition.y + 80) / 450, 0.9f, 0.0f, 0.0f, 0.0f, 1.0f);
	AEGfxPrint(font, "Pile", discardedDeckPosition.x / 800,
		(discardedDeckPosition.y + 40) / 450, 0.9f, 0.0f, 0.0f, 0.0f, 1.0f);
}


// Texture pointers for game end screens
AEGfxTexture* winScreenTexture = nullptr;
AEGfxTexture* loseScreenTexture = nullptr;

// Load the Win/Lose textures
void LoadGameEndTextures() {
	winScreenTexture = AEGfxTextureLoad("Assets/Win_Screen.png");
	loseScreenTexture = AEGfxTextureLoad("Assets/Lose_Screen.png");
}

// Free the textures when exiting the game
void FreeGameEndTextures() {
	AEGfxTextureUnload(winScreenTexture);
	AEGfxTextureUnload(loseScreenTexture);
}

bool ifClickFunction(AEVec2 buttonPos, float width, float height) {
	int mouseX, mouseY;
	AEInputGetCursorPosition(&mouseX, &mouseY);

	// Convert mouse coordinates to absolute game world coordinates
	float worldMouseX = static_cast<float>(mouseX) - 800.0f;
	float worldMouseY = 450.0f - static_cast<float>(mouseY);

	if (worldMouseX >= buttonPos.x && worldMouseX <= buttonPos.x + width &&
		worldMouseY >= buttonPos.y && worldMouseY <= buttonPos.y + height) {
		return AEInputCheckTriggered(AEVK_LBUTTON);
	}
	return false;
}

// Texture pointers
AEGfxTexture* MainMenuTexture = nullptr;
AEGfxTexture* MouseSelection = nullptr;
AEGfxTexture* giveCardTexture = nullptr;
AEGfxTexture* revealHandTexture = nullptr;
AEGfxTexture* swapHandsTexture = nullptr;
AEGfxTexture* reshuffleTexture = nullptr;

// Load power card textures
void LoadPowerCardTextures() {
	giveCardTexture = AEGfxTextureLoad("Assets/give_card.png");
	revealHandTexture = AEGfxTextureLoad("Assets/reveal_hand.png");
	swapHandsTexture = AEGfxTextureLoad("Assets/swap_hands.png");
	reshuffleTexture = AEGfxTextureLoad("Assets/reshuffle_power_cards.png");

	if (!giveCardTexture || !revealHandTexture || !swapHandsTexture || !reshuffleTexture) {
		printf("Error loading power card textures!\n");
	}
}

// Draw power card on the gameboard
void DrawPowerCard(std::string& label, float x, float y) {
	AEGfxTexture* textureToDraw = nullptr;

	if (label == "Swap Hands") {
		textureToDraw = swapHandsTexture;
	}
	else if (label == "Reveal Hand") {
		textureToDraw = revealHandTexture;
	}
	else if (label == "Give Card") {
		textureToDraw = giveCardTexture;
	}
	else if (label == "Swap Power Cards") {
		textureToDraw = reshuffleTexture;
	}
	else {
		return;
	}

	if (textureToDraw) {
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		AEGfxTextureSet(textureToDraw, 0, 0);

		// Apply transformations (position, size)
		AEMtx33 transform;
		AEMtx33Scale(&transform, cardSize.x, cardSize.y); 
		AEMtx33TransApply(&transform, &transform, x, y);

		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);

		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	}
}

void DrawOppPowerCard(float x, float y) {
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.0f);

	AEGfxTextureSet(OpponentBack, 0, 0);

	// Apply transformations (position, size)
	AEMtx33 transform;
	AEMtx33Scale(&transform, cardSize.x, cardSize.y);
	AEMtx33TransApply(&transform, &transform, x, y);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
}


// Free textures when game ends
void FreePowerCardTextures() {
	AEGfxTextureUnload(giveCardTexture);
	AEGfxTextureUnload(revealHandTexture);
	AEGfxTextureUnload(swapHandsTexture);
	AEGfxTextureUnload(reshuffleTexture);
}

void SortPlayerHand() {
	std::sort(currentPlayer.card_hand.begin(), currentPlayer.card_hand.end(),
		[](const Card& a, const Card& b) {
			return a.label < b.label; // Sort based on card label
		});
}

void ShufflePlayerHand() {
	std::random_device rd;
	std::mt19937 g(rd()); // Random number generator

	std::shuffle(currentPlayer.card_hand.begin(), currentPlayer.card_hand.end(), g);
}
