/**
 * The main state of the game
 *
 * @file src/gamestate.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmQuadtree.h>

#include <ld34/collide.h>
#include <ld34/game.h>
#include <ld34/gamestate.h>
#include <ld34/player.h>

#include <stdlib.h>
#include <string.h>

struct stGamestate {
    player *pPlayer;
    gfmObject *pObject;
};
typedef struct stGamestate gamestate;

/**
 * Initialize the game state
 *
 * NOTE: pState will be overwritten!
 */
gfmRV gamestate_init() {
    gamestate *pGamestate;
    gfmRV rv;

    pGamestate = (gamestate*)malloc(sizeof(gamestate));
    ASSERT(pGamestate, GFMRV_ALLOC_FAILED);
    memset(pGamestate, 0x0, sizeof(gamestate));

    /* TODO Initialize everything */
    rv = player_init(&(pGamestate->pPlayer), 64, 64);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_getNew(&(pGamestate->pObject));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_init(pGamestate->pObject, 0, 128, 320, 240, 0, FLOOR);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_setFixed(pGamestate->pObject);
    ASSERT(rv == GFMRV_OK, rv);

    pGame->width = 340;
    pGame->height = 260;

    pState = pGamestate;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/** Update the current state as a gamestate */
gfmRV gamestate_update() {
    gamestate *pGamestate;
    gfmRV rv;

    pGamestate = (gamestate*)pState;

    rv = gfmQuadtree_initRoot(pGame->pQt, -16, -16, pGame->width, pGame->height,
            6 /* maxDepth */, 10 /* maxNodes */);
    ASSERT(rv == GFMRV_OK, rv);

    /* TODO Update the game */
    rv = gfmObject_update(pGamestate->pObject, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmQuadtree_populateObject(pGame->pQt, pGamestate->pObject);
    ASSERT(rv == GFMRV_OK, rv);

    rv = player_preUpdate(pGamestate->pPlayer);
    ASSERT(rv == GFMRV_OK, rv);

    /* After everything collided */
    rv = player_postUpdate(pGamestate->pPlayer);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/** Render the current state as a gamestate */
gfmRV gamestate_draw() {
    gamestate *pGamestate;
    gfmRV rv;

    pGamestate = (gamestate*)pState;

    /* TODO Render the game */
    rv = player_draw(pGamestate->pPlayer);
    ASSERT(rv == GFMRV_OK, rv);

#ifdef DEBUG
    if (pGame->drawQt) {
        rv = gfmQuadtree_drawBounds(pGame->pQt, pGame->pCtx, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }
#endif

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free all game state related resources
 *
 * NOTE: pState will be overwritten!
 */
void gamestate_clean() {
    gamestate *pGamestate;

    pGamestate = (gamestate*)pState;

    /* TODO Release everything alloc'ed for the gamestate */
    gfmObject_free(&(pGamestate->pObject));
    player_clean(&(pGamestate->pPlayer));

    free(pState);
    pState = 0;
}

