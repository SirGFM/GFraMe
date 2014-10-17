/**
 * @include/GFraMe/GFraMe_util.h
 */
#ifndef __GFRAME_UTIL_H
#define __GFRAME_UTIL_H

#include <GFraMe/GFraMe_error.h>

/**
 * Super complex integration routine
 * @param	val	Value to be integrated
 * @param	time	Time the value should be integrated
 */
double GFraMe_util_integrate(double val, double time);

/**
 * Return the absolute of a double
 * @param	val	Value to have its absolute computed
 * @return	Absolute value computed
 */
double GFraMe_util_absd(double val);

/**
 * Return the square root of a double
 * @param	val	Value to have its square root calculated
 * @return	Square root computed
 */
double GFraMe_util_sqrtd(double val);

/**
 * Return an pseudo-random number.
 * @return	The pseudo-random integer
 */
int GFraMe_util_randomi();

/**
 * Return a strings length.
 */
int GFraMe_util_strlen(const char *str);

/**
 * Compare two strings. If they are equal, return GFraMe_ok, GFraMe_failed
 *otherwise.
 */
GFraMe_ret GFraMe_util_strcmp(const char *str1, const char *str2);

/**
 * Concatenate two string. At most 'len' characters shall be appended to 'dst'.
 *'dst' must point to the last character on the destiny buffer.
 * The value returned is a pointer to the final position on the appended string,
 *which allows for easy multiple calls to this function.
 */
char* GFraMe_util_strcat(char *dst, char *src, int *len);

/**
 * Convert a string into a data buffer, to easily display messages
 * through a tilemap.
 * 
 * @param	*data	Buffer to be modified with the string's data
 * @param	*str	String to be converted to tiles
 * @param	first_tile	First ASCII character ('!') posiiton on the spriteset
 * @return	First position, on the buffer, after the string.
 */
char* GFraMe_str2tiles(char *data, char *str, int first_tile);

/**
 * Open an browser on the desired webpage.
 * If run in an unsupported platform (as iOS), it'll return
 *GFraMe_platform_not_supported. Otherwise, it'll return GFraMe_ok, even if the
 *browser couldn't be opened.
 */
GFraMe_ret GFraMe_util_open_browser(char *url);

/**
 * Interpolate linearly between two number.
 */
double GFraMe_util_lerp(int a, int b, double time);

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
char* GFraMe_util_get_local_path(char *str, int *len);

#endif
