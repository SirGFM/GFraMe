/**
 * @src/gframe_messagebox.c
 */
#include <GFraMe/GFraMe_messagebox.h>
#include <SDL2/SDL_messagebox.h>

/**
 * Get the window reference; from gframe_screen.c
 */
extern SDL_Window *GFraMe_screen_get_window();

/**
 * Create a message box with two options: "OK" and "Cancel"
 */
GFraMe_button_ret GFraMe_messagebox_OkCancel(const char *title,
				const char *body, const char *ok, const char *cancel) {
	int res, button;
	// Create button info for SDL
	const SDL_MessageBoxButtonData buttons[] = {
		{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
		 GFraMe_button_ret_ok, ok},
		{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
		 GFraMe_button_ret_cancel, cancel},
	};
	// Create message box data
	SDL_MessageBoxData data = {SDL_MESSAGEBOX_INFORMATION,
		GFraMe_screen_get_window(), title, body, 2, buttons, NULL
	};
	res = SDL_ShowMessageBox(&data, &button);
	if (res == -1)
		return GFraMe_button_ret_failed;
	return button;
}

