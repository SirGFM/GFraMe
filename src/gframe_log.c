/**
 * @src/gframe_log.c
 */
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_util.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_filename.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_system.h>

#define MAX_FILENAME	512

int GFraMe_log_to_file = 0;
char GFraMe_log_filename[MAX_FILENAME];
SDL_RWops *logfile;

/**
 * Open file for logging
 *
 * @param	append	Whether should append to or overwrite the old log
 */
void GFraMe_log_init(int append) {
	int len = MAX_FILENAME;
	char *tmp;
	// Create the file  path
	tmp = GFraMe_log_filename;
#if defined(__ANDROID__) && __ANDROID__
	tmp = GFraMe_util_strcat(tmp, SDL_AndroidGetInternalStoragePath(), &len);
#else
	char *sdl_path = SDL_GetPrefPath(org, game);
	if (sdl_path) {
		tmp = GFraMe_util_strcat(tmp, sdl_path, &len);
		SDL_free(sdl_path);
	}
#endif
	tmp = GFraMe_util_strcat(tmp, "gframe_log.txt", &len);
	// Open the file
	if (append)
		logfile = SDL_RWFromFile(GFraMe_log_filename, "a");
	else
		logfile = SDL_RWFromFile(GFraMe_log_filename, "w");
	if (logfile)
		GFraMe_log_to_file = 1;
}

