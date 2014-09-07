/**
 * @include/GFraMe/GFraMe_tween.h
 */
#ifndef __GFRAME_TWEEN_H
#define __GFRAME_TWEEN_H

// Forward declaration
struct stGFraMe_object;
typedef struct stGFraMe_object GFraMe_object;

enum enGFraMe_tween_ret {
	GFraMe_tween_ret_ok = 0,
	GFraMe_tween_ret_complete,
	GFraMe_tween_ret_failed,
	GFraMe_tween_ret_stopped,
};
typedef enum enGFraMe_tween_ret GFraMe_tween_ret;

enum enGFraMe_tween_type {
	GFraMe_tween_lerp
};
typedef enum enGFraMe_tween_type GFraMe_tween_type;

struct stGFraMe_tween {
	int fromX;
	int fromY;
	int toX;
	int toY;
	GFraMe_tween_type type;
	double time;
	double maxTime;
};
typedef struct stGFraMe_tween GFraMe_tween;

void GFraMe_tween_clear(GFraMe_tween *tw);
void GFraMe_tween_init(GFraMe_tween *tw, int fromX, int fromY, int toX,
					   int toY, double time, GFraMe_tween_type type);
GFraMe_tween_ret GFraMe_tween_update(GFraMe_tween *tw, double elapsed);

GFraMe_tween_ret GFraMe_tween_set_obj(GFraMe_tween *tw, GFraMe_object *obj);

int GFraMe_tween_is_complete(GFraMe_tween *tw);

#endif

