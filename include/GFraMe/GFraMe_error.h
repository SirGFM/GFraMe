/**
 * @include/GFraMe/GFraMe_error.h
 */
#ifndef __GFRAME_ERROR_H_
#define __GFRAME_ERROR_H_

#include <SDL2/SDL_log.h>

enum enGFraMe_ret {
	GFraMe_ret_ok = 0,
	GFraMe_ret_sdl_init_failed,
	GFraMe_ret_bad_param,
	GFraMe_ret_window_creation_failed,
	GFraMe_ret_renderer_creation_failed,
	GFraMe_ret_backbuffer_creation_failed,
	GFraMe_ret_texture_creation_failed,
	GFraMe_ret_timer_creation_failed,
	GFraMe_ret_fps_req_low,
	GFraMe_ret_num_display_failed,
	GFraMe_ret_display_modes_failed,
	GFraMe_ret_new_acc_frame,
	GFraMe_ret_anim_new_frame,
	GFraMe_ret_anim_finished,
	GFraMe_ret_file_not_found,
	GFraMe_ret_memory_error,
	GFraMe_ret_read_file_failed,
	GFraMe_ret_no_overlap,
	GFraMe_ret_render_failed,
	GFraMe_ret_invalid_texture,
	GFraMe_ret_stop_timer_failed
};
typedef enum enGFraMe_ret GFraMe_ret;

/**
 * Logs a message and it's source/time info;
 * works just like printf
 */
#define GFraMe_log(message, ...) \
	SDL_Log(" @%s:%d --- "message, __FILE__, __LINE__, ##__VA_ARGS__)

/**
 * Check if an error occured and switch execution to an "error control part"
 * @param	condition	What's being evaluated that must be valid/true
 * @param	err_msg	Message to be logged on error
 * @param	return_expression	Expression to be executed on error, should set a return value
 * @param	label	Where to code should jump to (either to fix it or to exit the function)
 */
#define GFraMe_assertRV(condition, err_msg, return_expression, label) \
	do { \
		if (!(condition)) { \
			GFraMe_log(err_msg); \
			return_expression; \
			goto label; \
		} \
	} while (0)

/**
 * Check if an error occured and switch execution to an "error control part";
 * also logs SDL info about the error
 * @param	condition	What's being evaluated that must be valid/true
 * @param	err_msg	Message to be logged on error
 * @param	return_expression	Expression to be executed on error, should set a return value
 * @param	label	Where to code should jump to (either to fix it or to exit the function)
 */
#define GFraMe_SDLassertRV(condition, err_msg, return_expression, label) \
	do { \
		if (!(condition)) { \
			GFraMe_log(err_msg); \
			/* TODO log SDL info */ \
			return_expression; \
			goto label; \
		} \
	} while (0)

/**
 * Check if an error occured and switch execution to an "error control part";
 * this considers the return value was already set (or is uneeded)
 * @param	condition	What's being evaluated that must be valid/true
 * @param	err_msg	Message to be logged on error
 * @param	label	Where to code should jump to (either to fix it or to exit the function)
 */
#define GFraMe_assertRet(condition, err_msg, label) \
	do { \
		if (!(condition)) { \
			GFraMe_log(err_msg); \
			goto label; \
		} \
	} while (0)

/**
 * Check if an error occured and switch execution to an "error control part";
 * this considers the return value was already set (or is uneeded);
 * also logs SDL info about the error
 * @param	condition	What's being evaluated that must be valid/true
 * @param	err_msg	Message to be logged on error
 * @param	label	Where to code should jump to (either to fix it or to exit the function)
 */
#define GFraMe_SDLassertRet(condition, err_msg, label) \
	do { \
		if (!(condition)) { \
			GFraMe_log(err_msg); \
			/* TODO log SDL info */ \
			goto label; \
		} \
	} while (0)

#endif

