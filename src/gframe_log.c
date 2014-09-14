/**
 * @src/gframe_log.c
 */
#include <GFraMe/GFraMe.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_util.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_filesystem.h>
//#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_system.h>
#include <stdio.h>
#include <stdarg.h>

int GFraMe_log_to_file = 0;
char GFraMe_log_filename[GFraMe_log_max_filename];
//SDL_RWops *logfile;
FILE *logfile = NULL;

/**
 * Open file for logging
 *
 * @param	append	Whether should append to or overwrite the old log
 */
void GFraMe_log_init(int append) {
	int len = GFraMe_log_max_filename;
	char *tmp;
	// Create the file  path
	tmp = GFraMe_log_filename;
#if defined(__ANDROID__) && __ANDROID__
	tmp = GFraMe_util_strcat(tmp, SDL_AndroidGetInternalStoragePath(), &len);
#else
	char *sdl_path = SDL_GetPrefPath(GFraMe_org, GFraMe_title);
	if (sdl_path) {
		tmp = GFraMe_util_strcat(tmp, sdl_path, &len);
		SDL_free(sdl_path);
	}
#endif
	tmp = GFraMe_util_strcat(tmp, "gframe_log.txt", &len);
	// Open the file
	if (append)
		//logfile = SDL_RWFromFile(GFraMe_log_filename, "a");
		logfile = fopen(GFraMe_log_filename, "a");
	else
		//logfile = SDL_RWFromFile(GFraMe_log_filename, "w");
		logfile = fopen(GFraMe_log_filename, "w");
	if (logfile)
		GFraMe_log_to_file = 1;
}

void GFraMe_log_close() {
	if (logfile) {
		fclose(logfile);
		logfile = NULL;
	}
}

void GFraMe_log_private(char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	if (GFraMe_log_to_file && logfile)
		vfprintf(logfile, fmt, args);
	else
		vfprintf(stdout, fmt, args);
	va_end(args);
}

