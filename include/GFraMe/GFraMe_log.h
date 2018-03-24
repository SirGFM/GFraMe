/**
 * @include/GFraMe/GFraMe_log.h
 */
#ifndef __GFRAME_LOG_H_
#define __GFRAME_LOG_H_

#define GFraMe_log_max_filename 512

void GFraMe_log_init(int append);
void GFraMe_log_close();
void GFraMe_log_private(char * fmt, ...);

#if defined(DGFRAME_DEBUG)
#  define GFraMe_new_log(message, ...) \
     GFraMe_log_private(" "message"\n", ##__VA_ARGS__)
#  define GFraMe_error_log(message, ...) \
     GFraMe_log_private("@%s:%d - "message"\n",__FILE__,__LINE__, ##__VA_ARGS__)
#else
#  define GFraMe_new_log(message, ...)
#  define GFraMe_error_log(message, ...)
#endif

#endif

/**
 * Files where logging was updated:
 *  - gframe.c
 *  - gframe_audio_player.c
 *  - gframe_screen.c
 * Files where logging was deemed unnecessary:
 *  - gframe_accumulator.c
 *  - gframe_animation.c
 */
 
