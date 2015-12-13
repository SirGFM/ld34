/**
 * Small window used for rendering texts
 *
 * @file src/textManager.c
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/gfmText.h>

#include <ld34/game.h>
#include <ld34/textManager.h>

#include <stdlib.h>
#include <string.h>

struct stTextEvent {
    /** Collideable object */
    gfmObject *pSelf;
    /** Actual string */
    gfmString *pString;
    /** For how long should the text be displayed after completition */
    int ttl;
    /** Whether the event can be re-triggered */
    int repeat;
    /** Queued text event to be displayed after this one */
    struct stTextEvent *pNext;
};
typedef struct stTextEvent textEvent;

gfmGenArr_define(textEvent);

struct stTextManager {
    gfmGenArr_var(textEvent, pTexEvs);
    /** List of queued events */
    textEvent *pQueue;
    /** Currently playing text */
    gfmText *pText;
    /* Position of the window manager */
    int x;
    /* Position of the window manager */
    int y;
    /* Width of the window, in tiles */
    int width;
    /* height of the window, in tiles */
    int height;
    /* Whether a window should be rendered or not */
    int windowed;
};

static void textEvent_clean(textEvent **ppCtx) {
}

static gfmRV textEvent_getNew(textEvent **ppCtx) {
    gfmRV rv;
    textEvent *pCtx;

    pCtx = (textEvent*)malloc(sizeof(textEvent));
    ASSERT(pCtx, GFMRV_ALLOC_FAILED);
    memset(pCtx, 0x0, sizeof(textEvent));

    rv = gfmObject_getNew(&(pCtx->pSelf));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmString_getNew(&(pCtx->pString));
    ASSERT(rv == GFMRV_OK, rv);

    *ppCtx = pCtx;
    pCtx = 0;
    rv = GFMRV_OK;
__ret:
    if (pCtx) {
        textEvent_clean(&pCtx);
    }

    return rv;
}

/**
 * Alloc a new text manager
 *
 * @param  [out]ppCtx The text manager
 */
gfmRV textManager_init(textManager **ppCtx);

/**
 * Frees text manager
 *
 * @param  [ in]ppCtx The text manager
 */
gfmRV textManager_clean(textManager **ppCtx);

/**
 * Add a new text, to be triggered when the player touch it
 *
 * @param  [ in]pCtx    The text manager
 * @param  [ in]pParser The parser with the event's data
 */
gfmRV textManager_pushEvent(textManager *pCtx, gfmParser *pParser);

/**
 * Force a text to be displayed (it's actually pushed and only displayed after
 * the previous finishes)
 *
 * @param  [ in]pCtx The text manager
 * @param  [ in]pStr The string
 * @param  [ in]len  The string's length
 * @param  [ in]ttl  How long should the text be displayed after completition
 */
gfmRV textManager_pushText(textManager *pCtx, char *pStr, int len);

/**
 * Update the currently display text, if any
 *
 * @param  [ in]pCtx The text manager
 */
gfmRV textManager_update(textManager *pCtx);

/**
 * Render the currently display text, if any
 *
 * @param  [ in]pCtx The text manager
 */
gfmRV textManager_draw(textManager *pCtx);

