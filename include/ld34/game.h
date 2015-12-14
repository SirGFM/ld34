/**
 * Struct that contains all game data
 *
 * @file include/ld34/game.h
 */
#ifndef __GAME_H__
#define __GAME_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTypes.h>

#include <ld34/textManager.h>

enum enState {
    state_none = 0,
    state_intro,
    state_game,
    state_max
};
typedef enum enState state;

enum enParticles {
    P_BULLET = 0,
    P_PELLET1,
    P_PELLET2,
    P_EXPLOSION,
};

/** The main game struct */
struct stGameCtx {
    /** The game context */
    gfmCtx *pCtx;
    /** Particles used only for basic eye-candy */
    gfmGroup *pParticles;
    /** Particles used only for ~*awesome*~ eye-candy */
    gfmGroup *pBullets;
    /** Particles used only for ~*awesome*~ eye-candy */
    gfmGroup *pProps;
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

    textManager *pTextManager;

    /** Whether in fullscreen or windowed mode */
    int isFullscreen;
    int width;
    int height;
    int drawQt;

    int run;
    int next;
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

    int audBass1;
    int audBass2;
    int audMelody;

    int sfxLeftStep;
    int sfxRightStep;
    int sfxEnemyCrushed;
    int sfxEnemyExplosion;
    int sfxPlHurt;
    int sfxCheckpoint;
    int sfxEnemyShoot;
    int sfxText;
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
    button left_legBack;
    button left_start;
    button right_start;
    button right_leg;
    button right_legBack;
    button fullscreen;
    button quit;
    button drawQt;
    button gif;
    button reset;
    button run;
    button next;
    button pause;
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
#define BGCOLOR 0x332825
#define TEXATLAS "atlas.bmp"
#define COLORKEY 0xff00ff

#define SAVE_FILE "game.sav"

#define GRAV 100
#define PARTICLE_TTL 10000
#define NUM_PARTICLES 2048
#define TEXT_DELAY 60

#define PL_VX 30.0
#define PL_VY -52.5
#define PL_JUMP_VX 25.0
#define PL_JUMP_VY -80.0
#define PL_HOLD_T 600
#define PL_JUMP_T 125
#define PL_MAX_DIST 22

#define LIL_TANK_VX -40.0
#define LIL_TIME_TO_SHOOT 5000
#define LIL_TANK_TIME_TO_STRIKE_BACK 1000
#define LIL_TANK_BETWEEN_SHOOT 300
#define LIL_TANK_NUM_SHOOTS 5

#define TURRET_TIME_TO_SHOOT 10000
#define TURRET_BETWEEN_SHOOT 150
#define TURRET_NUM_SHOOTS 10
#define TURRET_BULLET_VY -80

#define PL_UPPER     gfmType_reserved_2
#define PL_LOWER     gfmType_reserved_3
#define PL_LEFT_LEG  gfmType_reserved_4
#define PL_RIGHT_LEG gfmType_reserved_5
#define FLOOR        gfmType_reserved_6
#define LIL_TANK     gfmType_reserved_7
#define BULLET       gfmType_reserved_8
#define PROP         gfmType_reserved_9
#define TEXT         gfmType_reserved_10
#define TURRET       gfmType_reserved_11
#define CHECKPOINT   gfmType_reserved_12
#define EXIT         gfmType_reserved_13

#endif /* __GAME_H__ */

