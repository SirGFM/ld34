/**
 * Game's entry point
 *
 * @file src/main.c
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSave.h>
#include <GFraMe/gfmQuadtree.h>
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
gameButtons *pButtons;
void *pState;

static gfmRV main_updateButtons() {
    gfmRV rv;

    rv = gfm_getKeyState(&(pButtons->left_leg.state), &(pButtons->left_leg.num),
            pGame->pCtx, pButtons->left_leg.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->right_leg.state),
            &(pButtons->right_leg.num), pGame->pCtx,
            pButtons->right_leg.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->left_start.state),
            &(pButtons->left_start.num), pGame->pCtx,
            pButtons->left_start.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->right_start.state),
            &(pButtons->right_start.num), pGame->pCtx,
            pButtons->right_start.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->quit.state), &(pButtons->quit.num),
            pGame->pCtx, pButtons->quit.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->fullscreen.state),
            &(pButtons->fullscreen.num), pGame->pCtx,
            pButtons->fullscreen.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->drawQt.state), &(pButtons->drawQt.num),
            pGame->pCtx, pButtons->drawQt.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->gif.state), &(pButtons->gif.num),
            pGame->pCtx, pButtons->gif.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->run.state), &(pButtons->run.num),
            pGame->pCtx, pButtons->run.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->next.state), &(pButtons->next.num),
            pGame->pCtx, pButtons->next.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->pause.state), &(pButtons->pause.num),
            pGame->pCtx, pButtons->pause.handle);
    ASSERT(rv == GFMRV_OK, rv);

    if ((pButtons->quit.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
        rv = gfm_setQuitFlag(pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);
    }

    if ((pButtons->drawQt.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
        pGame->drawQt = !pGame->drawQt;
    }

    if ((pButtons->gif.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
#ifdef DEBUG
        rv =  gfm_didExportGif(pGame->pCtx);
        if (rv == GFMRV_TRUE || rv == GFMRV_GIF_OPERATION_NOT_ACTIVE) {
            rv = gfm_recordGif(pGame->pCtx, 10000, "anim.gif", 8, 0);
            ASSERT(rv == GFMRV_OK, rv);
        }
#endif
    }

    if ((pButtons->run.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
#ifdef DEBUG
        pGame->run = 1;
#endif
    }
    if ((pButtons->next.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
#ifdef DEBUG
        pGame->next = 1;
#endif
    }
    if ((pButtons->pause.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
#ifdef DEBUG
        pGame->run = 0;
        pGame->next = 0;
#endif
    }

    if ((pButtons->fullscreen.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
        if (pGame->isFullscreen) {
            rv = gfm_setWindowed(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);
            pGame->isFullscreen = 0;
        }
        else {
            rv = gfm_setFullscreen(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);
            pGame->isFullscreen = 1;
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

static gfmRV main_loop() {
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

            rv = main_updateButtons();
            ASSERT(rv == GFMRV_OK, rv);

#if defined(DEBUG)
            if (pGame->run || pGame->next) {
#endif

            /* Update the current state */
            switch (pGame->curState) {
                case state_intro: ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED); break;
                case state_game: rv = gamestate_update(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }
            ASSERT(rv == GFMRV_OK, rv);

#if defined(DEBUG)
                if (pGame->next) {
                    pGame->next = 0;
                }
            }
#endif

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
                case state_game: gamestate_clean(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }

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
    pButtons = 0;
    pState = 0;

    /* Set all default values */
    config.dps = 60;
    config.fps = 60;
    config.vsync = 0;
    config.ups = 60;

    /* Alloc the buttons struct */
    pButtons = (gameButtons*)malloc(sizeof(gameButtons));
    ASSERT(pButtons, GFMRV_ALLOC_FAILED);
    memset(pButtons, 0x0, sizeof(gameButtons));

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

    /* Initialize all buttons */
    rv = gfm_addVirtualKey(&(pButtons->left_leg.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->left_start.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->right_leg.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->right_start.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->fullscreen.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->quit.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->drawQt.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->gif.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->run.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->next.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->pause.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_bindInput(pGame->pCtx, pButtons->left_leg.handle, gfmKey_f);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->left_leg.handle,
            gfmController_l1, 0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->left_leg.handle,
            gfmController_l1, 1/*port*/);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_bindInput(pGame->pCtx, pButtons->right_leg.handle, gfmKey_j);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_leg.handle,
            gfmController_r1, 0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_leg.handle,
            gfmController_r1, 1/*port*/);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_bindInput(pGame->pCtx, pButtons->left_start.handle, gfmKey_g);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->left_start.handle,
            gfmController_select, 0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->left_start.handle,
            gfmController_select, 1/*port*/);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_bindInput(pGame->pCtx, pButtons->right_start.handle, gfmKey_h);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_start.handle,
            gfmController_start, 0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_start.handle,
            gfmController_start, 1/*port*/);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_bindInput(pGame->pCtx, pButtons->fullscreen.handle, gfmKey_f12);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindInput(pGame->pCtx, pButtons->drawQt.handle, gfmKey_f1);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindInput(pGame->pCtx, pButtons->gif.handle, gfmKey_f2);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindInput(pGame->pCtx, pButtons->run.handle, gfmKey_f5);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindInput(pGame->pCtx, pButtons->next.handle, gfmKey_f6);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindInput(pGame->pCtx, pButtons->pause.handle, gfmKey_f7);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_bindInput(pGame->pCtx, pButtons->quit.handle, gfmKey_esc);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->quit.handle, gfmController_home,
            0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->quit.handle, gfmController_home,
            1/*port*/);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_setFPS(pGame->pCtx, config.fps);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_setStateFrameRate(pGame->pCtx, config.ups, config.dps);
    ASSERT(rv == GFMRV_OK, rv);

    /* Create all particles groups */
    rv = gfmGroup_getNew(&(pGame->pParticles));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_getNew(&(pGame->pCollideableParticles));
    ASSERT(rv == GFMRV_OK, rv);
    /* TODO Initialize all groups */

#ifdef DEBUG
    rv = gfm_initFPSCounter(pGame->pCtx, pAssets->pSset8x8, 0/*firstTile*/);
    ASSERT(rv == GFMRV_OK, rv);
#endif /* DEBUG */

    /* TODO Set the main state as the intro */
    pGame->nextState = state_game;

    rv = gfmQuadtree_getNew(&(pGame->pQt));
    ASSERT(rv == GFMRV_OK, rv);

    rv = main_loop();
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    if (pGame) {
        /* TODO Free everything else */
        gfmQuadtree_free(&(pGame->pQt));
        gfmGroup_free(&(pGame->pParticles));
        gfmGroup_free(&(pGame->pCollideableParticles));
        gfm_free(&(pGame->pCtx));
    }
    if (pGame) {
        free(pGame);
    }
    if (pAssets) {
        free(pAssets);
    }
    if (pButtons) {
        free(pButtons);
    }

    return rv;
}

