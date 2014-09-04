/**
 * @src/score.h
 */
#ifndef __SCORE_H_
#define __SCORE_H_

#include <GFraMe/GFraMe_error.h>

void score_init();
void score_update(int ms);
void score_inc(int val);
GFraMe_ret score_draw();

void highscore_init();
void highscore_update(int ms);

#endif

