/******************************************************************************
 * All content � 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.
 *
 * File:        Splashscreen.cpp
 * Author(s):   Edthan Yeo (100%)
 *
 * Description: Digipen logo splashscreen.
 ******************************************************************************/

#include "Splashscreen.h"
#include "AEEngine.h"
#include "GameStateManager.h"
#include "GameUtils.h"

AEMtx33 SplashscreenTextureScale = { 0 };
AEMtx33 SplashscreenTextureTranslate = { 0 };

AEGfxTexture* SplashTexture = nullptr;
static float timer = 0.0f;

void SplashScreen_Load() {
    SplashTexture = AEGfxTextureLoad("Assets/DigiPen_Splash.png"); 
}

void SplashScreen_Initialize() {
    timer = 0.0f;
    next = GS_SPLASHSCREEN;

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
    AEMtx33Scale(&SplashscreenTextureScale, 1600.0f, 900.0f);
    AEMtx33Trans(&SplashscreenTextureTranslate, -800.0f, -450.0f);
}

void SplashScreen_Update() {
    timer += (float)AEFrameRateControllerGetFrameTime();

    if (AEInputCheckTriggered(AEVK_ESCAPE) || timer > 3.0f) {
        next = GS_MAINMENU; // jump to main menu
    }
}

void SplashScreen_Draw() {
    // Draw Splash Screen Background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    AEGfxTextureSet(SplashTexture, 0, 0);
    AEMtx33 transform = { 0 };
    AEMtx33Concat(&transform, &SplashscreenTextureTranslate, &SplashscreenTextureScale);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);

}

void SplashScreen_Free() {
    if (rectMesh) {
        AEGfxMeshFree(rectMesh);
        rectMesh = nullptr; // Prevent accidental use after free
    }
}

void SplashScreen_Unload() {
    AEGfxTextureUnload(SplashTexture);
    SplashTexture = nullptr;
}
