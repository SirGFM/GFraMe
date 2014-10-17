/**
 * @src/gframe_util.c
 */
#include <GFraMe/GFraMe.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_util.h>
#include <SDL2/SDL_filesystem.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_system.h>
#include <math.h>
#include <stdlib.h>

/**
 * Super complex integration routine
 * @param	val	Value to be integrated
 * @param	time	Time the value should be integrated
 */
double GFraMe_util_integrate(double val, double time) {
	// Use Euler! (lol)
	return val*time;
}

/**
 * Return the absolute of a double
 * @param	val	Value to have its absolute computed
 * @return	Absolute value computed
 */
double GFraMe_util_absd(double val) {
	// If it's positive, simply return the number
	if (val >= 0)
		return val;
	// Otherwise, negate it and return
	return -val;
}

/**
 * Return the square root of a double
 * @param	val	Value to have its square root calculated
 * @return	Square root computed
 */
double GFraMe_util_sqrtd(double val) {
	return (double)sqrt((float)val);
}

/**
 * Return an pseudo-random number.
 * @return	The pseudo-random integer
 */
int GFraMe_util_randomi() {
	// TODO change the generator!
	return rand();
}

/**
 * Return a strings length.
 */
int GFraMe_util_strlen(const char *str) {
	int len = 0;
	while (*(str++)) len++;
	return len;
}

/**
 * Compare two strings. If they are equal, return GFraMe_ok, GFraMe_failed
 *otherwise.
 */
GFraMe_ret GFraMe_util_strcmp(const char *str1, const char *str2) {
	GFraMe_ret rv;
	
	// Loop while both chars are equal and not '\0'
	while (*str1 && *str2 && *str1 == *str2) {
		str1++;
		str2++;
	}
	
	// If both are '\0', both string were fully read and are equal
	if (!*str1 && !*str2)
		rv = GFraMe_ret_ok;
	else
		rv = GFraMe_ret_failed;
	
	return rv;
}

/**
 * Concatenate two string. At most 'len' characters shall be appended to 'dst'.
 *'dst' must point to the last character on the destiny buffer.
 * The value returned is a pointer to the final position on the appended string,
 *which allows for easy multiple calls to this function.
 */
char* GFraMe_util_strcat(char *dst, char *src, int *len) {
	// Copy from src to dst until the last character was reached or there's no
	//more space in 'dst'
	while (*src && *len > 0) {
		*dst = *src;
		src++;
		dst++;
		(*len)--;
	}
	
	// If there's still space in 'dst', put a '\0' on its end
	if (*len > 0)
		*dst = '\0';
	
	return dst;
}

/**
 * Convert a string into a data buffer, to easily display messages
 * through a tilemap.
 * 
 * @param	*data	Buffer to be modified with the string's data
 * @param	*str	String to be converted to tiles
 * @param	first_tile	First ASCII character ('!') posiiton on the spriteset
 * @return	First position, on the buffer, after the string.
 */
char* GFraMe_str2tiles(char *data, char *str, int first_tile) {
	while (*str) {
		*data = (char)(*str - '!' + first_tile);
		data++;
		str++;
	}
	return data;
}

/**
 * Open an browser on the desired webpage.
 * If run in an unsupported platform (as iOS), it'll return
 *GFraMe_platform_not_supported. Otherwise, it'll return GFraMe_ok, even if the
 *browser couldn't be opened.
 */
GFraMe_ret GFraMe_util_open_browser(char *url) {
	char buf[1024];
	char *tmp;
	int len = 1024;
	
	// Get the current platform
	const char *plat = SDL_GetPlatform();
	
	// Set which command should be executed to open the browser
	if (GFraMe_util_strcmp(plat, "Windows") == GFraMe_ret_ok)
		tmp = GFraMe_util_strcat(buf, "start ", &len);
	else if (GFraMe_util_strcmp(plat, "Linux") == GFraMe_ret_ok)
		tmp = GFraMe_util_strcat(buf, "xdg-open ", &len);
	else if (GFraMe_util_strcmp(plat, "Android") == GFraMe_ret_ok)
		tmp = GFraMe_util_strcat(buf, "am start -a android.intent.action.VIEW "
			"-d ", &len);
	else
		return GFraMe_platform_not_supported;
	
	// Append the url to the command
	tmp = GFraMe_util_strcat(tmp, url, &len);
	// and execute it
	system(buf);
	
	return GFraMe_ret_ok;
}

/**
 * Interpolate linearly between two number.
 */
double GFraMe_util_lerp(int a, int b, double time) {
	return (double)a * (1 - time) + (double)b * time;
}

/**
 * Get the directory to this application's local files. It's set according to
 *the organization and title set when initializing the game. The actual
 *directory depends on the current OS:
 *   - /data/data/organization|title/, on Android
 *   - %APPDATA%/organization|title/, on Windows
 *   - ~/.local/shared/organization|title/, on Linux
 * 'str' points to the buffer where it'll be written, while 'len' must have how
 *many free characters there are.
 */
char* GFraMe_util_get_local_path(char *str, int *len) {
	char *tmp = str;
	
#if defined(__ANDROID__) && __ANDROID__
	tmp = GFraMe_util_strcat(tmp, SDL_AndroidGetInternalStoragePath(), len);
#else
	char *sdl_path = SDL_GetPrefPath(GFraMe_org, GFraMe_title);
	if (sdl_path) {
		tmp = GFraMe_util_strcat(tmp, sdl_path, len);
		SDL_free(sdl_path);
	}
#endif
	return tmp;
}

