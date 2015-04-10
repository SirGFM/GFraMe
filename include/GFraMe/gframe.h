/**
 * @include/GFraMe/gframe.h
 */
#ifndef __GFRAME_H_
#define __GFRAME_H_

#define GFraMe_major_version	0
#define GFraMe_minor_version	1
#define GFraMe_fix_version		0
#define GFraMe_version	"0.1.0"

/** 'Exports' the gfmCtx structure */
typedef struct stGFMCtx gfmCtx;
/** 'Exportable' size of gfmString */
const size_t sizeofGFMCtx;

/**
 * Alloc a new gfmContext
 * 
 * @param  ppCtx The allocated context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getNew(gfmCtx **ppCtx);

/**
 * Set the game's title and organization from static buffers
 * 
 * @param  pCtx    The game's context
 * @param  org     Statically allocated organization's name
 * @param  name    Statically allocated game's title (i.e., char var[] = "...")
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_ALREADY_SET,
 *                 GFMRV_ALLOC_FAILED
 */
#define gfm_setTitleStatic(pCtx, pOrg, pName) \
    gfm_setTitle(pCtx, pOrg, sizeof(pOrg)-1, pName, sizeof(pName)-1)

/**
 * Set the game's title and organization
 * 
 * @param  pCtx     The game's context
 * @param  pOrg     Organization's name (used by log and save file)
 * @param  orgLen   Organization's name's length
 * @param  pName    Game's title (also used as window's title)
 * @param  nameLen  Game's title's length
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_ALREADY_SET,
 *                  GFMRV_ALLOC_FAILED
 */
gfmRV gfm_setTitle(gfmCtx *pCtx, char *pOrg, int orgLen, char *pName,
        int nameLen);

/**
 * Get the game's title and organization
 * 
 * @param  ppOrg      Organization's name (used by log and save file)
 * @param  ppTitle    Game's title (also used as window's title)
 * @param  pCtx       The game's context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_NOT_SET
 */
gfmRV gframe_getTitle(char **ppOrg, char **ppTitle, gfmCtx *pCtx);

/* ========================================================================== */
/* |                                                                        | */
/* |  OLD STUFF                                                             | */
/* |                                                                        | */
/* ========================================================================== */

#include <GFraMe/GFraMe_screen.h>


/**
 * Whether there is OpenGL support or not
 */
extern int GFraMe_gl;

#define GFraMe_max_org_len	80
/**
 * "Organization" name. Is used as part of paths.
 */
extern char GFraMe_org[GFraMe_max_org_len];

#define GFraMe_max_game_title_len 80
/**
 * Game's title. Is used as part of paths.
 */
extern char GFraMe_title[GFraMe_max_game_title_len];

#define GFraMe_max_path_len	512
/**
 * Path to the directory where the game is running
 */
extern char GFraMe_path[GFraMe_max_path_len];

/**
 * Initialize SDL, already creating a window and a backbuffer.
 * @param	vw	Buffer's width (virtual width)
 * @param	vh	Buffer's height (virtual height)
 * @param	sw	Window's width (screen width); if 0, uses the device width
 * @param	sh	Window's height(screen height);if 0, uses the device height
 * @param	org	Organization's name (used by the log and save file)
 * @param	name	Game's name (also used as window's title)
 * @param	flags	Window creation flags
 * @param	fps		At how many frames per second the game should run;
 *				  notice that this is independent from update and render
 *				  rate, those should be set on each state
 * @param	log_to_file	Whether should log to a file or to the terminal
 * @param	log_append	Whether should overwrite or append to an existing log
 * @return	0 - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_init(int vw, int vh, int sw, int sh, char *org,
				char *name, GFraMe_window_flags flags, GFraMe_wndext *ext,
				int fps, int log_to_file, int log_append);

void GFraMe_quit();

#endif

