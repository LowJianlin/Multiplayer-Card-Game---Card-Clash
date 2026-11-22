/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        HowToPlay.cpp
 * Author(s):   Edthan Yeo (100%)
 *
 * Description: How To Play game state.
 ******************************************************************************/

#include "HowToPlay.h"
#include "GameStateManager.h"
#include "MainMenu.h"
#include "GameUtils.h"

// Define global variables here (only in `Credits.cpp`)
AEMtx33 HTPBackgroundTextureScale = { 0 };
AEMtx33 HTPBackgroundTextureTranslate = { 0 };

// Load credit page background texture
AEGfxTexture* htpBackground = nullptr;

// Load the textures
void HowToPlay_Load() {
    htpBackground = AEGfxTextureLoad("Assets/How_To_Play.png");
}

// Initialize credits
void HowToPlay_Init() {
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
    AEMtx33Scale(&HTPBackgroundTextureScale, 1600.0f, 900.0f);
    AEMtx33Trans(&HTPBackgroundTextureTranslate, -800.0f, -450.0f);
}

// Draw credits based on page number
void HowToPlay_Draw() {
    // Draw Background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    AEGfxTextureSet(htpBackground, 0, 0);
    AEMtx33 transform = { 0 };
    AEMtx33Concat(&transform, &HTPBackgroundTextureTranslate, &HTPBackgroundTextureScale);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
}

// Handle button clicks
void HowToPlay_Update() {
    if (AEInputCheckCurr(AEVK_ESCAPE)) {
        next = GS_MAINMENU;
    }
}

// Free the textures
void HowToPlay_Free() {
    if (htpBackground) {
        AEGfxTextureUnload(htpBackground);
        htpBackground = nullptr;
    }
    if (rectMesh) {
        AEGfxMeshFree(rectMesh);
        rectMesh = nullptr;
    }
}

// Exit credits
void HowToPlay_Exit() {
}
