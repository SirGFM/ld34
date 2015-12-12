/**
 * The player
 *
 * @file src/player.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

#include <ld34/collide.h>
#include <ld34/game.h>
#include <ld34/player.h>

#include <stdlib.h>
#include <string.h>

struct stPlayer {
    gfmObject *pLowerTorso;
    gfmObject *pUpperTorso;
    gfmObject *pLeftLeg;
    gfmObject *pRightLeg;
    gfmSprite *pRenderSpr;
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
    player *pPlayer;

    pPlayer = (player*)malloc(sizeof(player));
    ASSERT(pPlayer, GFMRV_ALLOC_FAILED);
    memset(pPlayer, 0x0, sizeof(player));

    rv = gfmObject_getNew(&(pPlayer->pUpperTorso));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getNew(&(pPlayer->pLowerTorso));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getNew(&(pPlayer->pRightLeg));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getNew(&(pPlayer->pLeftLeg));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_getNew(&(pPlayer->pRenderSpr));
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_init(pPlayer->pUpperTorso, x, y, 10, 14, pPlayer,
            PL_UPPER);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_init(pPlayer->pLowerTorso, x, y+13, 10, 14, pPlayer,
            PL_LOWER);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_init(pPlayer->pLeftLeg, x+4, y+30, 10, 14, pPlayer,
            PL_LEFT_LEG);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_init(pPlayer->pRightLeg, x+1, y+30, 10, 14, pPlayer,
            PL_RIGHT_LEG);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_init(pPlayer->pRenderSpr, 0, 0, 2, 2, pAssets->pSset8x8, 0,
            0, 0, PL_UPPER);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_setAcceleration(pPlayer->pLeftLeg, 0, 100);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_setAcceleration(pPlayer->pRightLeg, 0, 100);
    ASSERT(rv == GFMRV_OK, rv);

    *ppPlayer = pPlayer;
    rv = GFMRV_OK;
__ret:
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
    gfmObject_free(&((*ppPlayer)->pUpperTorso));
    gfmObject_free(&((*ppPlayer)->pLowerTorso));
    gfmObject_free(&((*ppPlayer)->pLeftLeg));
    gfmObject_free(&((*ppPlayer)->pRightLeg));

    free(*ppPlayer);
    *ppPlayer = 0;
}

/**
 * Update the player's physics and handle inputs
 *
 * NOTE: Must be called before colliding!
 *
 * @param  [ in]pPlayer The player
 */
gfmRV player_preUpdate(player *pPlayer) {
    gfmRV rv;

    if (pButtons->left_leg.state & gfmInput_pressed) {
    }

    rv = gfmObject_update(pPlayer->pUpperTorso, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_update(pPlayer->pLowerTorso, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_update(pPlayer->pLeftLeg, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_update(pPlayer->pRightLeg, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    /* Collide left leg */
    rv = gfmQuadtree_collideObject(pGame->pQt, pPlayer->pLeftLeg);
    ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        rv = collide_run();
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Collide right leg */
    rv = gfmQuadtree_collideObject(pGame->pQt, pPlayer->pRightLeg);
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
    int ut_x, ut_y, lt_x, lt_y, ll_x, ll_y, rl_x, rl_y;

    rv = gfmObject_getPosition(&ut_x, &ut_y, pPlayer->pUpperTorso);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&lt_x, &lt_y, pPlayer->pLowerTorso);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&ll_x, &ll_y, pPlayer->pLeftLeg);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&rl_x, &rl_y, pPlayer->pRightLeg);
    ASSERT(rv == GFMRV_OK, rv);


    /* Left hip */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset8x8, lt_x+4,
            lt_y+10, 0, 0, 8, 8, 67);
    ASSERT(rv == GFMRV_OK, rv);
    /* Left knee (TODO Fix position) */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset8x8, lt_x+6,
            lt_y+13, 0, 0, 8, 8, 66);
    ASSERT(rv == GFMRV_OK, rv);
    /* Left leg */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset16x16, ll_x,
            ll_y, -2, -2, 10, 14, 33);
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
    /* Right knee (TODO Fix position) */
    rv = player_draw_module(pPlayer->pRenderSpr, pAssets->pSset8x8, lt_x+2,
            lt_y+13, 0, 0, 8, 8, 64);
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
            pObj = pPlayer->pLeftLeg;
        } break;
        case PL_RIGHT_LEG: {
            pObj = pPlayer->pRightLeg;
        } break;
        default: { return GFMRV_OK; }
    }

    rv = gfmObject_collide(pObj, pFloor);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_TRUE) {
        rv = gfmObject_setVerticalVelocity(pObj, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

