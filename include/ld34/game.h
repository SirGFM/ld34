/**
 * Struct that contains all game data
 *
 * @file include/ld34/game.h
 */
#ifndef __GAME_H__
#define __GAME_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTypes.h>

enum enState {
    state_none = 0,
    state_intro,
    state_game,
    state_max
};
typedef enum enState state;

/** The main game struct */
struct stGameCtx {
    /** The game context */
    gfmCtx *pCtx;
    /** The quadtree for collision */
    gfmQuadtreeRoot *pQt;
    /** Current state */
    state curState;
    /**
     * If this is different from state_none, the current state will be cleared
     * before re-entering the loop and the new state will be initialized the
     * next time it's run
     */
    state nextState;
    /** Whether in fullscreen or windowed mode */
    int isFullscreen;
    int width;
    int height;
    int drawQt;
};
typedef struct stGameCtx gameCtx;

struct stGameAssets {
    /** Texture handle */
    int texHandle;
    /** 8x8 spriteset */
    gfmSpriteset *pSset8x8;
    /** 16x16 spriteset */
    gfmSpriteset *pSset16x16;
    /** 32x16 spriteset */
    gfmSpriteset *pSset32x16;
};
typedef struct stGameAssets gameAssets;

struct stButton {
    /** Internal button handle */
    int handle;
    /** Number of times it was consecutively pressed */
    int num;
    /** Current button state */
    gfmInputState state;
};
typedef struct stButton button;

struct stGameButtons {
    button left_leg;
    button left_start;
    button right_start;
    button right_leg;
    button fullscreen;
    button quit;
    button drawQt;
    button gif;
};
typedef struct stGameButtons gameButtons;

struct stConfigCtx {
    int dps;
    int fps;
    int vsync;
    int ups;
};
typedef struct stConfigCtx configCtx;

/** The game context; Found on main.c */
extern gameCtx *pGame;
/** All texture, spritesets and songs; Found on main.c */
extern gameAssets *pAssets;
/** All buttons; Found on main.c */
extern gameButtons *pButtons;
/** Currently running state; Found on main.c */
extern void *pState;

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

#define PL_VX 15.0
#define PL_VY -50.0
#define PL_JUMP_VX 25.0
#define PL_JUMP_VY -80.0
#define PL_HOLD_T 600
#define PL_JUMP_T 125
#define PL_MAX_DIST 22

#define PL_UPPER     gfmType_reserved_2
#define PL_LOWER     gfmType_reserved_3
#define PL_LEFT_LEG  gfmType_reserved_4
#define PL_RIGHT_LEG gfmType_reserved_5
#define FLOOR        gfmType_reserved_6

#endif /* __GAME_H__ */

