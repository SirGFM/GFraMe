/**
 * @src/gframe.c
 */
#include <GFraMe/GFraMe.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_save.h>
#include <GFraMe/GFraMe_util.h>
#include <SDL2/SDL_filesystem.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_system.h>
#include <stdio.h>

#ifdef NEW_SAVE

typedef struct {
	/**
	 * One of GFraMe_save_type
	 */
	char type;
	/**
	 * Length of the id
	 */
	char id_len;
	/**
	 * Length of the stored data (in bytes)
	 */
	char len;
} GFraMe_save_header;

#define GFraMe_save_header_len	sizeof(GFraMe_save_header)
#define GFraMe_save_max_id_len	256

static char GFraMe_save_tmp_id[GFraMe_save_max_id_len];

typedef struct {
	GFraMe_save_header header;
	char *id;
	char *data;
} GFraMe_save_slot;

/**
 * Get a valid slot for reading or writing and set the file descriptor
 *position at the begining of the slot's data.
 */
static GFraMe_save_ret GFraMe_save_get_slot(GFraMe_save *sv, char *id,
	GFraMe_save_slot &slot);
static void GFraMe_save_get_file_size(GFraMe_save *sv);

GFraMe_ret GFraMe_save_bind(GFraMe_save *sv, char *filename) {
	char *tmp;
	GFraMe_ret rv = GFraMe_ret_ok;
	int len = GFraMe_save_max_len;
	
	// Truncate the filename to the maximum length
	tmp = sv->filename;
#if defined(__ANDROID__) && __ANDROID__
	tmp = GFraMe_util_strcat(tmp, SDL_AndroidGetInternalStoragePath(), &len);
#else
	char *sdl_path = SDL_GetPrefPath(GFraMe_org, GFraMe_title);
	if (sdl_path) {
		tmp = GFraMe_util_strcat(tmp, sdl_path, &len);
		SDL_free(sdl_path);
	}
#endif
	tmp = GFraMe_util_strcat(tmp, filename, &len);
	if (len <= 0)
		*(tmp-1) = '\0';
	
	// Try to open the file (create it, if it doesn't exists)
	sv->file = SDL_RWFromFile(sv->filename, "rb+");
	if (!sv->file)
		sv->file = SDL_RWFromFile(sv->filename, "wb+");
	GFraMe_assertRV(sv->file, "Failed to open file",
		rv = GFraMe_ret_failed, _ret);
	
	GFraMe_save_get_file_size(sv);
	sv->state = GFraMe_save_was_flushed;
_ret:
	return rv;
}

void GFraMe_save_close(GFraMe_save *sv) {
	if (sv->file) {
		GFraMe_save_flush(sv, 1);
		SDL_RWclose(sv->file);
		sv->file = NULL;
		sv->size = 0;
		sv->state = GFraMe_save_was_flushed;
	}
}

void GFraMe_save_flush(GFraMe_save *sv, int force) {
}

void GFraMe_save_clear(GFraMe_save *sv) {
	if (sv->file) {
		SDL_RWclose(sv->file);
		sv->file = SDL_RWFromFile(sv->filename, "wb");
		SDL_RWclose(sv->file);
		sv->file = SDL_RWFromFile(sv->filename, "rb+");
	}
}

GFraMe_ret GFraMe_save_write(GFraMe_save *sv, GFraMe_save_type type, char *id,
	void *data, int size) {
	
}
GFraMe_ret GFraMe_save_read(GFraMe_save *sv, char *id,
	void *data, int size, int count) {
	GFraMe_save_ret srv;
	
	srv = GFraMe_save_goto_ID_position(sv, id);
}

static GFraMe_save_ret GFraMe_save_get_slot(GFraMe_save *sv, char *id,
	GFraMe_save_slot &slot) {
	GFraMe_save_ret srv;
	GFraMe_save_header header;
	int pos;
	
	GFraMe_assert(sv->size > 0, "File is empty!",
		srv = GFraMe_save_ret_empty, _ret);
	SDL_RWseek(sv->file, SEEK_SET, 0);
	pos = 0;
	
	srv = GFraMe_save_ret_failed;
	while (1) {
		int read_len;
		
		GFraMe_assert(pos < sv->size, "Reached eof!",
			srv = GFraMe_save_ret_eof, _ret);
		
		read_len = SDL_RWread(sv->file, &header, GFraMe_save_header_len, 1);
		GFraMe_SDLassert(read_len == GFraMe_save_header_len,
			"Failed to read id", srv = GFraMe_save_ret_failed, _ret);
		pos += read_len;
		
		memset(GFraMe_save_tmp_id, 0x0, GFraMe_save_max_id_len);
		read_len = SDL_RWread(sv->file, GFraMe_save_tmp_id, header.id_len, 1);
		GFraMe_SDLassertRet(read_len == header.id_len,
			"Failed to read id", srv = GFraMe_save_ret_failed, _ret);
		pos += read_len;
		
		if (GFraMe_util_strcmp(id, GFraMe_save_tmp_id)) {
			srv = GFraMe_save_ret_ok;
			slot->header.type = header.type;
			slot->header.id_len = header.id_len;
			slot->header.len = header.len;
			slot->id = GFraMe_save_tmp_id;
			slot->data = NULL;
			break;
		}
		
		read_len = SDL_RWseek(sv->file, SEEK_CUR, header.len);
		GFraMe_assert(read_len != -1, "Failed to seek!",
			srv = GFraMe_save_ret_failed, _ret);
		GFraMe_assert(read_len == pos + header.len, "Failed to seek!",
			srv = GFraMe_save_ret_failed, _ret);
		pos = read_len;
	}
_ret:
	return srv;
}

static GFraMe_ret GFraMe_save_read_id(GFraMe_save *sv, char *id) {
}

static void GFraMe_save_get_file_size(GFraMe_save *sv) {
	int rv;
	
	rv = SDL_RWseek(sv->file, SEEK_END, 0);
	GFraMe_assert
	sv->size = (int)SDL_RWtell(sv->file);
}

#else

// (BYTE)ID_LEN (ID_LEN)ID (BYTE)DATA_LEN (DATA_LEN)DATA

static GFraMe_save_ret GFraMe_save_goto_ID_position(GFraMe_save *sv, char *id);
static GFraMe_ret GFraMe_save_read_id(GFraMe_save *sv, char *id);
static void GFraMe_save_get_file_size(GFraMe_save *sv);

GFraMe_ret GFraMe_save_bind(GFraMe_save *sv, char *filename) {
	GFraMe_ret rv = GFraMe_ret_ok;
	int len = GFraMe_save_max_len;
	char *tmp;
	// Truncate the filename to the maximum length
	tmp = sv->filename;
#if defined(__ANDROID__) && __ANDROID__
	tmp = GFraMe_util_strcat(tmp, SDL_AndroidGetInternalStoragePath(), &len);
#else
	char *sdl_path = SDL_GetPrefPath(GFraMe_org, GFraMe_title);
	if (sdl_path) {
		tmp = GFraMe_util_strcat(tmp, sdl_path, &len);
		SDL_free(sdl_path);
	}
#endif
	tmp = GFraMe_util_strcat(tmp, filename, &len);
	if (len <= 0)
		*(tmp-1) = '\0';
	// Try to open the file (create it, if it doesn't exists)
	sv->file = SDL_RWFromFile(sv->filename, "rb+");
	if (!sv->file)
		sv->file = SDL_RWFromFile(sv->filename, "wb+");
	GFraMe_assertRV(sv->file, "Failed to open file",
					rv = GFraMe_ret_failed, _ret);
	GFraMe_save_get_file_size(sv);
	sv->state = GFraMe_save_was_flushed;
_ret:
	return rv;
}

void GFraMe_save_close(GFraMe_save *sv) {
	if (sv->file) {
		GFraMe_save_flush(sv, 1);
		SDL_RWclose(sv->file);
		sv->file = NULL;
		sv->size = 0;
		sv->state = GFraMe_save_was_flushed;
	}
}

void GFraMe_save_flush(GFraMe_save *sv, int force) {
	if ( force
	  || ( (sv->state & GFraMe_save_did_write)
	    && (sv->state & GFraMe_save_is_reading)
		 )
	  || ( (sv->state & GFraMe_save_did_read)
	    && (sv->state & GFraMe_save_is_writing)
		 )
	   )
		SDL_RWseek(sv->file, 0, SEEK_CUR);
	sv->state = GFraMe_save_was_flushed;
}

void GFraMe_save_clear(GFraMe_save *sv) {
	if (sv->file) {
		SDL_RWclose(sv->file);
		sv->file = SDL_RWFromFile(sv->filename, "wb");
		SDL_RWclose(sv->file);
		sv->file = SDL_RWFromFile(sv->filename, "rb+");
	}
}

GFraMe_ret GFraMe_save_write(GFraMe_save *sv, char *id,
		void *data, int size, int count) {
	GFraMe_ret rv = GFraMe_ret_ok;
	GFraMe_save_ret srv = GFraMe_save_ret_ok;
	srv = GFraMe_save_goto_ID_position(sv, id);
	if (srv == GFraMe_save_ret_id_not_found
	 || srv == GFraMe_save_ret_empty) {
		char tmp;
		tmp = GFraMe_util_strlen(id);
		SDL_RWwrite(sv->file, &tmp, sizeof(char), 1);
		SDL_RWwrite(sv->file, id, sizeof(char), tmp);
		srv = GFraMe_save_ret_ok;
	}
	if (srv == GFraMe_save_ret_ok) {
		char tmp;
		tmp = size*count;
		SDL_RWwrite(sv->file, &tmp, sizeof(char), 1);
		SDL_RWwrite(sv->file, data, size, count);
		rv = GFraMe_ret_ok;
		GFraMe_save_get_file_size(sv);
	}
	else
		rv = GFraMe_ret_failed;
	return rv;
}

GFraMe_ret GFraMe_save_read(GFraMe_save *sv, char *id,
		void *data, int size, int count) {
	GFraMe_ret rv = GFraMe_ret_ok;
	GFraMe_save_ret srv = GFraMe_save_ret_ok;
	srv = GFraMe_save_goto_ID_position(sv, id);
	if (srv == GFraMe_save_ret_ok) {
		char tmp;
		SDL_RWread(sv->file, &tmp, sizeof(char), 1);
		if (tmp == size*count) {
			SDL_RWread(sv->file, data, size, count);
			rv = GFraMe_ret_ok;
		}
		else
			rv = GFraMe_ret_failed;
	}
	else
		rv = GFraMe_ret_failed;
	return rv;
}

static GFraMe_save_ret GFraMe_save_goto_ID_position(GFraMe_save *sv, char *id) {
	int pos;
	GFraMe_save_ret rv = GFraMe_save_ret_ok;
	char buf[GFraMe_save_max_len];
	// Check if the file isn't empty
	GFraMe_assertRV(sv->size > 0, "File's empty",
					rv = GFraMe_save_ret_empty, _ret);
	// Go back to the file's begin
	pos = SDL_RWseek(sv->file, 0, SEEK_SET);
	GFraMe_assertRV(pos >= 0, "ERROR", rv = GFraMe_save_ret_failed, _ret);
	while (1) {
		int res;
		char len;
		GFraMe_assertRV(pos != sv->size, "ID not found",
						rv = GFraMe_save_ret_id_not_found, _ret);
		// Try to read the current id
		rv = GFraMe_save_read_id(sv, buf);
		GFraMe_assertRet(rv == GFraMe_save_ret_ok, "Failed to seek id", _ret);
		// Exit loop if it was found
		if (GFraMe_util_strcmp(id, buf))
			break;
		// Skip the data
		res = SDL_RWread(sv->file, &len, sizeof(char), 1);
		GFraMe_assertRV(res > 0, "ERROR", rv = GFraMe_save_ret_failed, _ret);
		pos = SDL_RWseek(sv->file, len, SEEK_CUR);
	}
	rv = GFraMe_save_ret_ok;
_ret:
	return rv;
}

static GFraMe_ret GFraMe_save_read_id(GFraMe_save *sv, char *id) {
	int rv;
	char id_len;
	// Try to read the id length
	rv = SDL_RWread(sv->file, &id_len, sizeof(char), 1);
	GFraMe_SDLassertRet(rv == 1, "Couldn't read id len", _ret);
	// Try to read the id
	rv = SDL_RWread(sv->file, id, sizeof(char), id_len);
	GFraMe_SDLassertRet(rv == id_len, "Failed to read id", _ret);
	// id_len is at most 255, so no check is needed
	id[(int)id_len] = '\0';
_ret:
	return (rv != 0)?GFraMe_ret_ok:GFraMe_ret_failed;
}

static void GFraMe_save_get_file_size(GFraMe_save *sv) {
	int pos;
	pos = SDL_RWseek(sv->file, 0, SEEK_END);
	if (pos >= 0)
		sv->size = pos;
	pos = SDL_RWseek(sv->file, 0, SEEK_SET);
}

#endif
