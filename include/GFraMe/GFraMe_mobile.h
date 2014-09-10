/**
 * @include/GFraMe/GFraMe_mobile.h
 */
#ifndef __GFRAME_MOBILE_H_
#define __GFRAME_MOBILE_H_

enum enGFraMe_mobile_err {
	GFraMe_mobile_ok = 0,
	GFraMe_mobile_function_not_found
};
typedef enum enGFraMe_mobile_err GFraMe_mobile_err;

GFraMe_mobile_err GFraMe_mobile_call_void_function(char *functionName);

#endif

