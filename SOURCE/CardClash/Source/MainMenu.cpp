/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        MainMenu.cpp
 * Author(s):   Low Jianlin (100%)
 *
 * Description: Main menu game state.
 ******************************************************************************/

#include "AEEngine.h"
#include "GameStateManager.h"
#include "Cardlist.h"
#include "GameUtils.h"

static AEGfxTexture* MainMenuTexture;
static AEGfxTexture* MouseSelection;
static AEMtx33 BackgroundTextureScale = { 0 };
static AEMtx33 BackgroundTextureTranslate = { 0 };
static AEMtx33 ButtonScale = { 0 };
static AEMtx33 ButtonTranslate = { 0 };
static AEMtx33 SelectionTextureScale = { 0 };
static AEMtx33 SelectionTextureTranslate = { 0 };

static float SelectionTextureY = { 0 };
static bool StartGame = false;
static bool Tutorial = false;
static bool Credits = false;
static bool Exit = false;

void MainMenu_Load() {
	MainMenuTexture = AEGfxTextureLoad("Assets/MainMenu.png");
	MouseSelection = AEGfxTextureLoad("Assets/MouseSelection.png");
	Cardlist_Load();	
}

void MainMenu_Initialize() {
	AEGfxMeshStart();

	AEGfxTriAdd(
		0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f,    // Top-left
		1.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f,    // Top-right
		0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f);    // Bottom-left

	AEGfxTriAdd(
		0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,     // Bottom-left
		1.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f,    // Top-right
		1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 1.0f);    // Bottom-right

	rectMesh = AEGfxMeshEnd();

	// Settings for background texture
	AEMtx33Scale(&BackgroundTextureScale, 1600.0f, 900.0f);
	AEMtx33Trans(&BackgroundTextureTranslate, -800.0f, -450.0f);

	// Scale matrix for Buttons
	AEMtx33Scale(&ButtonScale, 302.0f, 57.0f);

	// Settings for Selection texture
	AEMtx33Scale(&SelectionTextureScale, 60.0f, 60.0f);
}

void MainMenu_Update() {
	int MouseX, MouseY;
	AEInputGetCursorPosition(&MouseX, &MouseY);

	double GameMouseX = MouseX - 800.0;
	double GameMouseY = 450.0 - MouseY;
	//printf("value: %f, %f\n", GameTestX, GameTestY);


	if (GameMouseX >= -151.0 && GameMouseX <= 151.0 &&
		GameMouseY <= 21.0 && GameMouseY >= -36.0) {
			StartGame = true;
			SelectionTextureY = -36.0f;
			if (AEInputCheckTriggered(AEVK_LBUTTON)) next = GS_GAMEBOARD;
	} else StartGame = false;

	if (GameMouseX >= -151.0 && GameMouseX <= 151.0 &&
		GameMouseY <= -53.0 && GameMouseY >= -110.0) {
			Tutorial = true;
			SelectionTextureY = -110.0f;
			if (AEInputCheckTriggered(AEVK_LBUTTON)) next = GS_HOWTOPLAY;
	} else Tutorial = false;

	if (GameMouseX >= -151.0 && GameMouseX <= 151.0 &&
		GameMouseY <= -127.0 && GameMouseY >= -184.0) {
			Credits = true;
			SelectionTextureY = -184.0f;
			if (AEInputCheckTriggered(AEVK_LBUTTON)) next = GS_CREDITS;
	} else Credits = false;

	if (GameMouseX >= -151.0 && GameMouseX <= 151.0 &&
		GameMouseY <= -201.0 && GameMouseY >= -258.0) {
			Exit = true;
			SelectionTextureY = -258.0f;
			if (AEInputCheckTriggered(AEVK_LBUTTON)) next = GS_QUIT;
	} else Exit = false;
}

void MainMenu_Draw() {
	{	// Draw Background
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		AEGfxTextureSet(MainMenuTexture, 0, 0);
		AEMtx33 transform = { 0 };
		AEMtx33Concat(&transform, &BackgroundTextureTranslate, &BackgroundTextureScale);
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
	}

	{	// Draw Button Selection
		AEMtx33 transform = { 0 };

		if (StartGame) {
			{	// Button Highlight
				AEGfxSetRenderMode(AE_GFX_RM_COLOR);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEMtx33Trans(&ButtonTranslate, -151.0f, SelectionTextureY); //-151, -36

				AEMtx33Concat(&transform, &ButtonTranslate, &ButtonScale);
				AEGfxSetTransform(transform.m);
				AEGfxSetColorToMultiply(0.768f, 0.654f, 0.466f, 0.4f);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
			{	// Texture
				AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
				AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEGfxSetBlendMode(AE_GFX_BM_BLEND);
				AEGfxSetTransparency(1.0f);

				AEMtx33Trans(&SelectionTextureTranslate, 160.0f, SelectionTextureY);

				AEGfxTextureSet(MouseSelection, 0, 0);
				AEMtx33Concat(&transform, &SelectionTextureTranslate, &SelectionTextureScale);
				AEGfxSetTransform(transform.m);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
		}

		if (Tutorial) {
			{	// Button Highlight
				AEGfxSetRenderMode(AE_GFX_RM_COLOR);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEMtx33Trans(&ButtonTranslate, -151.0f, SelectionTextureY); //-151, -36

				AEMtx33Concat(&transform, &ButtonTranslate, &ButtonScale);
				AEGfxSetTransform(transform.m);
				AEGfxSetColorToMultiply(0.768f, 0.654f, 0.466f, 0.4f);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
			{	// Texture
				AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
				AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEGfxSetBlendMode(AE_GFX_BM_BLEND);
				AEGfxSetTransparency(1.0f);

				AEMtx33Trans(&SelectionTextureTranslate, 160.0f, SelectionTextureY);

				AEGfxTextureSet(MouseSelection, 0, 0);
				AEMtx33Concat(&transform, &SelectionTextureTranslate, &SelectionTextureScale);
				AEGfxSetTransform(transform.m);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
		}

		if (Credits) {
			{	// Button Highlight
				AEGfxSetRenderMode(AE_GFX_RM_COLOR);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEMtx33Trans(&ButtonTranslate, -151.0f, SelectionTextureY); //-151, -36

				AEMtx33Concat(&transform, &ButtonTranslate, &ButtonScale);
				AEGfxSetTransform(transform.m);
				AEGfxSetColorToMultiply(0.768f, 0.654f, 0.466f, 0.4f);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
			{	// Texture
				AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
				AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEGfxSetBlendMode(AE_GFX_BM_BLEND);
				AEGfxSetTransparency(1.0f);

				AEMtx33Trans(&SelectionTextureTranslate, 160.0f, SelectionTextureY);

				AEGfxTextureSet(MouseSelection, 0, 0);
				AEMtx33Concat(&transform, &SelectionTextureTranslate, &SelectionTextureScale);
				AEGfxSetTransform(transform.m);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
		}

		if (Exit) {
			{	// Button Highlight
				AEGfxSetRenderMode(AE_GFX_RM_COLOR);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEMtx33Trans(&ButtonTranslate, -151.0f, SelectionTextureY); //-151, -36

				AEMtx33Concat(&transform, &ButtonTranslate, &ButtonScale);
				AEGfxSetTransform(transform.m);
				AEGfxSetColorToMultiply(0.768f, 0.654f, 0.466f, 0.4f);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
			{	// Texture
				AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
				AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
				AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

				AEGfxSetBlendMode(AE_GFX_BM_BLEND);
				AEGfxSetTransparency(1.0f);

				AEMtx33Trans(&SelectionTextureTranslate, 160.0f, SelectionTextureY);

				AEGfxTextureSet(MouseSelection, 0, 0);
				AEMtx33Concat(&transform, &SelectionTextureTranslate, &SelectionTextureScale);
				AEGfxSetTransform(transform.m);
				AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
			}
		}
	}
}

void MainMenu_Free() {
	//printf("mainmenu free\n");
	AEGfxMeshFree(rectMesh);

	if (MainMenuTexture) {
		AEGfxTextureUnload(MainMenuTexture);
		MainMenuTexture = nullptr;
	}
	if (MouseSelection) {
		AEGfxTextureUnload(MouseSelection);
		MouseSelection = nullptr;
	}
}

void MainMenu_Unload() {
	//printf("mainmenu unload\n");
}