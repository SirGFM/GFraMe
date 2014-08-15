/**
 * @src/gframe_util.c
 */
#include <GFraMe/GFraMe_util.h>
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

