/**
 * @file include/GFraMe/gfmAssert.h
 * 
 * Assert module. Defines some macros for jumping to the end of a function on
 * error and log, if desired.
 * All 'ASSERT*' will jump to a label '__ret' and write the error code to 'rv'.
 * To jump to a specific label, use 'CASSERT*'
 */
#ifndef __GFMASSERT_H__
#define __GFMASSERT_H__

/**
 * Check if a statement is true and jump to an error handling part of the code
 * if not;
 * 
 * @param stmt The statement to be evaluated
 * @param err  The return value
 */
#define ASSERT(stmt, err) \
  do { \
    if (!(stmt)) { \
      rv = err; \
      goto __ret; \
    } \
  } while (0)

/**
 * Check if a statement is true and jump to an error handling part of the code
 * if not; No return value is set on this 'function'
 * 
 * @param stmt The statement to be evaluated
 */
#define ASSERT_NR(stmt) \
  do { \
    if (!(stmt)) { \
      goto __ret; \
    } \
  } while (0)

/**
 * Check if a statement is true and jump to an error handling part of the code
 * if not;
 * 
 * @param stmt  The statement to be evaluated
 * @param err   The return value
 * @param label Label that should be jumped to
 */
#define CASSERT(stmt, err, label) \
  do { \
    if (!(stmt)) { \
      rv = err; \
      goto label; \
    } \
  } while (0)

/**
 * Check if a statement is true and jump to an error handling part of the code
 * if not; No return value is set on this 'function'
 * 
 * @param stmt  The statement to be evaluated
 * @param label Label that should be jumped to
 */
#define CASSERT_NR(stmt, label) \
  do { \
    if (!(stmt)) { \
      goto label; \
    } \
  } while (0)

#endif


#endif

