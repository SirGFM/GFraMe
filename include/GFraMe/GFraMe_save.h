/**
 * @include/GFraMe/GFraMe_save.h
 */
#ifndef __GFRAME_SAVE_H_
#define __GFRAME_SAVE_H_

#include <SDL2/SDL_rwops.h>

#define GFraMe_save_max_len	512

#ifdef NEW_SAVE

typedef enum {
	CHAR,
	INT,
	BYTES,
} GFraMe_save_type;

#endif

/**
 * Possibles return code from save operations
 */
enum enGFraMe_save_ret {
	/**
	 * Function was successful
	 */
	GFraMe_save_ret_ok = 0,
	/**
	 * Reached end of file
	 */
	GFraMe_save_ret_eof,
	/**
	 * File is empty
	 */
	GFraMe_save_ret_empty,
	/**
	 * 
	 */
	GFraMe_save_ret_fno, // file not opened
	GFraMe_save_ret_id_not_found,
	GFraMe_save_ret_failed
};
typedef enum enGFraMe_save_ret GFraMe_save_ret;

enum enGFraMe_save_state {
	GFraMe_save_was_flushed = 0x0,
	GFraMe_save_did_write = 0x1,
	GFraMe_save_is_writing = 0x2,
	GFraMe_save_did_read = 0x3,
	GFraMe_save_is_reading = 0x8,
};
typedef enum enGFraMe_save_state GFraMe_save_state;

/**
 * Simple structure to store a save file info
 */
struct stGFraMe_save {
	/**
	 * Current 
	 */
	SDL_RWops *file;
	char filename[GFraMe_save_max_len];
	int size;
	GFraMe_save_state	state;
};
typedef struct stGFraMe_save GFraMe_save;

GFraMe_ret GFraMe_save_bind(GFraMe_save *sv, char *filename);
void GFraMe_save_close(GFraMe_save *sv);
void GFraMe_save_flush(GFraMe_save *sv, int force);
void GFraMe_save_clear(GFraMe_save *sv);
GFraMe_ret GFraMe_save_write(GFraMe_save *sv, char *id,
		void *data, int size, int count);
GFraMe_ret GFraMe_save_read(GFraMe_save *sv, char *id,
		void *data, int size, int count);

#endif

