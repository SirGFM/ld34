/**
 * Game's entry point
 *
 * @file src/main.c
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSave.h>
#include <GFraMe/core/gfmAudio_bkend.h>

#include <ld34/game.h>
#include <ld34/gamestate.h>

#include <stdlib.h>
#include <string.h>

/**
 * Main game struct; Kept outside main in case I try to compile it for
 * emscripten;
 *
 * I like trying weird things, from time to time, so I'll see how bad is to use
 * a global variable...
 */
gameCtx *pGame;
gameAssets *pAssets;
void *pState;

gfmRV main_loop() {
    gfmRV rv;

    while (gfm_didGetQuitFlag(pGame->pCtx) != GFMRV_TRUE) {
        /* Check if switching states */
        if (pGame->nextState != state_none) {
            /* Init the current state */
            switch (pGame->nextState) {
                case state_intro: ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED); break;
                case state_game: rv = gamestate_init(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }
            ASSERT(rv == GFMRV_OK, rv);

            pGame->curState = pGame->nextState;
            pGame->nextState = state_none;
        }

        rv = gfm_handleEvents(pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);

        while (gfm_isUpdating(pGame->pCtx) == GFMRV_TRUE) {
            rv = gfm_fpsCounterUpdateBegin(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);

            /* Update the current state */
            switch (pGame->curState) {
                case state_intro: ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED); break;
                case state_game: rv = gamestate_update(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }
            ASSERT(rv == GFMRV_OK, rv);

            rv = gfm_fpsCounterUpdateEnd(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);
        }
        while (gfm_isDrawing(pGame->pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);

            /* Render the current state */
            switch (pGame->curState) {
                case state_intro: ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED); break;
                case state_game: rv = gamestate_draw(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }

#ifdef DEBUG
            rv = gfm_drawRenderInfo(pGame->pCtx, pAssets->pSset8x8,
                    BBWDT - 8*7/*x*/, 0/*y*/, 0/*tile*/);
            ASSERT(rv == GFMRV_OK, rv);
#endif /* DEBUG */
            rv = gfm_drawEnd(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);
        }

        /* Check if switching states */
        if (pGame->nextState != state_none) {
            /* Clear the current state */
            switch (pGame->curState) {
                case state_intro: ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED); break;
                case state_game: rv = gamestate_clean(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }
            ASSERT(rv == GFMRV_OK, rv);

            pGame->curState = state_none;
        }
    }

    rv = GFMRV_OK;
__ret:
    if (pGame->curState != state_none) {
        /* Clear the current state */
        switch (pGame->curState) {
            case state_intro: ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED); break;
            case state_game: gamestate_clean(); break;
            default: {}
        }
        pGame->curState = state_none;
    }

    return rv;
}

/**
 * Entry point
 *
 * @param  [ in]argc Number of arguments
 * @param  [ in]argv List of arguments
 */
int main(int argc, char *argv[]) {
    configCtx config;
    gfmRV rv;

    /* Clean the game, so it's properly release on error */
    pGame = 0;
    pAssets = 0;

    /* Set all default values */
    config.dps = 60;
    config.fps = 60;
    config.vsync = 0;
    config.ups = 60;

    /* Alloc the assets struct */
    pAssets = (gameAssets*)malloc(sizeof(gameAssets));
    ASSERT(pAssets, GFMRV_ALLOC_FAILED);
    memset(pAssets, 0x0, sizeof(gameAssets));

    /* Alloc the game */
    pGame = (gameCtx*)malloc(sizeof(gameCtx));
    ASSERT(pGame, GFMRV_ALLOC_FAILED);
    memset(pGame, 0x0, sizeof(gameCtx));

    rv = gfm_getNew(&(pGame->pCtx));
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_initStatic(pGame->pCtx, ORG, TITLE);
    ASSERT(rv == GFMRV_OK, rv);

    /* TODO Load config file */

    rv = gfm_initGameWindow(pGame->pCtx, BBWDT, BBHGT, WNWDT, WNHGT, CAN_RESIZE,
            config.vsync);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_setBackground(pGame->pCtx, BGCOLOR);
    ASSERT(rv == GFMRV_OK, rv);

    /* TODO Enable audio */
#if 0
    rv = gfm_initAudio(pGame->pCtx, gfmAudio_defQuality);
    ASSERT(rv == GFMRV_OK, rv);
#endif /* 0 */

    rv = gfm_loadTextureStatic(&(pAssets->texHandle), pGame->pCtx, TEXATLAS,
            COLORKEY);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_createSpritesetCached(&(pAssets->pSset8x8), pGame->pCtx,
            pAssets->texHandle, 8, 8);
    rv = gfm_createSpritesetCached(&(pAssets->pSset16x16), pGame->pCtx,
            pAssets->texHandle, 16, 16);
    rv = gfm_createSpritesetCached(&(pAssets->pSset32x16), pGame->pCtx,
            pAssets->texHandle, 32, 16);
    ASSERT(rv == GFMRV_OK, rv);
    /* TODO Load audio assets */

    rv = gfm_setFPS(pGame->pCtx, config.fps);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_setStateFrameRate(pGame->pCtx, config.ups, config.dps);
    ASSERT(rv == GFMRV_OK, rv);

#ifdef DEBUG
    rv = gfm_initFPSCounter(pGame->pCtx, pAssets->pSset8x8, 0/*firstTile*/);
    ASSERT(rv == GFMRV_OK, rv);
#endif /* DEBUG */

    /* TODO Set the main state as the intro */
    pGame->nextState = state_game;

    rv = main_loop();
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    if (pGame) {
        /* TODO Free everything else */
        gfm_free(&(pGame->pCtx));
    }
    if (pGame) {
        free(pGame);
    }
    if (pAssets) {
        free(pAssets);
    }

    return rv;
}

