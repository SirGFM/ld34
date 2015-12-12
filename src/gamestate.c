/**
 * The main state of the game
 *
 * @file src/gamestate.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

#include <ld34/game.h>
#include <ld34/gamestate.h>

#include <stdlib.h>
#include <string.h>

struct stGamestate {
    int bla;
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

    /* TODO Update the game */

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

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free all game state related resources
 *
 * NOTE: pState will be overwritten!
 */
gfmRV gamestate_clean() {
    gamestate *pGamestate;
    gfmRV rv;

    pGamestate = (gamestate*)pState;

    /* TODO Release everything alloc'ed for the gamestate */

    free(pState);
    pState = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

