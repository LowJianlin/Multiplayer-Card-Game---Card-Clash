/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        GameStateManager.cpp
 * Author(s):   Yar Xuan (30%) (base code, gameboard)
 * 				Edthan Yeo (40%) (splashscreen, how to play, credits)
 * 				Low Jianlin (15%) (main menu)
 * 				Xiong Yang (15%) (credits)
 *
 * Description: Game state manager to handle game states.
 ******************************************************************************/

#include "AEEngine.h"
#include "GameStateManager.h"
#include "MainMenu.h"
#include "Gameboard.h"
#include "Credits.h"
#include "HowToPlay.h"
#include "Splashscreen.h"

int current = 0, previous = 0, next = 0;
FP fpLoad = nullptr, fpInitialize = nullptr, fpUpdate = nullptr, fpDraw = nullptr, fpFree = nullptr, fpUnload = nullptr;

void GSM_Initialize(int startingState) {
	current = previous = next = startingState;
}

void GSM_Update() {
	switch (current) {
	case GS_SPLASHSCREEN:
		fpLoad = SplashScreen_Load;
		fpInitialize = SplashScreen_Initialize;
		fpUpdate = SplashScreen_Update;
		fpDraw = SplashScreen_Draw;
		fpFree = SplashScreen_Free;
		fpUnload = SplashScreen_Unload;
		break;

	case GS_MAINMENU:
		fpLoad = MainMenu_Load;
		fpInitialize = MainMenu_Initialize;
		fpUpdate = MainMenu_Update;
		fpDraw = MainMenu_Draw;
		fpFree = MainMenu_Free;
		fpUnload = MainMenu_Unload;
	break; 
	case GS_GAMEBOARD:
		fpLoad = Gameboard_Load;
		fpInitialize = Gameboard_Initialize;
		fpUpdate = Gameboard_Update;
		fpDraw = Gameboard_Draw;
		fpFree = Gameboard_Free;
		fpUnload = Gameboard_Unload;
		break;
	case GS_CREDITS:
		fpLoad = Credit_Load;
		fpInitialize = Credits_Init; 
		fpUpdate = Credits_Update;   
		fpDraw = Credits_Draw;     
		fpFree = Credits_Free;       
		fpUnload = Credits_Exit;     
		break;
	case GS_HOWTOPLAY:
		fpLoad = HowToPlay_Load;
		fpInitialize = HowToPlay_Init;
		fpUpdate = HowToPlay_Update;
		fpDraw = HowToPlay_Draw;
		fpFree = HowToPlay_Free;
		fpUnload = HowToPlay_Exit;
		break;
	case GS_RESTART:
		break;
	case GS_QUIT:
		break;
	default:
		break;
	}
}