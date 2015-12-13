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

static int grp_anim_data[] = {
              /*   len|fps|loop|data... */
/* BULLET    */    16 , 12, 0  , 68,69,70,69,70,69,70,69,70,69,70,69,70,69,70,69,
/* PELLET1   */     1 , 0 , 0  , 71,
/* PELLET2   */     1 , 0 , 0  , 72,
/* EXPLOSION */     8 , 16, 0  , 73,74,75,76,77,77,77,77
};
static int grp_anim_dataLen = sizeof(grp_anim_data) / sizeof(int);

static gfmRV main_updateButtons() {
    gfmRV rv;

    rv = gfm_getKeyState(&(pButtons->left_legBack.state),
            &(pButtons->left_legBack.num), pGame->pCtx,
            pButtons->left_legBack.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->left_leg.state), &(pButtons->left_leg.num),
            pGame->pCtx, pButtons->left_leg.handle);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_getKeyState(&(pButtons->right_legBack.state),
            &(pButtons->right_legBack.num), pGame->pCtx,
            pButtons->right_legBack.handle);
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
    rv = gfm_getKeyState(&(pButtons->reset.state), &(pButtons->reset.num),
            pGame->pCtx, pButtons->reset.handle);
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

    if ((pButtons->reset.state & gfmInput_justReleased) ==
            gfmInput_justReleased) {
        if (pGame->curState == state_game) {
            pGame->nextState = state_game;
        }
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
    gfmSave *pSave;

    /* Clean the game, so it's properly release on error */
    pGame = 0;
    pAssets = 0;
    pButtons = 0;
    pState = 0;
    pSave = 0;

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

    /* Erase the save file */
    rv = gfmSave_getNew(&pSave);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSave_bindStatic(pSave, pGame->pCtx, SAVE_FILE);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSave_erase(pSave);
    ASSERT(rv == GFMRV_OK, rv);
    gfmSave_free(&pSave);
    pSave = 0;

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
    rv = gfm_addVirtualKey(&(pButtons->left_legBack.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->left_start.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->right_leg.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->right_legBack.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->right_start.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->fullscreen.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->quit.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->drawQt.handle), pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_addVirtualKey(&(pButtons->reset.handle), pGame->pCtx);
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
    rv = gfm_bindInput(pGame->pCtx, pButtons->left_legBack.handle, gfmKey_d);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->left_legBack.handle,
            gfmController_l2, 0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->left_legBack.handle,
            gfmController_l2, 1/*port*/);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_bindInput(pGame->pCtx, pButtons->right_leg.handle, gfmKey_j);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_leg.handle,
            gfmController_r1, 0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_leg.handle,
            gfmController_r1, 1/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindInput(pGame->pCtx, pButtons->right_legBack.handle, gfmKey_k);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_legBack.handle,
            gfmController_r2, 0/*port*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_bindGamepadInput(pGame->pCtx, pButtons->right_legBack.handle,
            gfmController_r2, 1/*port*/);

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

    rv = gfm_bindInput(pGame->pCtx, pButtons->reset.handle, gfmKey_r);
    ASSERT(rv == GFMRV_OK, rv);
    /* TODO Bind to gamepad button */

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
    rv = gfmGroup_setDefType(pGame->pParticles, BULLET);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefSpriteset(pGame->pParticles, pAssets->pSset8x8);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefAnimData(pGame->pParticles, grp_anim_data,
            grp_anim_dataLen);
    rv = gfmGroup_setDefDimensions(pGame->pParticles, 2, 2, -3, -3);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefVelocity(pGame->pParticles, 0, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefAcceleration(pGame->pParticles, 0, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDeathOnLeave(pGame->pParticles, 1);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDeathOnTime(pGame->pParticles, PARTICLE_TTL);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDrawOrder(pGame->pParticles, gfmDrawOrder_linear);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_preCache(pGame->pParticles, NUM_PARTICLES, NUM_PARTICLES);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmGroup_getNew(&(pGame->pBullets));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefType(pGame->pBullets, BULLET);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefSpriteset(pGame->pBullets, pAssets->pSset8x8);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefAnimData(pGame->pBullets, grp_anim_data,
            grp_anim_dataLen);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefDimensions(pGame->pBullets, 2, 2, -3, -3);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefVelocity(pGame->pBullets, 0, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefAcceleration(pGame->pBullets, 0, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDeathOnLeave(pGame->pBullets, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDeathOnTime(pGame->pBullets, PARTICLE_TTL);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDrawOrder(pGame->pBullets, gfmDrawOrder_linear);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_preCache(pGame->pBullets, NUM_PARTICLES, NUM_PARTICLES);
    ASSERT(rv == GFMRV_OK, rv);
#if 0
    /* This would be a terrible idea (so, I really want to enable it :D) */
    rv = gfmGroup_setCollisionQuality(pGame->pBullets,
            gfmCollisionQuality_collideEverything);
#endif
    rv = gfmGroup_setCollisionQuality(pGame->pBullets,
            gfmCollisionQuality_visibleOnly);

    rv = gfmGroup_getNew(&(pGame->pProps));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefType(pGame->pProps, PROP);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefSpriteset(pGame->pProps, pAssets->pSset8x8);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefAnimData(pGame->pProps, grp_anim_data,
            grp_anim_dataLen);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefDimensions(pGame->pProps, 2, 2, -3, -3);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefVelocity(pGame->pProps, 0, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefAcceleration(pGame->pProps, 0, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDeathOnLeave(pGame->pProps, 0);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDeathOnTime(pGame->pProps, PARTICLE_TTL);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDrawOrder(pGame->pProps, gfmDrawOrder_linear);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_preCache(pGame->pProps, NUM_PARTICLES, NUM_PARTICLES);
    ASSERT(rv == GFMRV_OK, rv);
#if 0
    /* This would be a terrible idea (so, I really want to enable it :D) */
    rv = gfmGroup_setCollisionQuality(pGame->pProps,
            gfmCollisionQuality_collideEverything);
#endif
    rv = gfmGroup_setCollisionQuality(pGame->pProps,
            gfmCollisionQuality_visibleOnly);
    ASSERT(rv == GFMRV_OK, rv);

#ifdef DEBUG
    rv = gfm_initFPSCounter(pGame->pCtx, pAssets->pSset8x8, 0/*firstTile*/);
    ASSERT(rv == GFMRV_OK, rv);
#endif /* DEBUG */

    /* TODO Set the main state as the intro */
    pGame->nextState = state_game;
    pGame->run = 1;

    rv = gfmQuadtree_getNew(&(pGame->pQt));
    ASSERT(rv == GFMRV_OK, rv);

    rv = main_loop();
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    gfmSave_free(&pSave);
    if (pGame) {
        /* TODO Free everything else */
        gfmQuadtree_free(&(pGame->pQt));
        gfmGroup_free(&(pGame->pParticles));
        gfmGroup_free(&(pGame->pBullets));
        gfmGroup_free(&(pGame->pProps));
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

