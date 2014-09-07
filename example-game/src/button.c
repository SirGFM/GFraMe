/**
 * @src/button.c
 */
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_spriteset.h>
#include "global.h"
#include "button.h"

void button_init(Button *bt, int x, int y, int rel, int over,
				 int press, int label) {
	GFraMe_draw_debug = 1;
	GFraMe_sprite_init(&bt->base, x, y, 44, 44, &gl_sset32, 6, 6);
	bt->justReleased = 0;
	bt->wasPressed = 0;
	bt->released = rel;
	bt->over = over;
	bt->pressed = press;
	if (label >= 0) {
		GFraMe_sprite_init(&bt->label, x+14, y+12, 44, 44, &gl_sset16, 0, 0);
		bt->label.cur_tile = label;
	}
	else
		bt->label.is_visible = 0;
	bt->base.cur_tile = rel;
}

void button_update(Button *bt, int ms, int mouseX, int mouseY,
				   int pressed) {
	int isOver;
	
	bt->justReleased = 0;
	isOver = mouseX >= bt->base.obj.x
		  && mouseX <= bt->base.obj.x + bt->base.obj.hitbox.hw * 2
		  && mouseY >= bt->base.obj.y
		  && mouseY <= bt->base.obj.y + bt->base.obj.hitbox.hh * 2;
	
	if (!isOver) {
		bt->state = RELEASED;
		bt->label.offset_y = 0;
		bt->base.cur_tile = bt->released;
	}
	else if (pressed) {
		bt->state = PRESSED;
		bt->base.cur_tile = bt->pressed;
		bt->label.offset_y = 2;
	}
	else if (bt->wasPressed) {
		bt->justReleased = 1;
	}
	else if (bt->state != OVER) {
		bt->state = OVER;
		bt->base.cur_tile = bt->over;
		bt->label.offset_y = 1;
	}
	bt->wasPressed = isOver && pressed;
}

void button_draw(Button *bt) {
	GFraMe_sprite_draw(&bt->base);
	if (bt->label.is_visible)
		GFraMe_sprite_draw(&bt->label);
}

