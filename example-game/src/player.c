/**
 * @src/player.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_util.h>
#include "global.h"
#include "player.h"

#define BASE_JUMP	200.0
/**
 * Player's sprite
 */
static GFraMe_sprite player;
/**
 * Target that the player will jump/dash toward
 */
static GFraMe_sprite tgt;

static double jump_speed;
static int cooldown;
static int combo;

void player_init() {
	// Draw debug!!
	GFraMe_draw_debug = 1;
	// Init the player itself
	GFraMe_sprite_init(&player, 10, 10, 8, 14, &gl_sset16, -4, -2);
	player.cur_tile = 8;
	player.obj.ay = 500;
	// Init the target
	GFraMe_sprite_init(&tgt, -16, -16, 16, 16, &gl_sset16, 0, 0);
	tgt.cur_tile = 9;
	tgt.id = 0;
	// Set the players initial jump speed
	jump_speed = BASE_JUMP;
	// Set the timer (for consecutive jump) and counter
	cooldown = 0;
	combo = 0;
}

void player_update(int ms) {
	if (player.obj.hit & GFraMe_direction_down) {
		if (player.obj.vy >= 0.0 && player.obj.ax == 0.0f &&
			player.obj.vx != 0.0)
			player.obj.ax = -player.obj.vx * 4.0;
		else if(player.obj.ax != 0.0 &&
				GFraMe_util_absd(player.obj.vx) <= 16.0) {
			player.obj.vx = 0.0;
			player.obj.ax = 0.0;
		}
	}
	else if (tgt.id) {
		double X = tgt.obj.x - player.obj.x;
		double Y = tgt.obj.y - player.obj.y;
		double dist = GFraMe_util_sqrtd(X*X + Y*Y);
		player.obj.vx = player.obj.vx / 4.0 + X / dist * jump_speed;
		player.obj.vy = Y / dist * jump_speed;
	}
	GFraMe_sprite_update(&player, ms);
	if (cooldown > 0)
		cooldown -= ms;
	if (tgt.id > 0)
		GFraMe_sprite_update(&tgt, ms);
}

void player_draw() {
	GFraMe_sprite_draw(&player);
	if (tgt.id)
		GFraMe_sprite_draw(&tgt);
}

int player_slowdown() {
	return cooldown > 0 || (!(player.obj.hit && GFraMe_direction_down) &&
			GFraMe_util_absd(player.obj.vy) < 32.0 && !tgt.id);
}

void player_on_ground() {
	player.obj.vy = 0.0;
	tgt.id = 0;
	combo = 0;
	cooldown = 0;
	jump_speed = BASE_JUMP;
}

GFraMe_ret player_on_squash() {
	// Check if the player did squash an enemy
	if (player.obj.y + player.obj.hitbox.cy + player.obj.hitbox.hh >= 158 ||
		!(player.obj.hit & GFraMe_direction_down))
		return GFraMe_ret_failed;
	// Increase its speed
	if (jump_speed < 500)
		jump_speed += 25;
	// Sets the cooldown for fast jumping
	if (combo < 20)
		cooldown = 300 - combo * 10;
	else
		cooldown = 100;
	// Increment the combo counter
	combo++;
	// Make the player jump
	player.obj.vy = -jump_speed;
	tgt.id = 0;
	return GFraMe_ret_ok;
}

void player_set_target(int X, int Y) {
	if (Y > 164 && (GFraMe_util_absd(player.obj.vy) < 64.0 ||
					cooldown > 0)) {
		tgt.id = 1;
		GFraMe_object_set_pos(&tgt.obj, X, Y);
		cooldown = 0;
	}
}

GFraMe_ret player_jump(int X) {
	// Check if the player is jumping
	if (!(player.obj.hit & GFraMe_direction_down))
		return GFraMe_ret_failed;
	// Otherwise, make it jump
	player.obj.vy = -jump_speed;
	player.obj.vx = X - player.obj.x;
	player.obj.ax = 0.0;
	return GFraMe_ret_ok;
}

GFraMe_object *player_get_object() {
	return &player.obj;
}

