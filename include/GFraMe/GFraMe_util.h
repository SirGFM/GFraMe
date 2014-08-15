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

#endif

