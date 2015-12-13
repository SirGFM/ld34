/**
 * @file src/enemy.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>

#include <ld34/game.h>
#include <ld34/enemy.h>

#include <stdlib.h>
#include <string.h>

int en_liltank_data[] = {
              /* len|fps|loop|data... */
/* WALKING  */    4 , 6 , 1  , 64,65,66,67,
/* SHOOTING */    2 , 2 , 0  , 66,67,
/* DEATH    */   12 , 6 , 0  , 68,68,68,68,69,68,69,68,69,68,69,68
};

struct stEnemy {
    gfmSprite *pSpr;
};

/**
 * Alloc a new enemy
 *
 * @param  [out]ppEnemy The alloc enemy
 */
gfmRV enemy_getNew(enemy **ppEnemy) {
    gfmRV rv;

    *ppEnemy = (enemy*)malloc(sizeof(enemy));
    ASSERT(*ppEnemy, GFMRV_ALLOC_FAILED);
    memset(*ppEnemy, 0x0, sizeof(enemy));

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free an enemy and all of its resources
 *
 * @param  [ in]ppEnemy The enemy
 */
void enemy_clean(enemy **ppEnemy) {
    gfmSprite_free(&((*ppEnemy)->pSpr));
    free(*ppEnemy);
    *ppEnemy = 0;
}

/**
 * Initialize an enemy from the parser
 *
 * @param  [ in]pEnemy  The enemy
 * @param  [ in]pParser The parser
 * @param  [ in]type    The enemie's type
 */
gfmRV enemy_init(enemy *pEnemy, gfmParser *pParser, int type) {
    gfmRV rv;
    gfmSpriteset *pSset;
    int dataLen, firstAnim, h, ox, oy, *pData, w, x, y;

    rv = gfmSprite_getNew(&(pEnemy->pSpr));
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, rv);

    pData = 0;
    firstAnim = 0;
    switch (type) {
        case LIL_TANK: {
            w = 6;
            h = 8;
            ox = -8;
            oy = -5;
            pSset = pAssets->pSset16x16;

            pData = en_liltank_data;
            dataLen = sizeof(en_liltank_data) / sizeof(int);
        } break;
        default: {
            w = 1;
            h = 1;
            ox = 0;
            oy = 0;
            pSset = 0;
        }
    }

    rv = gfmSprite_init(pEnemy->pSpr, x, y, w, h, pSset, ox, oy, pEnemy, type);
    ASSERT(rv == GFMRV_OK, rv);

    if (pData) {
        rv = gfmSprite_addAnimations(pEnemy->pSpr, pData, dataLen);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmSprite_playAnimation(pEnemy->pSpr, firstAnim);
        ASSERT(rv == GFMRV_OK, rv);
    }

    switch (type) {
        case LIL_TANK: {
            rv = gfmSprite_setVelocity(pEnemy->pSpr, LIL_TANK_VX, 0.0);
            ASSERT(rv == GFMRV_OK, rv);
            rv = gfmSprite_setAcceleration(pEnemy->pSpr, 0.0, GRAV);
            ASSERT(rv == GFMRV_OK, rv);
        } break;
        default: {}
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the enemy
 *
 * @param  [ in]pEnemy  The enemy
 */
gfmRV enemy_update(enemy *pEnemy);

/**
 * Render the enemy
 *
 * @param  [ in]pEnemy  The enemy
 */
gfmRV enemy_draw(enemy *pEnemy);

/**
 * Collide the enemy agains the floor
 *
 * @param  [ in]pEnemy  The enemy
 * @param  [ in]pObj    The floor
 */
gfmRV enemy_collideFloor(enemy *pEnemy, gfmObject *pFloor) {
    gfmObject *pSelf;
    gfmRV rv;

    rv = gfmSprite_getObject(&pSelf, pEnemy->pSpr);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmObject_collide(pSelf, pFloor);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_TRUE) {
        gfmCollision dir;

        rv = gfmObject_getCurrentCollision(&dir, pSelf);
        ASSERT(rv == GFMRV_OK, rv);

        /* Don't stop if was colliding horizontally */
        if (dir & gfmCollision_down) {
            rv = gfmObject_setVerticalVelocity(pSelf, 0.0);
            ASSERT(rv == GFMRV_OK, rv);
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

