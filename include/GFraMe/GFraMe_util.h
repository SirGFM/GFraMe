/**
 * @include/GFraMe/GFraMe_util.h
 */
#ifndef __GFRAME_UTIL_H
#define __GFRAME_UTIL_H

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

int GFraMe_util_strlen(const char *str);
int GFraMe_util_strcmp(const char *str1, const char *str2);
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
 * Open a web browser with the requested url;
 * only works on Windows, Linux and Android
 */
void GFraMe_util_open_browser(char *url);

#endif

