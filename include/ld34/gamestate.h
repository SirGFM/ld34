/**
 * The main state of the game
 *
 * @file include/ld34/gamestate.h
 */
#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__

#include <GFraMe/gfmError.h>

/**
 * Initialize the game state
 *
 * NOTE: pState will be overwritten!
 */
gfmRV gamestate_init();

/** Update the current state as a gamestate */
gfmRV gamestate_update();

/** Render the current state as a gamestate */
gfmRV gamestate_draw();

/**
 * Free all game state related resources
 *
 * NOTE: pState will be overwritten!
 */
gfmRV gamestate_clean();

#endif /* __GAMESTATE_H__ */

