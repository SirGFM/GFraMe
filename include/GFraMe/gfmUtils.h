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

#endif /* __GFMUTILS_H__ */

