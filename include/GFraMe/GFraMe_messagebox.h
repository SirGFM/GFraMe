/**
 * @include/GFraMe/GFraMe_messagebox.h
 */
#ifndef __GFRAME_MESSAGEBOX_H_
#define __GFRAME_MESSAGEBOX_H_

enum enGFraMe_button_ret {
	GFraMe_button_ret_ok,
	GFraMe_button_ret_cancel,
	GFraMe_button_ret_failed
};
typedef enum enGFraMe_button_ret GFraMe_button_ret;

/**
 * Create a message box with two options: "OK" and "Cancel"
 */
GFraMe_button_ret GFraMe_messagebox_OkCancel(const char *title,
				const char *body, const char *ok, const char *cancel);

#endif

