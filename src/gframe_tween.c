/**
 * @src/gframe_tween.c
 */
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_tween.h>
#include <GFraMe/GFraMe_util.h>

void GFraMe_tween_clear(GFraMe_tween *tw) {
	tw->fromX = 0;
	tw->fromY = 0;
	tw->toX = 0;
	tw->toY = 0;
	tw->time = 0.0;
	tw->maxTime = 0.0;
}

void GFraMe_tween_init(GFraMe_tween *tw, int fromX, int fromY, int toX,
					   int toY, double time, GFraMe_tween_type type) {
	tw->fromX = fromX;
	tw->fromY = fromY;
	tw->toX = toX;
	tw->toY = toY;
	tw->time = 0.0;
	tw->type = type;
	tw->maxTime = time;
}

GFraMe_tween_ret GFraMe_tween_update(GFraMe_tween *tw, double elapsed) {
	if (tw->time >= tw->maxTime)
		return GFraMe_tween_ret_stopped;
	tw->time += elapsed;
	if (tw->time >= tw->maxTime)
		return GFraMe_tween_ret_complete;
	return GFraMe_tween_ret_ok;
}

GFraMe_tween_ret GFraMe_tween_set_obj(GFraMe_tween *tw, GFraMe_object *obj) {
	double X, Y;
	switch (tw->type) {
		case GFraMe_tween_lerp:
			X = GFraMe_util_lerp(tw->fromX, tw->toX, tw->time /tw->maxTime);
			Y = GFraMe_util_lerp(tw->fromY, tw->toY, tw->time /tw->maxTime);
		break;
		default:
			GFraMe_log("Tween mode not implemented");
			return GFraMe_tween_ret_failed;
		break;
	}
	GFraMe_object_set_pos(obj, X, Y);
	return GFraMe_tween_ret_ok;
}

int GFraMe_tween_is_complete(GFraMe_tween *tw) {
	return tw->maxTime != 0.0 && tw->time >= tw->maxTime;
}

