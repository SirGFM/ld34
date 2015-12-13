/**
 * @file src/enemy.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>

#include <ld34/collide.h>
#include <ld34/enemy.h>
#include <ld34/game.h>

#include <stdlib.h>
#include <string.h>

int en_liltank_data[] = {
              /* len|fps|loop|data... */
/* WALKING  */    4 , 6 , 1  , 64,65,66,67,
/* SHOOTING */    2 , 2 , 0  , 66,67,
/* DEATH    */   12 , 6 , 0  , 68,68,68,68,69,68,69,68,69,68,69,68
};

int en_turret_data[] = {
              /* len|fps|loop|data... */
/* IDLE     */    2 , 6 , 1  , 96,97,
/* DEATH    */   12 , 6 , 0  , 98,99,98,99,98,99,98,99,98,99,98,99
};

struct stEnemy {
    gfmSprite *pSpr;
    int timeToAction;
    int switchDir;
    int num;
    int type;
    int isHurt;
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
    rv = gfmParser_getDimensions(&w, &h, pParser);
    ASSERT(rv == GFMRV_OK, rv);

    y -= h;

    pData = 0;
    firstAnim = 0;
    switch (type) {
        case LIL_TANK: {
            w = 6;
            h = 8;
            ox = -5;
            oy = -8;
            pSset = pAssets->pSset16x16;

            pData = en_liltank_data;
            dataLen = sizeof(en_liltank_data) / sizeof(int);
        } break;
        case TURRET: {
            x += 4;
            w = 8;
            h = 8;
            ox = -4;
            oy = -8;
            pSset = pAssets->pSset16x16;

            pData = en_turret_data;
            dataLen = sizeof(en_turret_data) / sizeof(int);
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

            pEnemy->timeToAction = LIL_TANK_BETWEEN_SHOOT;
            pEnemy->num = LIL_TANK_NUM_SHOOTS;
        } break;
        case TURRET: {
            pEnemy->timeToAction = TURRET_BETWEEN_SHOOT;
            pEnemy->num = TURRET_NUM_SHOOTS;
            rv = gfmSprite_setAcceleration(pEnemy->pSpr, 0.0, GRAV);
            ASSERT(rv == GFMRV_OK, rv);
        } break;
        default: {}
    }

    pEnemy->type = type;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the enemy (and collide it against the quadtree)
 *
 * @param  [ in]pEnemy  The enemy
 */
gfmRV enemy_preUpdate(enemy *pEnemy) {
    gfmRV rv;
    int elapsed;

    if (pEnemy->isHurt == 3) {
        return GFMRV_OK;
    }
    else if (pEnemy->isHurt == 2) {
        int i;

        rv = gfmSprite_update(pEnemy->pSpr, pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmSprite_didAnimationFinish(pEnemy->pSpr);
        if (rv == GFMRV_TRUE || rv == GFMRV_NO_ANIMATION_PLAYING) {
            i = 0;
            while (i < 8) {
                gfmSprite *pSpr;
                int vx, vy, x, y;

                if (i % 4 == 0) {
                    vx = 0;
                    vy = 50 * (1 - 2 * (i == 0));
                }
                else if (i % 4 == 2) {
                    vx = 50 * (1 - 2 * (i == 0));
                    vy = 0;
                }
                else {
                    vx = 50 * 0.707106781 * (1 - 2 * (i > 4));
                    vy = 50 * 0.707106781 * (1 - 2 * (((i + 1) % 8) > 4));
                }

                rv = gfmSprite_getPosition(&x, &y, pEnemy->pSpr);
                ASSERT(rv == GFMRV_OK, rv);

                rv = gfmGroup_recycle(&pSpr, pGame->pParticles);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmGroup_setPosition(pGame->pParticles, x, y);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmGroup_setVelocity(pGame->pParticles, vx, vy);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmGroup_setAnimation(pGame->pParticles, P_EXPLOSION);
                ASSERT(rv == GFMRV_OK, rv);

                i++;
            }

            pEnemy->isHurt = 3;
        }

        return GFMRV_OK;
    }

    rv = gfm_getElapsedTime(&elapsed, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    if (pEnemy->timeToAction > 0) {
        pEnemy->timeToAction -= elapsed;
    }
    else {
        /* Do next action */
        switch (pEnemy->type) {
            case TURRET: {
                if (pEnemy->num > 0) {
                    gfmSprite *pSpr;
                    int x, y;

                    rv = gfmSprite_getPosition(&x, &y, pEnemy->pSpr);
                    ASSERT(rv == GFMRV_OK, rv);

                    x += 4;

                    /* Spawn a bullet */
                    rv = gfmGroup_recycle(&pSpr, pGame->pBullets);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setPosition(pGame->pBullets, x, y-3);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setAnimation(pGame->pBullets, P_BULLET);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setVelocity(pGame->pBullets, 0.0, TURRET_BULLET_VY);
                    ASSERT(rv == GFMRV_OK, rv);

                    /* Spawn a pellet */
                    rv = gfmGroup_recycle(&pSpr, pGame->pProps);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setPosition(pGame->pProps, x, y-1);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setAnimation(pGame->pProps, P_PELLET1);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setVelocity(pGame->pProps, 25,
                            TURRET_BULLET_VY * 0.75);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setAcceleration(pGame->pProps, 0, GRAV);
                    ASSERT(rv == GFMRV_OK, rv);


                    pEnemy->timeToAction = LIL_TANK_BETWEEN_SHOOT;
                    pEnemy->num--;
                }
                else {
                    pEnemy->timeToAction = TURRET_TIME_TO_SHOOT;
                    pEnemy->num = TURRET_NUM_SHOOTS;
                }
            } break;
            case LIL_TANK: {
                int flipped;

                rv = gfmSprite_getDirection(&flipped, pEnemy->pSpr);
                ASSERT(rv == GFMRV_OK, rv);

                if (pEnemy->num > 0) {
                    gfmSprite *pSpr;
                    int vx, vy, x, y;

                    rv = gfmSprite_setHorizontalVelocity(pEnemy->pSpr, 0.0);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmSprite_getPosition(&x, &y, pEnemy->pSpr);
                    ASSERT(rv == GFMRV_OK, rv);

                    if (flipped) {
                        x += 8;
                        vx = 40;
                    }
                    else {
                        x -= 4;
                        vx = -40;
                    }
                    vy = -30;

                    /* Spawn a bullet */
                    rv = gfmGroup_recycle(&pSpr, pGame->pBullets);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setPosition(pGame->pBullets, x, y-1);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setAnimation(pGame->pBullets, P_BULLET);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setVelocity(pGame->pBullets, vx, vy);
                    ASSERT(rv == GFMRV_OK, rv);

                    /* Spawn a pellet */
                    rv = gfmGroup_recycle(&pSpr, pGame->pProps);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setPosition(pGame->pProps, x, y-1);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setAnimation(pGame->pProps, P_PELLET1);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setVelocity(pGame->pProps, -vx, vy);
                    ASSERT(rv == GFMRV_OK, rv);
                    rv = gfmGroup_setAcceleration(pGame->pProps, 0, GRAV);
                    ASSERT(rv == GFMRV_OK, rv);


                    pEnemy->timeToAction = LIL_TANK_BETWEEN_SHOOT;
                    pEnemy->num--;
                }
                else {
                    double vx;

                    if (flipped) {
                        vx = -LIL_TANK_VX;
                    }
                    else {
                        vx = LIL_TANK_VX;
                    }

                    rv = gfmSprite_setHorizontalVelocity(pEnemy->pSpr, vx);
                    ASSERT(rv == GFMRV_OK, rv);

                    pEnemy->timeToAction = LIL_TIME_TO_SHOOT;
                    pEnemy->num = LIL_TANK_NUM_SHOOTS;
                }
            } break;
            default: {}
        }
    }

    rv = gfmSprite_update(pEnemy->pSpr, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmQuadtree_collideSprite(pGame->pQt, pEnemy->pSpr);
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
 * Change state after all collisions
 *
 * @param  [ in]pEnemy  The enemy
 */
gfmRV enemy_postUpdate(enemy *pEnemy) {
    gfmRV rv;

    if (!pEnemy->isHurt && pEnemy->switchDir) {
        double vx;
        int flipped;

        rv = gfmSprite_getDirection(&flipped, pEnemy->pSpr);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmSprite_setDirection(pEnemy->pSpr, !flipped);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmSprite_getHorizontalVelocity(&vx, pEnemy->pSpr);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmSprite_setHorizontalVelocity(pEnemy->pSpr, -vx);
        ASSERT(rv == GFMRV_OK, rv);

        pEnemy->switchDir = 0;
    }
    else if (pEnemy->isHurt == 1) {
        rv = gfmSprite_setVelocity(pEnemy->pSpr, 0, 0);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmSprite_setAcceleration(pEnemy->pSpr, 0, 0);
        ASSERT(rv == GFMRV_OK, rv);

        switch (pEnemy->type) {
            case LIL_TANK: {
                rv = gfmSprite_playAnimation(pEnemy->pSpr, 2);
            } break;
            case TURRET: {
                rv = gfmSprite_playAnimation(pEnemy->pSpr, 1);
            } break;
            default: { rv = GFMRV_OK; }
        }
        ASSERT(rv == GFMRV_OK, rv);

        pEnemy->isHurt = 2;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Render the enemy
 *
 * @param  [ in]pEnemy  The enemy
 */
gfmRV enemy_draw(enemy *pEnemy) {
    gfmRV rv;

    if (pEnemy->isHurt < 3) {
        rv = gfmSprite_draw(pEnemy->pSpr, pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

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

        if (dir & gfmCollision_down) {
            rv = gfmObject_setVerticalVelocity(pSelf, 0.0);
            ASSERT(rv == GFMRV_OK, rv);
        }
        if (dir & gfmCollision_hor) {
            pEnemy->switchDir = 1;
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

