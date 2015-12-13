/**
 * The main state of the game
 *
 * @file src/gamestate.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmTilemap.h>

#include <ld34/collide.h>
#include <ld34/enemy.h>
#include <ld34/game.h>
#include <ld34/gamestate.h>
#include <ld34/player.h>

#include <stdlib.h>
#include <string.h>

#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
#  include <signal.h>
#endif

gfmGenArr_define(enemy);

static const char *pTmDict[] = {
    "floor"
};
static const int tmDictType[] = {
    FLOOR
};
static const int tmDictLen = sizeof(tmDictType) / sizeof(int);

struct stGamestate {
    player *pPlayer;
    gfmTilemap *pTm;
    gfmGenArr_var(enemy, pEnes);
};
typedef struct stGamestate gamestate;

/**
 * Initialize the game state
 *
 * NOTE: pState will be overwritten!
 */
gfmRV gamestate_init() {
    gamestate *pGamestate;
    gfmCamera *pCam;
    gfmParser *pParser;
    gfmRV rv;

    pParser = 0;

    pGamestate = (gamestate*)malloc(sizeof(gamestate));
    ASSERT(pGamestate, GFMRV_ALLOC_FAILED);
    memset(pGamestate, 0x0, sizeof(gamestate));

    /* TODO Initialize everything */
    rv = player_init(&(pGamestate->pPlayer), 64, 64);
    ASSERT(rv == GFMRV_OK, rv);

    /* Load the map */
    rv = gfmTilemap_getNew(&(pGamestate->pTm));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmTilemap_init(pGamestate->pTm, pAssets->pSset8x8, 100/*widthInTiles*/,
            40/*heightInTiles*/, -1/*defTile*/);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmTilemap_loadf(pGamestate->pTm, pGame->pCtx, "map_tile.gfm", 12, (char**)pTmDict,
            (int*)tmDictType, tmDictLen);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmTilemap_getDimension(&(pGame->width), &(pGame->height),
            pGamestate->pTm);
    ASSERT(rv == GFMRV_OK, rv);

    /* Parse and spawn stuff */
    rv = gfmParser_getNew(&pParser);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmParser_initStatic(pParser, pGame->pCtx, "map_obj.gfm");
    ASSERT(rv == GFMRV_OK, rv);

    while (1) {
        gfmParserType type;

        rv = gfmParser_parseNext(pParser);
        ASSERT(rv == GFMRV_OK || rv == GFMRV_PARSER_FINISHED, rv);
        if (rv == GFMRV_PARSER_FINISHED) {
            break;
        }

        rv = gfmParser_getType(&type, pParser);
        ASSERT(rv == GFMRV_OK, rv);

        if (type == gfmParserType_area) {
            /* TODO Handle areas later, if needed */
#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
            raise(SIGINT);
#endif
            ASSERT(0, GFMRV_INTERNAL_ERROR);
        }
        else {
            char *pType;

            rv = gfmParser_getIngameType(&pType, pParser);
            ASSERT(rv == GFMRV_OK, rv);

            if (strcmp("lil_tank", pType) == 0) {
                enemy *pEnemy;

                gfmGenArr_getNextRef(enemy, pGamestate->pEnes, 1, pEnemy, enemy_getNew);
                gfmGenArr_push(pGamestate->pEnes);

                rv = enemy_init(pEnemy, pParser, LIL_TANK);
                ASSERT(rv == GFMRV_OK, rv);
            }
            else {
                /* Got something that still isn't handled */
#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
                raise(SIGINT);
#endif
            ASSERT(0, GFMRV_INTERNAL_ERROR);
            }
        }
    }

    pCam = 0;
    rv = gfm_getCamera(&pCam, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmCamera_setWorldDimensions(pCam, pGame->width, pGame->height);
    ASSERT(rv == GFMRV_OK, rv);

    pGame->width += 16;
    pGame->height += 16;

    rv = gfmCamera_setDeadzone(pCam, 16/*x*/, 0/*y*/, 96/*w*/, 240);
    ASSERT(rv == GFMRV_OK, rv);

    pState = pGamestate;
    rv = GFMRV_OK;
__ret:
    if (pParser) {
        gfmParser_free(&pParser);
    }

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
    rv = gfmTilemap_update(pGamestate->pTm, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmQuadtree_populateTilemap(pGame->pQt, pGamestate->pTm);
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
    rv = gfmTilemap_draw(pGamestate->pTm, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
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
    gfmTilemap_free(&(pGamestate->pTm));
    player_clean(&(pGamestate->pPlayer));
    gfmGenArr_clean(pGamestate->pEnes, enemy_clean);

    free(pState);
    pState = 0;
}

