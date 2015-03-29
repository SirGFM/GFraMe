/**
 * @include/GFraMe/GFraMe_event.h
 */
#ifndef __GFRAME_EVENT_H_
#define __GFRAME_EVENT_H_

#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_controller.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_pointer.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_keys.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_events.h>

#define __dt__			__FILE__##__gframe_delta_time
#define __lasttime__	__FILE__##__gframe_event_lasttime
#define	__updacc__		__FILE__##__gframe_upd_acc
#define __drawacc__		__FILE__##__gframe_draw_acc
#define GFraMe_event_elapsed	__FILE__##__gframe_elapsed_time

#define GFraMe_event_setup() \
	static Uint32 __dt__; \
	static Uint32 __lasttime__; \
	static GFraMe_accumulator __updacc__; \
	static GFraMe_accumulator __drawacc__; \
	static int GFraMe_event_elapsed

#define GFraMe_event_init(update_fps, draw_fps) \
	__lasttime__ = SDL_GetTicks(); \
	GFraMe_accumulator_init_fps(&__updacc__, update_fps, 6); \
	GFraMe_accumulator_init_fps(&__drawacc__, draw_fps, 1); \

#define GFraMe_event_begin() \
	SDL_Event event; \
	GFraMe_SDLassertRet(SDL_WaitEvent(&event) == 1, "Failed while waiting for events", __gframe_event_err_); \
	while (1) { \
		switch (event.type) { \
			/* 'Null' case*/ \
			case 0:

#define GFraMe_event_on_timer() \
			/* Close previous case */ \
			break; \
			/* Check if it's a timer event*/ \
			case SDL_USEREVENT: \
				/* Calculate elapsed time from previous frame */ \
				__dt__ = event.user.timestamp - __lasttime__; \
				__lasttime__ = event.user.timestamp; \
				GFraMe_accumulator_update(&__updacc__, __dt__); \
				GFraMe_accumulator_update(&__drawacc__, __dt__); \

#define GFraMe_event_on_mouse_up() \
			break; \
			case SDL_MOUSEBUTTONUP: \
				GFraMe_pointer_pressed = 0; \
				GFraMe_pointer_finger_ev = 0

#define GFraMe_event_on_mouse_down() \
			break; \
			case SDL_MOUSEBUTTONDOWN: \
				GFraMe_pointer_pressed = 1; \
				GFraMe_pointer_finger_ev = 0

#define GFraMe_event_on_mouse_moved() \
			break; \
			case SDL_MOUSEMOTION: \
				GFraMe_pointer_x = (event.motion.x - GFraMe_buffer_x) / GFraMe_screen_ratio_h; \
				if (GFraMe_pointer_x < 0) \
					GFraMe_pointer_x = 0; \
				else if (GFraMe_pointer_x > GFraMe_buffer_w) \
					GFraMe_pointer_x = GFraMe_buffer_w - 1; \
				GFraMe_pointer_y = (event.motion.y - GFraMe_buffer_y) / GFraMe_screen_ratio_v; \
				if (GFraMe_pointer_y < 0) \
					GFraMe_pointer_y = 0; \
				else if (GFraMe_pointer_y > GFraMe_buffer_h) \
					GFraMe_pointer_y = GFraMe_buffer_h - 1; \
				GFraMe_pointer_finger_ev = 0

#define GFraMe_event_on_finger_down() \
			break; \
			case SDL_FINGERDOWN: \
				GFraMe_pointer_x = (int)((event.tfinger.x * GFraMe_window_w - GFraMe_buffer_x) / GFraMe_screen_ratio_h); \
				GFraMe_pointer_y = (int)((event.tfinger.y * GFraMe_window_h - GFraMe_buffer_y) / GFraMe_screen_ratio_v); \
				GFraMe_pointer_pressed = 1; \
				GFraMe_pointer_finger_ev = 1

#define GFraMe_event_on_finger_up() \
			break; \
			case SDL_FINGERUP: \
				/* GFraMe_pointer_x = -1; */ \
				/* GFraMe_pointer_y = -1; */ \
				GFraMe_pointer_pressed = 0; \
				GFraMe_pointer_finger_ev = 1

#define GFraMe_event_on_key_down() \
			break; \
			case SDL_KEYDOWN: \
                GFraMe_key_upd(&event, 1)

#define GFraMe_event_on_controller() \
            break; \
            case SDL_CONTROLLERDEVICEADDED: \
            case SDL_CONTROLLERDEVICEREMOVED: \
            case SDL_CONTROLLERDEVICEREMAPPED: \
            case SDL_CONTROLLERAXISMOTION: \
            case SDL_CONTROLLERBUTTONDOWN: \
            case SDL_CONTROLLERBUTTONUP: \
                GFraMe_controller_update(&event)

#define GFraMe_event_on_key_up() \
			break; \
			case SDL_KEYUP: \
                GFraMe_key_upd(&event, 0)

#define GFraMe_event_on_quit() \
			break; \
			case SDL_QUIT: 

#define GFraMe_event_on_bg() \
			break; \
			case SDL_APP_WILLENTERBACKGROUND:

#define GFraMe_event_on_fg() \
			break; \
			case SDL_APP_WILLENTERFOREGROUND:

#define GFraMe_event_end() \
			break; \
			default: break;\
		} \
		if (SDL_PollEvent(&event) == 0) { \
__gframe_event_err_: \
			break; \
		} \
	}

#define GFraMe_event_update_begin() \
	while (GFraMe_accumulator_loop(&__updacc__)) { \
		GFraMe_event_elapsed = __updacc__.timeout

#define GFraMe_event_update_end() \
	}

#define GFraMe_event_draw_begin() \
	if (GFraMe_accumulator_loop(&__drawacc__)) { \
		GFraMe_init_render()

#define GFraMe_event_draw_end() \
		GFraMe_finish_render(); \
	}

#endif

