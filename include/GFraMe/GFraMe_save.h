/**
 * @include/GFraMe/GFraMe_save.h
 */
#ifndef __GFRAME_SAVE_H_
#define __GFRAME_SAVE_H_

#include <SDL2/SDL_rwops.h>

#define GFraMe_save_max_len	512
#define GFraMe_save_id_max_len	256

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
	int version;
	GFraMe_save_state	state;
};
typedef struct stGFraMe_save GFraMe_save;

/**
 * Binds a save file to a GFraMe_save context.
 * The file is create on a proper folder ('|' means concatenation):
 *   - /data/data/organization|title/filename, on Android
 *   - %APPDATA%/organization|title/filename, on Windows
 *   - ~/.local/shared/organization|title/filename, on Linux
 */
GFraMe_ret GFraMe_save_bind(GFraMe_save *sv, char *filename);

/**
 * Closes a previously bound context.
 * This assures that the content of the file was written to the disk.
 */
void GFraMe_save_close(GFraMe_save *sv);

/**
 * Erase all content from a file. It's kept open for writing/reading.
 */
void GFraMe_save_flush(GFraMe_save *sv, int force);

/**
 * Erase all content from a file. It's kept open for writing/reading.
 */
void GFraMe_save_erase(GFraMe_save *sv);
/**
 * Writes a 32 bits integer to the file.
 * If this was compiled for a 64 bits architecture, the upper 32 bits will be
 *discarded when saving.
 */
GFraMe_ret GFraMe_save_write_int(GFraMe_save *sv, char *id, int val);
/**
 * Reads a 32 bits integer from the files.
 */
GFraMe_ret GFraMe_save_read_int(GFraMe_save *sv, char *id, int *ret);
/**
 * Writes 'size' bytes to the file.
 */
GFraMe_ret GFraMe_save_write_bytes(GFraMe_save *sv, char *id, char *data,
	int size);
/**
 * If data is NULL, the data size (on the file) is returned on the 'size'
 *parameter. Otherwise, it expects 'data' to be long enough and returns the
 *number of bytes read on 'size'.
 */
GFraMe_ret GFraMe_save_read_bytes(GFraMe_save *sv, char *id, char *data,
	int *size);

#endif

