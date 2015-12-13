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

#include <ld34/collide.h>
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

gfmGenArr_define(textEvent);

struct stTextManager {
    gfmGenArr_var(textEvent, pTexEvs);
    textEvent *pCurEv;
    /** List of queued events */
    textEvent *pQueue;
    /** Currently playing text */
    gfmText *pText;
    /** Position of the window manager */
    int x;
    /** Position of the window manager */
    int y;
    /** Width of the window, in tiles */
    int width;
    /** height of the window, in tiles */
    int height;
    /** Whether a window should be rendered or not */
    int windowed;
    /** Time since the text finished displaying */
    int elapsed;
};

static void textEvent_clean(textEvent **ppCtx) {
    if ((*ppCtx)->pSelf) {
        gfmObject_free(&((*ppCtx)->pSelf));
    }
    if ((*ppCtx)->pString) {
        gfmString_free(&((*ppCtx)->pString));
    }
    free(*ppCtx);
    *ppCtx = 0;
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
 * @param  [out]ppCtx      The text manager
 * @param  [ in]x          Window's position
 * @param  [ in]y          Window's position
 * @param  [ in]w          Window's dimensions
 * @param  [ in]h          Window's dimensions
 * @param  [ in]showWindow Whether the window should be displayed
 */
gfmRV textManager_init(textManager **ppCtx, int x, int y, int w, int h,
        int showWindow) {
    gfmRV rv;

    *ppCtx = (textManager*)malloc(sizeof(textManager));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(textManager));

    rv = gfmText_getNew(&((*ppCtx)->pText));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmText_init((*ppCtx)->pText, x+8, y+8, w-2, h-2, TEXT_DELAY, 0,
            pAssets->pSset8x8, 0/*tile*/);
    ASSERT(rv == GFMRV_OK, rv);

    (*ppCtx)->x = x;
    (*ppCtx)->y = y;
    (*ppCtx)->width = w;
    (*ppCtx)->height = h;
    (*ppCtx)->windowed = showWindow;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Frees text manager
 *
 * @param  [ in]ppCtx The text manager
 */
void textManager_clean(textManager **ppCtx) {
    gfmGenArr_clean((*ppCtx)->pTexEvs, textEvent_clean);

    if ((*ppCtx)->pText) {
        gfmText_free(&((*ppCtx)->pText));
    }
    free(*ppCtx);
    *ppCtx = 0;
}

/**
 * Add a new text, to be triggered when the player touch it
 *
 * @param  [ in]pCtx    The text manager
 * @param  [ in]pParser The parser with the event's data
 */
gfmRV textManager_addEvent(textManager *pCtx, gfmParser *pParser) {
    gfmRV rv;
    textEvent *pEv;
    int h, num, w, x, y;

    gfmGenArr_getNextRef(textEvent, pCtx->pTexEvs, 1, pEv, textEvent_getNew);
    gfmGenArr_push(pCtx->pTexEvs);

    rv = gfmParser_getNumProperties(&num, pParser);
    ASSERT(rv == GFMRV_OK, rv);
    ASSERT(num == 3, GFMRV_INTERNAL_ERROR);

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmParser_getDimensions(&w, &h, pParser);
    ASSERT(rv == GFMRV_OK, rv);

    y -= h;

    rv = gfmObject_init(pEv->pSelf, x, y, w, h, pEv, TEXT);
    ASSERT(rv == GFMRV_OK, rv);

    while (num > 0) {
        char *pKey, *pVal;
        num--;

        rv = gfmParser_getProperty(&pKey, &pVal, pParser, num);
        ASSERT(rv == GFMRV_OK, rv);

        if (strcmp("string", pKey) == 0) {
            rv = gfmString_init(pEv->pString, pVal, strlen(pVal), 1/*doCopy*/);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else if (strcmp("repeat", pKey) == 0) {
            pEv->repeat = (pVal[0] == 't');
        }
        else if (strcmp("ttl", pKey) == 0) {
            int val;

            val = 0;
            while (*pVal) {
                val = val * 10 + (*pVal) - '0';
                pVal++;
            }

            pEv->ttl = val;
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Push a text to be displayed as soon as possible
 *
 * @param  [ in]pCtx The text manager
 * @param  [ in]pEv  The event to be pushed
 */
void textManager_pushEvent(textManager *pCtx, textEvent *pEv) {
    if (!pCtx->pQueue) {
        pCtx->pQueue = pEv;
    }
    else {
        textEvent *pTmp;

        pTmp = pCtx->pQueue;
        while (pTmp->pNext) {
            pTmp = pTmp->pNext;
        }
        pTmp->pNext = pEv;
    }

    if (!pEv->repeat) {
        gfmObject_setPosition(pEv->pSelf, -100, -100);
        gfmObject_setDimensions(pEv->pSelf, 2, 2);
    }
}

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
 * Update all events and collide 'em
 *
 * @param  [ in]pCtx The text manager
 */
gfmRV textManager_preUpdate(textManager *pCtx) {
    gfmRV rv;
    int i;

    i = 0;
    while (i < gfmGenArr_getUsed(pCtx->pTexEvs)) {
        textEvent *pEv;

        pEv = gfmGenArr_getObject(pCtx->pTexEvs, i);

        rv = gfmObject_update(pEv->pSelf, pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);

        rv = gfmQuadtree_collideObject(pGame->pQt, pEv->pSelf);
        ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            rv = collide_run();
            ASSERT(rv == GFMRV_OK, rv);
        }

        i++;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the currently display text, if any
 *
 * @param  [ in]pCtx The text manager
 */
gfmRV textManager_postUpdate(textManager *pCtx) {
    gfmRV rv;

    rv = gfmText_didFinish(pCtx->pText);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_TRUE) {
        if (pCtx->pCurEv) {
            int elapsed;

            rv = gfm_getElapsedTime(&elapsed, pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);
            pCtx->elapsed += elapsed;

            if (pCtx->elapsed > pCtx->pCurEv->ttl) {
                pCtx->pCurEv = 0;
                pCtx->elapsed = 0;
            }
        }
        if (!pCtx->pCurEv && pCtx->pQueue) {
            char *pStr;
            int len;

            pCtx->pCurEv = pCtx->pQueue;
            pCtx->pQueue = pCtx->pQueue->pNext;

            pCtx->pCurEv = 0;

            rv = gfmString_getString(&pStr, pCtx->pCurEv->pString);
            ASSERT(rv == GFMRV_OK, rv);
            rv = gfmString_getLength(&len, pCtx->pCurEv->pString);
            ASSERT(rv == GFMRV_OK, rv);

            rv = gfmText_setText(pCtx->pText, pStr, len, 0/*doCopy*/);
            ASSERT(rv == GFMRV_OK, rv);
        }
    }

    rv = gfmText_update(pCtx->pText, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Render the currently display text, if any
 *
 * @param  [ in]pCtx The text manager
 */
gfmRV textManager_draw(textManager *pCtx) {
    gfmRV rv;

    if (!pCtx->pCurEv) {
        return GFMRV_OK;
    }

    if (pCtx->windowed) {
        /* TODO Draw window */
    }

    rv = gfmText_draw(pCtx->pText, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

