/**
 * Struct that contains all game data
 *
 * @file include/ld34/game.h
 */
#ifndef __GAME_H__
#define __GAME_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmSpriteset.h>

/** The main game struct */
struct stGameCtx {
/* ========================================================================== */
    /** The game context */
    gfmCtx *pCtx;
/* == BUTTONS =============================================================== */
/* == STATE ================================================================= */
};
typedef struct stGameCtx gameCtx;

struct stGameAssets {
    /** Texture handle */
    int texHandle;
    /** 8x8 spriteset */
    gfmSpriteset *pSset8x8;
};
typedef struct stGameAssets gameAssets;

struct stConfigCtx {
    int dps;
    int fps;
    int vsync;
    int ups;
};
typedef struct stConfigCtx configCtx;

/**
 * Make the game context accessible to every *.c file (that includes this);
 *
 * It can be found on main.c
 */
extern gameCtx *pGame;
/**
 * Make the assets accessible to every *.c file (that includes this);
 *
 * It can be found on main.c
 */
extern gameAssets *pAssets;

#define ORG "com.gfmgamecorner"
#define TITLE "LD34"
#define BBWDT 320
#define BBHGT 240
#define WNWDT 640
#define WNHGT 480
#define CAN_RESIZE 1
#define BGCOLOR 0x000000
#define TEXATLAS "atlas.bmp"
#define COLORKEY 0xff00ff

#endif /* __GAME_H__ */

