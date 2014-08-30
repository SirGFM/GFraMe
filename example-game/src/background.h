/**
 * src/background.h
 */
#ifndef __BACKGROUND_H
#define __BACKGROUND_H

#include <GFraMe/GFraMe_object.h>

#define	BG_W	40
#define BG_H	10
#define	FLOOR_W	20
#define	FLOOR_H	6
#define FLOOR_Y	144
#define BASE_BG_TILE	32
#define BASE_FLOOR_TILE	21

void background_init();
void background_draw();
GFraMe_object* background_get_object();

#endif

