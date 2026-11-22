/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Main.cpp
 * Author(s):   Yar Xuan (100%)
 *
 * Description: Main program loop including game state manager and game loop.
 ******************************************************************************/

#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "GameStateManager.h"
#include "Cardlist.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// initialize system
	AESysInit(hInstance, nCmdShow, 1600, 900, 0, 60, false, NULL);
	AESysReset();
	AESysSetWindowTitle("Card Clash");

	// initialize GSM
	GSM_Initialize(GS_SPLASHSCREEN);

	while (current != GS_QUIT) {
		if (current != GS_RESTART) {
			GSM_Update();
			fpLoad();
		}
		else {
			next = previous;
			current = previous;
		}

		fpInitialize();

		// game loop
		while (next == current) {
			AESysFrameStart();
			fpUpdate();
			fpDraw();
			AESysFrameEnd();
			
			if (0 == AESysDoesWindowExist()) {
				next = GS_QUIT;
			}
		}

		fpFree();

		if (next != GS_RESTART) {
			fpUnload();
		}

		previous = current;
		current = next;
	}
	// free the system
	Cardlist_Free();
	AESysExit();
}