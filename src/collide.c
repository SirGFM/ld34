/**
 * Handle collisions
 *
 * @file src/collide.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSpriteset.h>

#include <ld34/collide.h>
#include <ld34/enemy.h>
#include <ld34/game.h>
#include <ld34/player.h>

#include <stdlib.h>

#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
#  include <signal.h>
#endif

static inline gfmRV collide_getSubtype(void **ppObj, int *pType, gfmObject *pObj) {
    gfmRV rv;

    rv = gfmObject_getChild(ppObj, pType, pObj);
    ASSERT(rv == GFMRV_OK, rv);

    if (*pType == gfmType_sprite) {
        gfmSprite *pSpr;

        pSpr = *((gfmSprite**)ppObj);
        rv = gfmSprite_getChild(ppObj, pType, pSpr);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/** Continue the currently executing collision */
gfmRV collide_run() {
    gfmRV rv;

    rv = GFMRV_QUADTREE_OVERLAPED;
    while (rv != GFMRV_QUADTREE_DONE) {
        gfmObject *pObj1, *pObj2;
        void *pChild1, *pChild2;
        int type1, type2, orType;

        rv = gfmQuadtree_getOverlaping(&pObj1, &pObj2, pGame->pQt);
        ASSERT(rv == GFMRV_OK, rv);

        rv = collide_getSubtype(&pChild1, &type1, pObj1);
        ASSERT(rv == GFMRV_OK, rv);
        rv = collide_getSubtype(&pChild2, &type2, pObj2);
        ASSERT(rv == GFMRV_OK, rv);

        orType = type1 | (type2 << 16);

        rv = GFMRV_OK;
        switch (orType) {
            case PL_UPPER | (PL_UPPER << 16):
            case PL_UPPER | (PL_LOWER << 16):
            case PL_UPPER | (PL_LEFT_LEG << 16):
            case PL_UPPER | (PL_RIGHT_LEG << 16):
            case PL_UPPER | (FLOOR << 16):
            case PL_UPPER | (PROP << 16):
            case PL_LOWER | (PL_UPPER << 16):
            case PL_LOWER | (PL_LOWER << 16):
            case PL_LOWER | (PL_LEFT_LEG << 16):
            case PL_LOWER | (PL_RIGHT_LEG << 16):
            case PL_LOWER | (FLOOR << 16):
            case PL_LOWER | (PROP << 16):
            case PL_LEFT_LEG | (PL_UPPER << 16):
            case PL_LEFT_LEG | (PL_LOWER << 16):
            case PL_LEFT_LEG | (PL_LEFT_LEG << 16):
            case PL_LEFT_LEG | (PL_RIGHT_LEG << 16):
            case PL_RIGHT_LEG | (PL_UPPER << 16):
            case PL_RIGHT_LEG | (PL_LOWER << 16):
            case PL_RIGHT_LEG | (PL_LEFT_LEG << 16):
            case PL_RIGHT_LEG | (PL_RIGHT_LEG << 16):
            case FLOOR | (PL_UPPER << 16):
            case FLOOR | (PL_LOWER << 16):
            case BULLET | (LIL_TANK << 16):
            case BULLET | (BULLET << 16):
            case BULLET | (PROP << 16):
            case LIL_TANK | (BULLET << 16):
            case PROP | (BULLET << 16):
            case LIL_TANK | (PROP << 16):
            case PROP | (LIL_TANK << 16):
            {
                /* Filter this collision */
            } break;
            case PL_LEFT_LEG | (FLOOR << 16):
            case PL_RIGHT_LEG | (FLOOR << 16): {
                rv = player_collideLimbFloor((player*)pChild1, type1, pObj2);
            } break;
            case FLOOR | (PL_LEFT_LEG << 16):
            case FLOOR | (PL_RIGHT_LEG << 16): {
                rv = player_collideLimbFloor((player*)pChild2, type2, pObj1);
            } break;
            case LIL_TANK | (FLOOR << 16): {
                rv = enemy_collideFloor((enemy*)pChild1, pObj2);
            } break;
            case FLOOR | (LIL_TANK << 16): {
                rv = enemy_collideFloor((enemy*)pChild2, pObj1);
            } break;
            case FLOOR | (PROP << 16): {
                double vx, vy;

                rv = gfmObject_collide(pObj1, pObj2);
                ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);

                rv = gfmObject_getVelocity(&vx, &vy, pObj2);
                ASSERT(rv == GFMRV_OK, rv);

                rv = gfmObject_setVelocity(pObj2, vx * 0.75, -abs(vy) * 0.25 -
                        0.1);
            } break;
            case PROP | (FLOOR << 16): {
                double vx, vy;

                rv = gfmObject_collide(pObj1, pObj2);
                ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);

                rv = gfmObject_getVelocity(&vx, &vy, pObj1);
                ASSERT(rv == GFMRV_OK, rv);

                rv = gfmObject_setVelocity(pObj1, vx * 0.25, -abs(vy) * 0.25 -
                        0.1);
            } break;
            case PROP | (PROP << 16): {
                double vx1, vx2, vy1, vy2;

                rv = gfmObject_collide(pObj1, pObj2);
                ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);

                rv = gfmObject_getVelocity(&vx1, &vy1, pObj1);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmObject_getVelocity(&vx2, &vy2, pObj2);
                ASSERT(rv == GFMRV_OK, rv);

                rv = gfmObject_setVelocity(pObj1, vx1 * 0.5, -abs(vy2) * 0.25 -
                        0.1);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmObject_setVelocity(pObj2, vx2 * 0.5, -abs(vy1) * 0.25 -
                        0.1);
            } break;
            default: {
#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
                /* Unfiltered collision, do something about it */
                raise(SIGINT);
                rv = GFMRV_INTERNAL_ERROR;
#endif
            }
        }
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmQuadtree_continue(pGame->pQt);
        ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE,
                rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

