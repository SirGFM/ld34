/**
 * Small window used for rendering texts
 *
 * @file include/ld34/textManager.h
 */
#ifndef __TEXTMANAGER_STRUCT__
#define __TEXTMANAGER_STRUCT__

typedef struct stTextManager textManager;

#endif /* __TEXTMANAGER_STRUCT__ */

#ifndef __TEXTMANAGER_H__
#define __TEXTMANAGER_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>

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

#endif /* __TEXTMANAGER_H__ */

