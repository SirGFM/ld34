/**
 * The player
 *
 * @file include/ld34/player.h
 */
#ifndef __PLAYER_STRUCT__
#define __PLAYER_STRUCT__

typedef struct stPlayer player;

#endif /* __PLAYER_STRUCT__ */

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <GFraMe/gfmError.h>

/**
 * Alloc and initialize the player
 *
 * @param  [out]ppPlayer The player
 * @param  [ in]x        Initial horizontal position (torso's upper left)
 * @param  [ in]y        Initial vertical position (torso's upper left)
 */
gfmRV player_init(player **ppPlayer, int x, int y);

/**
 * Release the player
 *
 * @param  [ in]ppPlayer The player
 */
void player_clean(player **ppPlayer);

/**
 * Update the player's physics and handle inputs
 *
 * NOTE: Must be called before colliding!
 *
 * @param  [ in]pPlayer The player
 */
gfmRV player_preUpdate(player *pPlayer);

/**
 * Update the player's animation and anything related to collision (e.g.,
 * jumping)
 *
 * NOTE: Must be called after colliding!
 *
 * @param  [ in]pPlayer The player
 */
gfmRV player_postUpdate(player *pPlayer);

/**
 * Render the player
 *
 * @param  [ in]pPlayer The player
 */
gfmRV player_draw(player *pPlayer);

#endif /* __PLAYER_H__ */

