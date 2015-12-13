/**
 * @file include/ld34/enemy.h
 */
#ifndef __ENEMY_STRUCT__
#define __ENEMY_STRUCT__

typedef struct stEnemy enemy;

#endif /* __ENEMY_STRUCT__ */

#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>

/**
 * Alloc a new enemy
 *
 * @param  [out]ppEnemy The alloc enemy
 */
gfmRV enemy_getNew(enemy **ppEnemy);

/**
 * Free an enemy and all of its resources
 *
 * @param  [ in]ppEnemy The enemy
 */
void enemy_clean(enemy **ppEnemy);

/**
 * Initialize an enemy from the parser
 *
 * @param  [ in]pEnemy  The enemy
 * @param  [ in]pParser The parser
 * @param  [ in]type    The enemie's type
 */
gfmRV enemy_init(enemy *pEnemy, gfmParser *pParser, int type);

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
gfmRV enemy_collideFloor(enemy *pEnemy, gfmObject *pFloor);

#endif /* __ENEMY_H__ */

