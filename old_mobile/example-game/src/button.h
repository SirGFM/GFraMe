/**
 * @src/button.h
 */
#ifndef __BUTTON_H_
#define __BUTTON_H_

#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>

enum enBtState {
	RELEASED,
	OVER,
	PRESSED
};
typedef enum enBtState btState;

struct stButton {
	GFraMe_sprite base;
	GFraMe_sprite label;
	btState state;
	int wasPressed;
	int justReleased;
	int released;
	int over;
	int pressed;
};
typedef struct stButton Button;

void button_init(Button *bt, int x, int y, int rel, int over,
				 int press, int label);
void button_update(Button *bt, int ms);
void button_draw(Button *bt);

#endif

