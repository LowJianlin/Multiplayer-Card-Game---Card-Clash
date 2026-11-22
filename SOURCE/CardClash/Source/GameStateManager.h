/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        GameStateManager.h
 * Author(s):   Yar Xuan (30%) (base code, gameboard)
 * 				Edthan Yeo (40%) (splashscreen, how to play, credits)
 * 				Low Jianlin (15%) (main menu)
 * 				Xiong Yang (15%) (credits)
 *
 * Description: Header file for game state manager.
 ******************************************************************************/

#ifndef GSM_H // Include guard to prevent multiple inclusions
#define GSM_H

typedef void(*FP)(void);

extern int current, previous, next;
extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

void GSM_Initialize(int startingState);
void GSM_Update();

enum GS_STATES
{
	GS_MAINMENU = 0,
	GS_SETTINGS,
	GS_GAMEBOARD,
	GS_CREDITS,      
	GS_HOWTOPLAY,   
	GS_SPLASHSCREEN,
	GS_QUIT,
	GS_RESTART
};

#endif