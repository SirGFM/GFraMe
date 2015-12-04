/**
 * @src/multiplier.h
 */
#ifndef __MULTIPLIER_H_
#define __MULTIPLIER_H_

#include <GFraMe/GFraMe_error.h>

void multi_init();
void multi_update(int ms);
GFraMe_ret multi_draw();
double multi_get();
void multi_inc();
void multi_half();
void multi_reset();

#endif

