/**
 * Handle collisions
 *
 * @file src/collide.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSave.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>

#include <ld34/collide.h>
#include <ld34/enemy.h>
#include <ld34/game.h>
#include <ld34/player.h>
#include <ld34/textManager.h>

#include <stdlib.h>

#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
#  include <signal.h>
#endif

static inline gfmRV collide_checkpoint(gfmObject *pPl, gfmObject *pCheckpoint) {
    gfmRV rv;
    gfmSave *pSave;
    int x, y;

    pSave = 0;

    rv = gfmObject_isOverlaping(pPl, pCheckpoint);
    if (rv != GFMRV_TRUE) {
        return GFMRV_OK;
    }

    rv = gfmObject_getPosition(&x, &y, pPl);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_setPosition(pCheckpoint, -100, -100);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_setDimensions(pCheckpoint, 4, 4);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSave_getNew(&pSave);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSave_bindStatic(pSave, pGame->pCtx, SAVE_FILE);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSave_writeStatic(pSave, "checkpoint_x", x);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSave_writeStatic(pSave, "checkpoint_y", y-16);
    ASSERT(rv == GFMRV_OK, rv);

    rv = textManager_pushTextStatic(pGame->pTextManager, "               CHECKPOINT", 2000);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_playAudio(0, pGame->pCtx, pAssets->sfxCheckpoint, 0.4);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    gfmSave_free(&pSave);

    return rv;
}

static inline gfmRV collide_handlePlEnemy(player *pPl, gfmObject *pPlObj,
        enemy *pEne, gfmObject *pEneObj) {
    gfmRV rv;

    rv = gfmObject_isOverlaping(pPlObj, pEneObj);
    if (rv == GFMRV_TRUE) {
        double vy;

        rv = gfmObject_getVerticalVelocity(&vy, pPlObj);
        ASSERT(rv == GFMRV_OK, rv);
        rv = enemy_getHurt(pEne, vy);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmObject_setFixed(pEneObj);
        ASSERT(rv == GFMRV_OK, rv);
        gfmObject_separateVertical(pEneObj, pPlObj);
        rv = gfmObject_setMovable(pEneObj);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}


static inline gfmRV collide_spawnExplosion(gfmGroupNode *pCtx, gfmObject *pObj) {
    gfmRV rv;
    gfmSprite *pSpr;
    int x, y;

    rv = gfmGroup_removeNode(pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&x, &y, pObj);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmGroup_recycle(&pSpr, pGame->pParticles);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setPosition(pGame->pParticles, x, y);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setAnimation(pGame->pParticles, P_EXPLOSION);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_playAudio(0, pGame->pCtx, pAssets->sfxPlHurt, 0.4);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Push the colliding object away
 *
 * @param  [ in]pSelf The current object
 * @param  [ in]pObj  The other object
 */
gfmRV collide_pushObject(gfmObject *pSelf, gfmObject *pObj) {
    gfmRV rv;

    rv = gfmObject_setFixed(pSelf);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_collide(pSelf, pObj);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    rv = gfmObject_setMovable(pSelf);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV collide_bounceOff(gfmObject *pSelf, gfmObject *pObj) {
    gfmRV rv;

    rv = gfmObject_collide(pSelf, pObj);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);

    if (rv == GFMRV_TRUE) {
        double vx, vy;

        rv = gfmObject_getVelocity(&vx, &vy, pObj);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmObject_setVelocity(pSelf, vx * 0.75, -abs(vy) * 0.5);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV collide_elastic(gfmObject *pObj1, gfmObject *pObj2) {
    gfmRV rv;
    int x1, x2, y1, y2;
    double vx1, vx2, vy1, vy2;

    rv = gfmObject_getPosition(&x1, &y1, pObj1);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getPosition(&x2, &y2, pObj2);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_collide(pObj1, pObj2);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);

    if (rv == GFMRV_TRUE) {
        if (y1 > y2) {
            rv = gfmObject_setMovable(pObj1);
        }
        else {
            rv = gfmObject_setMovable(pObj2);
        }
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmObject_getVelocity(&vx1, &vy1, pObj1);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmObject_getVelocity(&vx2, &vy2, pObj2);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmObject_setVelocity(pObj1, -vx1 * 0.5, vy1 * 0.5);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmObject_setVelocity(pObj2, -vx2 * 0.5, vy2 * 0.5);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:

    return rv;
}

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
            /* Filter those collisions */
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
            case PL_LOWER | (CHECKPOINT << 16):
            case PL_LEFT_LEG | (PL_UPPER << 16):
            case PL_LEFT_LEG | (PL_LOWER << 16):
            case PL_LEFT_LEG | (PL_LEFT_LEG << 16):
            case PL_LEFT_LEG | (PL_RIGHT_LEG << 16):
            case PL_LEFT_LEG | (CHECKPOINT << 16):
            case PL_RIGHT_LEG | (PL_UPPER << 16):
            case PL_RIGHT_LEG | (PL_LOWER << 16):
            case PL_RIGHT_LEG | (PL_LEFT_LEG << 16):
            case PL_RIGHT_LEG | (PL_RIGHT_LEG << 16):
            case PL_RIGHT_LEG | (CHECKPOINT << 16):
            case FLOOR | (PL_UPPER << 16):
            case FLOOR | (PL_LOWER << 16):
            case FLOOR | (FLOOR << 16):
            case FLOOR | (BULLET << 16):
            case FLOOR | (TEXT << 16):
            case FLOOR | (CHECKPOINT << 16):
            case BULLET | (FLOOR << 16):
            case BULLET | (LIL_TANK << 16):
            case BULLET | (TURRET << 16):
            case BULLET | (BULLET << 16):
            case BULLET | (PROP << 16):
            case BULLET | (TEXT << 16):
            case BULLET | (CHECKPOINT << 16):
            case LIL_TANK | (BULLET << 16):
            case LIL_TANK | (TEXT << 16):
            case TURRET | (BULLET << 16):
            case TURRET | (TEXT << 16):
            case PROP | (BULLET << 16):
            case PROP | (TEXT << 16):
            case PROP | (CHECKPOINT << 16):
            case TEXT | (FLOOR << 16):
            case TEXT | (LIL_TANK << 16):
            case TEXT | (TURRET << 16):
            case TEXT | (BULLET << 16):
            case TEXT | (PROP << 16):
            case TEXT | (TEXT << 16):
            case TEXT | (CHECKPOINT << 16):
            case PL_UPPER | (TURRET << 16):
            case PL_LOWER | (TURRET << 16):
            case TURRET | (PL_UPPER << 16):
            case TURRET | (PL_LOWER << 16):
            case PL_UPPER | (LIL_TANK << 16):
            case PL_LOWER | (LIL_TANK << 16):
            case LIL_TANK | (PL_UPPER << 16):
            case LIL_TANK | (PL_LOWER << 16):
            case CHECKPOINT | (TEXT << 16):
            case CHECKPOINT | (PROP << 16):
            case CHECKPOINT | (BULLET << 16):
            case CHECKPOINT | (FLOOR << 16):
            case CHECKPOINT | (PL_LEFT_LEG << 16):
            case CHECKPOINT | (PL_RIGHT_LEG << 16):
            case CHECKPOINT | (PL_LOWER << 16):

            case PL_LEFT_LEG | (PROP << 16):
            case PL_RIGHT_LEG | (PROP << 16):
            case PROP | (PL_LEFT_LEG << 16):
            case PROP | (PL_RIGHT_LEG << 16):

            break;
            /* Collide against floor */
            case PL_LEFT_LEG | (FLOOR << 16):
            case PL_RIGHT_LEG | (FLOOR << 16): {
                rv = player_collideLimbFloor((player*)pChild1, type1, pObj2);
            } break;
            case FLOOR | (PL_LEFT_LEG << 16):
            case FLOOR | (PL_RIGHT_LEG << 16): {
                rv = player_collideLimbFloor((player*)pChild2, type2, pObj1);
            } break;
#if 0
            /* Walk over pellets */
            case PL_LEFT_LEG | (PROP << 16):
            case PL_RIGHT_LEG | (PROP << 16): {
                rv = gfmObject_setFixed(pObj2);
                ASSERT(rv == GFMRV_OK, rv);
                rv = player_collideLimbFloor((player*)pChild1, type1, pObj2);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmObject_setMovable(pObj2);
            } break;
            case PROP | (PL_LEFT_LEG << 16):
            case PROP | (PL_RIGHT_LEG << 16): {
                rv = gfmObject_setFixed(pObj1);
                ASSERT(rv == GFMRV_OK, rv);
                rv = player_collideLimbFloor((player*)pChild2, type2, pObj1);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmObject_setMovable(pObj1);
            } break;
#endif
            /* Hurt player */
            case PL_UPPER | (BULLET << 16):
            case PL_LOWER | (BULLET << 16):
            case PL_LEFT_LEG | (BULLET << 16):
            case PL_RIGHT_LEG | (BULLET << 16): {
                rv = collide_spawnExplosion((gfmGroupNode*)pChild2, pObj2);
            } break;
            case BULLET | (PL_UPPER << 16):
            case BULLET | (PL_LOWER << 16):
            case BULLET | (PL_LEFT_LEG << 16):
            case BULLET | (PL_RIGHT_LEG << 16): {
                rv = collide_spawnExplosion((gfmGroupNode*)pChild1, pObj1);
            } break;
            /* Hurt player or kill enemy */
            case PL_LEFT_LEG | (TURRET << 16):
            case PL_RIGHT_LEG | (TURRET << 16):
            case PL_LEFT_LEG | (LIL_TANK << 16):
            case PL_RIGHT_LEG | (LIL_TANK << 16): {
                rv = collide_handlePlEnemy((player*)pChild1, pObj1,
                        (enemy*)pChild2, pObj2);
            } break;
            case TURRET | (PL_LEFT_LEG << 16):
            case TURRET | (PL_RIGHT_LEG << 16):
            case LIL_TANK | (PL_LEFT_LEG << 16):
            case LIL_TANK | (PL_RIGHT_LEG << 16): {
                rv = collide_handlePlEnemy((player*)pChild2, pObj2,
                        (enemy*)pChild1, pObj1);
            } break;
            /* Collide enemy with floor */
            case TURRET | (FLOOR << 16):
            case LIL_TANK | (FLOOR << 16): {
                rv = enemy_collideFloor((enemy*)pChild1, pObj2);
            } break;
            case FLOOR | (TURRET << 16):
            case FLOOR | (LIL_TANK << 16): {
                rv = enemy_collideFloor((enemy*)pChild2, pObj1);
            } break;
            /* Make enemies push pellets */
            case TURRET | (PROP << 16):
            case LIL_TANK | (PROP << 16): {
                rv = collide_pushObject(pObj1, pObj2);
            } break;
            case PROP | (TURRET << 16):
            case PROP | (LIL_TANK << 16): {
                rv = collide_pushObject(pObj2, pObj1);
            } break;
            /* Bounce pellets off floor and itsef */
            case FLOOR | (PROP << 16): {
                rv = collide_bounceOff(pObj2, pObj1);
            } break;
            case PROP | (FLOOR << 16): {
                rv = collide_bounceOff(pObj1, pObj2);
            } break;
            case PROP | (PROP << 16): {
                rv = collide_elastic(pObj1, pObj2);
            } break;
            /* Queue a text to be displayed */
            case PL_LEFT_LEG | (TEXT << 16):
            case PL_RIGHT_LEG | (TEXT << 16): {
                textManager_pushEvent(pGame->pTextManager, (textEvent*)pChild2);
                rv = GFMRV_OK;
            } break;
            case TEXT | (PL_LEFT_LEG << 16):
            case TEXT | (PL_RIGHT_LEG << 16): {
                textManager_pushEvent(pGame->pTextManager, (textEvent*)pChild1);
                rv = GFMRV_OK;
            } break;
            /* Checkpoint! */
            case PL_UPPER | (CHECKPOINT << 16): {
                rv = collide_checkpoint(pObj1, pObj2);
            } break;
            case CHECKPOINT | (PL_UPPER << 16): {
                rv = collide_checkpoint(pObj2, pObj1);
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

