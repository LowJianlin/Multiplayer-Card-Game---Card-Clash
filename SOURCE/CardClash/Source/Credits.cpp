/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Credits.cpp
 * Author(s):   Xiong Yang (50%)
 *              Edthan Yeo (50%)
 *
 * Description: Credits game state.
 ******************************************************************************/

#include "Credits.h"
#include "GameStateManager.h"
#include "MainMenu.h"
#include "GameUtils.h"

// Define global variables here (only in `Credits.cpp`)
AEMtx33 CreditBackgroundTextureScale = { 0 };
AEMtx33 CreditBackgroundTextureTranslate = { 0 };

// Load credit page background texture
AEGfxTexture* creditsBackground = nullptr;

// Load the textures
void Credit_Load() {
    creditsBackground = AEGfxTextureLoad("Assets/Credits_Page.png");
}

// Initialize credits
void Credits_Init() {
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
    AEMtx33Scale(&CreditBackgroundTextureScale, 1600.0f, 900.0f);
    AEMtx33Trans(&CreditBackgroundTextureTranslate, -800.0f, -450.0f);
}

// Draw credits based on page number
void Credits_Draw() {
    // Draw Background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    AEGfxTextureSet(creditsBackground, 0, 0);
    AEMtx33 transform = { 0 };
    AEMtx33Concat(&transform, &CreditBackgroundTextureTranslate, &CreditBackgroundTextureScale);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
}

// Handle button clicks
void Credits_Update() {
    if (AEInputCheckCurr(AEVK_ESCAPE)) {
        next = GS_MAINMENU;
    }
}

// Free the textures
void Credits_Free() {
    if (creditsBackground) {
        AEGfxTextureUnload(creditsBackground);
        creditsBackground = nullptr; // Prevent dangling pointer
    }
    if (rectMesh) {
        AEGfxMeshFree(rectMesh);
        rectMesh = nullptr; // Prevent dangling pointer
    }
}


// Exit credits
void Credits_Exit() {
}
