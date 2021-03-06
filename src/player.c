/**
 * The player
 *
 * @file src/player.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSave.h>
#include <GFraMe/gfmSprite.h>

#include <ld34/collide.h>
#include <ld34/game.h>
#include <ld34/player.h>

#include <stdlib.h>
#include <string.h>

struct stPlayer {
    gfmObject *lower_pTorso;
    gfmObject *upper_pTorso;
    gfmObject *left_pLeg;
    gfmObject *right_pLeg;
    gfmSprite *pRenderSpr;
    int left_raisingTime;
    int left_elapsedSinceStep;
    int right_raisingTime;
    int right_elapsedSinceStep;
    int didJump;
};

/**
 * Alloc and initialize the player
 *
 * @param  [out]ppPlayer The player
 * @param  [ in]x        Initial horizontal position (torso's upper left)
 * @param  [ in]y        Initial vertical position (torso's upper left)
 */
gfmRV player_init(player **ppPlayer, int x, int y) {
    gfmRV rv;
    gfmSave *pSave;
    player *pPlayer;
    int _x, _y;

    pSave = 0;

    rv = gfmSave_getNew(&pSave);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSave_bindStatic(pSave, pGame->pCtx, SAVE_FILE);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSave_readStatic(&_x, pSave, "checkpoint_x");
    if (rv != GFMRV_OK) {
        _x = -1;
    }
    rv = gfmSave_readStatic(&_y, pSave, "checkpoint_y");
    if (rv != GFMRV_OK) {
        _y = -1;
    }
    gfmSave_free(&pSave);
    pSave = 0;

    if (_x != -1 && _y != -1) {
        x = _x;
        y = _y;
    }

    pPlayer = (player*)malloc(sizeof(player));
    ASSERT(pPlayer, GFMRV_ALLOC_FAILED);
    memset(pPlayer, 0x0, sizeof(player));

    rv = gfmObject_getNew(&(pPlayer->upper_pTorso));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getNew(&(pPlayer->lower_pTorso));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getNew(&(pPlayer->right_pLeg));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getNew(&(pPlayer->left_pLeg));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_getNew(&(pPlayer->pRenderSpr));
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_init(pPlayer->upper_pTorso, x, y, 10, 14, pPlayer,
            PL_UPPER);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_init(pPlayer->lower_pTorso, x, y+13, 10, 14, pPlayer,
            PL_LOWER);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_init(pPlayer->left_pLeg, x+4, y+30, 10, 14, pPlayer,
            PL_LEFT_LEG);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_init(pPlayer->right_pLeg, x+1, y+30, 10, 14, pPlayer,
            PL_RIGHT_LEG);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_init(pPlayer->pRenderSpr, 0, 0, 2, 2, pAssets->pSset8x8, 0,
            0, 0, PL_UPPER);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_setAcceleration(pPlayer->left_pLeg, 0, GRAV);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_setAcceleration(pPlayer->right_pLeg, 0, GRAV);
    ASSERT(rv == GFMRV_OK, rv);

    pPlayer->didJump = 1;

    *ppPlayer = pPlayer;
    rv = GFMRV_OK;
__ret:
    gfmSave_free(&pSave);

    return rv;
}

/**
 * Release the player
 *
 * @param  [ in]ppPlayer The player
 */
void player_clean(player **ppPlayer) {
    if (!ppPlayer || !(*ppPlayer)) {
        return;
    }

    gfmSprite_free(&((*ppPlayer)->pRenderSpr));
    gfmObject_free(&((*ppPlayer)->upper_pTorso));
    gfmObject_free(&((*ppPlayer)->lower_pTorso));
    gfmObject_free(&((*ppPlayer)->left_pLeg));
    gfmObject_free(&((*ppPlayer)->right_pLeg));

    free(*ppPlayer);
    *ppPlayer = 0;
}

static inline gfmRV player_moveLeg(gfmObject *pObj, button *pBt, int *pTime,
        int didJump, double vx) {
    gfmRV rv;

    if ((pBt->state & gfmInput_justPressed) == gfmInput_justPressed) {
        gfmCollision dir;
        int elapsed;

        rv = gfm_getElapsedTime(&elapsed, pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmObject_getCollision(&dir, pObj);
        ASSERT(rv == GFMRV_OK, rv);

        if (dir & gfmCollision_down) {
            rv = gfmObject_setVelocity(pObj, vx, PL_VY);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else {
            *pTime = PL_HOLD_T;
        }

        *pTime += elapsed;
    }
    else if ((pBt->state & gfmInput_pressed) == gfmInput_pressed && *pTime < PL_HOLD_T) {
        int elapsed;
        double _vx, _vy;

        rv = gfm_getElapsedTime(&elapsed, pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);

        _vx = vx + vx * 0.1 * (PL_HOLD_T - *pTime) / (double)PL_HOLD_T;
        _vy = PL_VY  * 0.8 * (PL_HOLD_T - *pTime) / (double)PL_HOLD_T;

        rv = gfmObject_setVelocity(pObj, _vx, _vy);
        ASSERT(rv == GFMRV_OK, rv);

        *pTime += elapsed;
    }
    else if ((pBt->state & gfmInput_justReleased) == gfmInput_justReleased &&
            !didJump) {
        double vy;

        *pTime = 0;

        rv = gfmObject_getVerticalVelocity(&vy, pObj);
        ASSERT(rv == GFMRV_OK, rv);

        if (vy < 0.0) {
            vy = 0.0;
        }
        rv = gfmObject_setVelocity(pObj, 0.0, 0.0);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if ((pBt->state & gfmInput_released) == gfmInput_released) {
        *pTime = 0;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the player's physics and handle inputs
 *
 * NOTE: Must be called before colliding!
 *
 * @param  [ in]pPlayer The player
 */
gfmRV player_preUpdate(player *pPlayer) {
#if 0
    double vx;
#endif
    gfmRV rv;
    int l_x, r_x;

    rv = player_moveLeg(pPlayer->left_pLeg, &(pButtons->left_leg),
            &(pPlayer->left_raisingTime), pPlayer->didJump, PL_VX);
    ASSERT(rv == GFMRV_OK, rv);
    rv = player_moveLeg(pPlayer->right_pLeg, &(pButtons->right_leg),
            &(pPlayer->right_raisingTime), pPlayer->didJump, PL_VX);
    ASSERT(rv == GFMRV_OK, rv);
#if 0
    rv = gfmObject_getHorizontalVelocity(&vx, pPlayer->left_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    if (vx == 0 || vx > 0) {
        rv = player_moveLeg(pPlayer->left_pLeg, &(pButtons->left_leg),
                &(pPlayer->left_raisingTime), pPlayer->didJump, PL_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }
    if (vx == 0 || vx < 0) {
        rv = player_moveLeg(pPlayer->left_pLeg, &(pButtons->left_legBack),
                &(pPlayer->left_raisingTime), pPlayer->didJump, -PL_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = gfmObject_getHorizontalVelocity(&vx, pPlayer->right_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    if (vx == 0 || vx > 0) {
        rv = player_moveLeg(pPlayer->right_pLeg, &(pButtons->right_leg),
                &(pPlayer->right_raisingTime), pPlayer->didJump, PL_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }
    if (vx == 0 || vx < 0) {
        rv = player_moveLeg(pPlayer->right_pLeg, &(pButtons->right_legBack),
                &(pPlayer->right_raisingTime), pPlayer->didJump, -PL_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }
#endif

    /* Check that the maximum distance wasn't reached */
    rv = gfmObject_getHorizontalPosition(&l_x, pPlayer->left_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getHorizontalPosition(&r_x, pPlayer->right_pLeg);

    ASSERT(rv == GFMRV_OK, rv);
    if (abs(l_x - r_x) >= PL_MAX_DIST && !pPlayer->didJump) {
        if (l_x > r_x) {
            rv = gfmObject_setHorizontalVelocity(pPlayer->left_pLeg, 0.0);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else {
            rv = gfmObject_setHorizontalVelocity(pPlayer->right_pLeg, 0.0);
            ASSERT(rv == GFMRV_OK, rv);
        }
    }


    rv = gfmObject_update(pPlayer->upper_pTorso, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_update(pPlayer->lower_pTorso, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_update(pPlayer->left_pLeg, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_update(pPlayer->right_pLeg, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    /* Collide left leg */
    rv = gfmQuadtree_collideObject(pGame->pQt, pPlayer->left_pLeg);
    ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        rv = collide_run();
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Collide right leg */
    rv = gfmQuadtree_collideObject(pGame->pQt, pPlayer->right_pLeg);
    ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        rv = collide_run();
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Collide upper torso */
    rv = gfmQuadtree_collideObject(pGame->pQt, pPlayer->upper_pTorso);
    ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        rv = collide_run();
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Collide lower torso */
    rv = gfmQuadtree_collideObject(pGame->pQt, pPlayer->lower_pTorso);
    ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        rv = collide_run();
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the player's animation and anything related to collision (e.g.,
 * jumping)
 *
 * NOTE: Must be called after colliding!
 *
 * @param  [ in]pPlayer The player
 */
gfmRV player_postUpdate(player *pPlayer) {
    gfmRV rv;
    gfmCamera *pCam;
    gfmCollision l_cur, l_last, r_cur, r_last;
    int elapsed, l_x, l_y, r_x, r_y, t_x, t_y;

    rv = gfm_getElapsedTime(&elapsed, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_getCollision(&l_cur, pPlayer->left_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getLastCollision(&l_last, pPlayer->left_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getCollision(&r_cur, pPlayer->right_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getLastCollision(&r_last, pPlayer->right_pLeg);
    ASSERT(rv == GFMRV_OK, rv);

    if (!(l_last & gfmCollision_down) && (l_cur & gfmCollision_down)) {
        pPlayer->left_elapsedSinceStep = 0;

        rv = gfm_playAudio(0, pGame->pCtx, pAssets->sfxLeftStep, 0.3);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        pPlayer->left_elapsedSinceStep += elapsed;
    }

    if (!(r_last & gfmCollision_down) && (r_cur & gfmCollision_down)) {
        pPlayer->right_elapsedSinceStep = 0;

        rv = gfm_playAudio(0, pGame->pCtx, pAssets->sfxRightStep, 0.3);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        pPlayer->right_elapsedSinceStep += elapsed;
    }

    if (pPlayer->left_elapsedSinceStep <= PL_JUMP_T &&
            pPlayer->right_elapsedSinceStep <= PL_JUMP_T && !pPlayer->didJump) {
        /* Jump */
        rv = gfmObject_setVelocity(pPlayer->left_pLeg, PL_JUMP_VX, PL_JUMP_VY);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmObject_setVelocity(pPlayer->right_pLeg, PL_JUMP_VX, PL_JUMP_VY);
        ASSERT(rv == GFMRV_OK, rv);

        pPlayer->didJump = 1;
    }
    else if ((l_cur & gfmCollision_down) && (r_cur & gfmCollision_down)) {
        pPlayer->didJump = 0;
        pPlayer->left_elapsedSinceStep = PL_JUMP_T;
        pPlayer->right_elapsedSinceStep = PL_JUMP_T;
    }

    /* Position the torso */
    rv = gfmObject_getPosition(&l_x, &l_y, pPlayer->left_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&r_x, &r_y, pPlayer->right_pLeg);
    ASSERT(rv == GFMRV_OK, rv);

    /* Use left most as base */
    if (l_x < r_x) {
        t_x = l_x - 3;
    }
    else {
        t_x = r_x - 3;
    }
    t_x += abs(l_x - r_x) / 2;

    /* Use bottom most as base */
    if (l_y > r_y) {
        t_y = l_y - 4;
    }
    else {
        t_y = r_y - 4;
    }

    /* Make it a little smaller when the feets are apart */
    t_y += abs(l_x - r_x) / ((float)PL_MAX_DIST / 4.0);

    /* Move slightly, if jumping */
    if (pPlayer->didJump) {
        double vy;

        rv = gfmObject_getVerticalVelocity(&vy, pPlayer->left_pLeg);
        ASSERT(rv == GFMRV_OK, rv);

        if (vy > 0) {
            t_y += 6 * (PL_JUMP_VY + vy) / (float)PL_JUMP_VY;
        }
        else if (vy < 0) {
            t_y += 6 * (PL_JUMP_VY - vy) / (float)PL_JUMP_VY;
        }
    }
    else {
        t_y -= abs(l_y - r_y) / 4;
    }

    rv = gfmObject_setPosition(pPlayer->upper_pTorso, t_x, t_y - 26);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_setPosition(pPlayer->lower_pTorso, t_x, t_y - 13);
    ASSERT(rv == GFMRV_OK, rv);

    pCam = 0;
    rv = gfm_getCamera(&pCam, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getCenter(&t_x, &t_y, pPlayer->lower_pTorso);
    ASSERT(rv == GFMRV_OK, rv);
    gfmCamera_centerAtPoint(pCam, t_x, t_y);
#if 0
    rv = gfmCamera_centerAtPoint(pCam, t_x, t_y);
    ASSERT(rv == GFMRV_OK, rv);
#endif

    rv = GFMRV_OK;
__ret:
    return rv;
}

static inline gfmRV player_draw_module(gfmSprite *pSpr, gfmSpriteset *pSset,
        int x, int y, int ox, int oy, int w, int h, int tile) {
    gfmRV rv;

    rv = gfmSprite_setDimensions(pSpr, w, h);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_setPosition(pSpr, x, y);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_setOffset(pSpr, ox, oy);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_setSpriteset(pSpr, pSset);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_setFrame(pSpr, tile);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSprite_draw(pSpr, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Render the player
 *
 * @param  [ in]pPlayer The player
 */
gfmRV player_draw(player *pPlayer) {
    gfmRV rv;
    int ut_x, ut_y, lt_x, lt_y, ll_x, ll_y, rl_x, rl_y, lk_x, lk_y, rk_x, rk_y;

    rv = gfmObject_getPosition(&ut_x, &ut_y, pPlayer->upper_pTorso);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&lt_x, &lt_y, pPlayer->lower_pTorso);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&ll_x, &ll_y, pPlayer->left_pLeg);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&rl_x, &rl_y, pPlayer->right_pLeg);
    ASSERT(rv == GFMRV_OK, rv);


    /* Left leg */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset16x16, ll_x,
            ll_y, -2, -2, 10, 14, 33);
    ASSERT(rv == GFMRV_OK, rv);
    /* Left knee */
    lk_x = (lt_x + 2) * 0.75 + (ll_x + 1) * 0.25;
    lk_y = (lt_y + 13) * 0.75+ (ll_y - 4) * 0.25;
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset8x8, lk_x, lk_y,
            0, 0, 8, 8, 66);
    ASSERT(rv == GFMRV_OK, rv);
    /* Left hip */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset8x8, lt_x+4,
            lt_y+10, 0, 0, 8, 8, 67);
    ASSERT(rv == GFMRV_OK, rv);

    /* Lower torso */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset32x16, lt_x,
            lt_y, -12, -2, 10, 14, 18);
    ASSERT(rv == GFMRV_OK, rv);
    /* Upper torso */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset32x16, ut_x,
            ut_y, -12, -2, 10, 14, 17);
    ASSERT(rv == GFMRV_OK, rv);

    /* Right hip */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset8x8, lt_x,
            lt_y+10, 0, 0, 8, 8, 65);
    ASSERT(rv == GFMRV_OK, rv);
    /* Right knee */
    rk_x = (lt_x + 2) * 0.75 + (rl_x + 1) * 0.25;
    rk_y = (lt_y + 13) * 0.75 + (rl_y - 4) * 0.25;
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset8x8, rk_x, rk_y,
            0, 0, 8, 8, 64);
    ASSERT(rv == GFMRV_OK, rv);
    /* Right leg */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset16x16, rl_x,
            rl_y, -2, -2, 10, 14, 32);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Collide the player's limbs against the floor
 *
 * @param  [ in]pPlayer The colliding player
 * @param  [ in]type    Which limb collided
 * @param  [ in]pFloor  The floor
 */
gfmRV player_collideLimbFloor(player *pPlayer, int type, gfmObject *pFloor) {
    gfmObject *pObj;
    gfmRV rv;

    switch (type) {
        case PL_LEFT_LEG: {
            pObj = pPlayer->left_pLeg;
        } break;
        case PL_RIGHT_LEG: {
            pObj = pPlayer->right_pLeg;
        } break;
        default: { return GFMRV_OK; }
    }

    rv = gfmObject_collide(pObj, pFloor);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_TRUE) {
        gfmCollision dir;

        rv = gfmObject_getCurrentCollision(&dir, pObj);
        ASSERT(rv == GFMRV_OK, rv);

        /* Don't stop if was colliding horizontally */
        if (dir == gfmCollision_down) {
            rv = gfmObject_setVelocity(pObj, 0.0, 0.0);
            ASSERT(rv == GFMRV_OK, rv);
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

