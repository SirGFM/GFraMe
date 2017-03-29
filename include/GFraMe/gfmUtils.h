/**
 * @file include/GFraMe/gfmUtils.h
 * 
 * Module with diverse utilities
 */
#ifndef __GFMUTILS_H__
#define __GFMUTILS_H__

#include <GFraMe/gfmError.h>

/**
 * Check if a given value is a power of two
 * 
 * @param  n The number
 * @return   GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmUtils_isPow2(int n);

#if !defined(DISABLE_CUSTOM_STRNLEN)
#  if defined(__WIN32) || defined(__WIN32__) || defined(__APPLE__) || defined(__MACH__)
/**
 * Count how many characters are in a string, up to a limit
 *
 * @param  [ in]pStr   The string
 * @param  [ in]maxLen How many characters there may be on the string, at most
 * @return             The minimum of the string's length and maxLen
 */
int strnlen(const char *pStr, int maxLen);
#  endif /* defined(__WIN32) || defined(__WIN32__) || defined(__APPLE__) || defined(__MACH__) */
#endif /* !defined(DISABLE_CUSTOM_STRNLEN) */

#endif /* __GFMUTILS_H__ */

