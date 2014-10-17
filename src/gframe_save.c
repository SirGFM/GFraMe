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

enum {
	INT = 0,
	BYTEARRAY,
};

typedef struct {
	GFraMe_save *sv;
	char *id;
	char idlen;
	char type;
	char objlen;
	int pos;
} GFraMe_save_ctx;

static GFraMe_ret GFraMe_save_get_file_size(GFraMe_save *sv);
GFraMe_ret GFraMe_save_goto_id(GFraMe_save_ctx *ctx, char *tgt_id);
GFraMe_ret GFraMe_save_read_id(GFraMe_save_ctx *ctx);
GFraMe_ret GFraMe_save_write_id(GFraMe_save_ctx *ctx);

/**
 * Binds a save file to a GFraMe_save context.
 * The file is create on a proper folder ('|' means concatenation):
 *   - /data/data/organization|title/filename, on Android
 *   - %APPDATA%/organization|title/filename, on Windows
 *   - ~/.local/shared/organization|title/filename, on Linux
 */
GFraMe_ret GFraMe_save_bind(GFraMe_save *sv, char *filename) {
	char *tmp;
	GFraMe_ret rv = GFraMe_ret_ok;
	int len = GFraMe_save_max_len;
	
	// Truncate the filename to the maximum length
	tmp = sv->filename;
	tmp = GFraMe_util_get_local_path(tmp, &len);
	tmp = GFraMe_util_strcat(tmp, filename, &len);
	if (len <= 0)
		*(tmp-1) = '\0';
	
	// Try to open the file (create it, if it doesn't exists)
	sv->file = SDL_RWFromFile(sv->filename, "rb+");
	if (!sv->file) {
		sv->file = SDL_RWFromFile(sv->filename, "wb+");
		GFraMe_save_write_int(sv, "GFraMe_V", 0x00010000);
		sv->version = 100;
	}
	else {
		GFraMe_save_get_file_size(sv);
		rv = GFraMe_save_read_int(sv, "GFraMe_V", &sv->version);
		if (rv != GFraMe_ret_ok) {
			// Ok, sorry... for now, I'll overwrite the file
			GFraMe_save_erase(sv);
			GFraMe_save_write_int(sv, "GFraMe_V", 0x00010000);
			sv->version = 100;
		}
	}
	GFraMe_assertRV(sv->file, "Failed to open file",
		rv = GFraMe_ret_failed, _ret);
	
	// Cache the file size
	GFraMe_save_get_file_size(sv);
	sv->state = GFraMe_save_was_flushed;
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
}

/**
 * Closes a previously bound context.
 * This assures that the content of the file was written to the disk.
 */
void GFraMe_save_close(GFraMe_save *sv) {
	if (sv->file) {
		GFraMe_save_flush(sv, 1);
		SDL_RWclose(sv->file);
		sv->file = NULL;
		sv->size = 0;
		sv->state = GFraMe_save_was_flushed;
	}
}

/**
 * Erase all content from a file. It's kept open for writing/reading.
 */
void GFraMe_save_erase(GFraMe_save *sv) {
	if (sv->file) {
		SDL_RWclose(sv->file);
		sv->file = SDL_RWFromFile(sv->filename, "wb");
		SDL_RWclose(sv->file);
		sv->file = SDL_RWFromFile(sv->filename, "rb+");
	}
}

GFraMe_ret GFraMe_save_write_id(GFraMe_save_ctx *ctx) {
	GFraMe_ret rv;
	int irv;
	
	irv = SDL_RWwrite(ctx->sv->file, &(ctx->idlen), sizeof(char), 1);
	GFraMe_SDLassertRV(irv == 1, "Couldn't write id len",
		rv = GFraMe_ret_failed, _ret);
	ctx->pos += 1;
	
	// Try to write the id
	irv = SDL_RWwrite(ctx->sv->file, ctx->id, sizeof(char), ctx->idlen);
	GFraMe_SDLassertRV(irv == ctx->idlen, "Failed to write id",
		rv = GFraMe_ret_failed, _ret);
	ctx->pos += ctx->idlen;
	
	// Try to write the object's type
	irv = SDL_RWwrite(ctx->sv->file, &(ctx->type), sizeof(char), 1);
	GFraMe_SDLassertRV(irv == 1, "Couldn't write obj type",
		rv = GFraMe_ret_failed, _ret);
	ctx->pos += 1;
	
	// Also write the length if it's a byte array
	if (ctx->type == BYTEARRAY) {
		irv = SDL_RWwrite(ctx->sv->file, &(ctx->objlen), sizeof(char), 1);
		GFraMe_SDLassertRV(irv == 1, "Couldn't write obj type",
			rv = GFraMe_ret_failed, _ret);
		ctx->pos += 1;
	}
	else if (ctx->type == INT) {
		ctx->objlen = 4;
	}
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
}

GFraMe_ret GFraMe_save_read_id(GFraMe_save_ctx *ctx) {
	GFraMe_ret rv;
	int irv;
	
	// Try to read the id length
	irv = SDL_RWread(ctx->sv->file, &(ctx->idlen), sizeof(char), 1);
	GFraMe_SDLassertRV(irv == 1, "Couldn't read id len",
		rv = GFraMe_ret_failed, _ret);
	ctx->pos += 1;
	
	// Try to read the id
	irv = SDL_RWread(ctx->sv->file, ctx->id, sizeof(char), ctx->idlen);
	GFraMe_SDLassertRV(irv == ctx->idlen, "Failed to read id",
		rv = GFraMe_ret_failed, _ret);
	ctx->pos += ctx->idlen;
	
	// id_len is at most 255, so no check is needed
	ctx->id[(int)ctx->idlen] = '\0';
	
	// Try to read the object's type
	irv = SDL_RWread(ctx->sv->file, &(ctx->type), sizeof(char), 1);
	GFraMe_SDLassertRV(irv == 1, "Couldn't read obj type",
		rv = GFraMe_ret_failed, _ret);
	ctx->pos += 1;
	
	// Also read the length if it's a byte array
	if (ctx->type == BYTEARRAY) {
		irv = SDL_RWread(ctx->sv->file, &(ctx->objlen), sizeof(char), 1);
		GFraMe_SDLassertRV(irv == 1, "Couldn't read obj type",
			rv = GFraMe_ret_failed, _ret);
		ctx->pos += 1;
	}
	else if (ctx->type == INT) {
		ctx->objlen = 4;
	}
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
}

GFraMe_ret GFraMe_save_goto_id(GFraMe_save_ctx *ctx, char *tgt_id) {
	GFraMe_ret rv;
	
	// Go back to the file's begin
	ctx->pos = SDL_RWseek(ctx->sv->file, 0, SEEK_SET);
	GFraMe_assertRV(ctx->pos >= 0, "ERROR", rv = GFraMe_save_ret_failed, _ret);
	
	// Loop searching for the tgt_it
	while (ctx->pos < ctx->sv->size) {
		rv = GFraMe_save_read_id(ctx);
		GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to read from file",
			rv = GFraMe_ret_failed, _ret);
		
		// Exit loop if it was found
		if (GFraMe_util_strcmp(ctx->id, tgt_id) == GFraMe_ret_ok) {
			rv = GFraMe_ret_ok;
			break;
		}
		
		ctx->pos = SDL_RWseek(ctx->sv->file, ctx->objlen, SEEK_CUR);
	}
	
	if (ctx->pos >= ctx->sv->size)
		rv = GFraMe_ret_failed;
	
_ret:
	return rv;
}

/**
 * Writes a 32 bits integer to the file.
 * If this was compiled for a 64 bits architecture, the upper 32 bits will be
 *discarded when saving.
 */
GFraMe_ret GFraMe_save_write_int(GFraMe_save *sv, char *id, int val) {
	GFraMe_save_ctx ctx;
	GFraMe_ret rv;
	int irv, do_recache_size = 0;
	char id_buf[GFraMe_save_id_max_len];
	
	ctx.sv = sv;
	ctx.id = (char *)id_buf;
	
	rv = GFraMe_save_goto_id(&ctx, id);
	
	if (rv == GFraMe_ret_failed) {
		int len = GFraMe_save_id_max_len;
		
		GFraMe_util_strcat(ctx.id, id, &len);
		ctx.idlen = GFraMe_save_id_max_len - len;
		ctx.type = INT;
		rv = GFraMe_save_write_id(&ctx);
		GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to write id", 
			rv = GFraMe_ret_failed, _ret);
		do_recache_size = 1;
	}
	else
		GFraMe_assertRV(ctx.type == INT, "Can't convert type on save",
			rv = GFraMe_ret_failed, _ret);
	
	irv = SDL_WriteLE32(sv->file, (Uint32)val);
	GFraMe_assertRV(irv == 1, "Failed to write value", rv = GFraMe_ret_failed,
		_ret);
	
	if (do_recache_size)
		GFraMe_save_get_file_size(sv);
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
}

/**
 * Reads a 32 bits integer from the files.
 */
GFraMe_ret GFraMe_save_read_int(GFraMe_save *sv, char *id, int *ret) {
	GFraMe_save_ctx ctx;
	GFraMe_ret rv;
	Uint32 val;
	char id_buf[GFraMe_save_id_max_len];
	
	ctx.sv = sv;
	ctx.id = (char *)id_buf;
	
	rv = GFraMe_save_goto_id(&ctx, id);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "ID not found in file",
		rv = GFraMe_ret_failed, _ret);
	GFraMe_assertRV(ctx.type == INT, "Can't convert type on load",
		rv = GFraMe_ret_failed, _ret);
	
	val = SDL_ReadLE32(sv->file);
	*ret = (int)val;
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
}

/**
 * Writes 'size' bytes to the file.
 */
GFraMe_ret GFraMe_save_write_bytes(GFraMe_save *sv, char *id, char *data,
	int size) {
	GFraMe_save_ctx ctx;
	GFraMe_ret rv;
	int irv, do_recache_size = 0;
	char id_buf[GFraMe_save_id_max_len];
	
	ctx.sv = sv;
	ctx.id = (char *)id_buf;
	
	rv = GFraMe_save_goto_id(&ctx, id);
	
	if (rv == GFraMe_ret_failed) {
		int len = GFraMe_save_id_max_len;
		
		GFraMe_util_strcat(ctx.id, id, &len);
		ctx.idlen = GFraMe_save_id_max_len - len;
		ctx.type = BYTEARRAY;
		ctx.objlen = size;
		rv = GFraMe_save_write_id(&ctx);
		GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to write id",
			rv = GFraMe_ret_failed, _ret);
		do_recache_size = 1;
	}
	else {
		GFraMe_assertRV(ctx.type == BYTEARRAY, "Can't convert type on save",
			rv = GFraMe_ret_failed, _ret);
		GFraMe_assertRV(ctx.objlen == size, "Can't resize array on save",
			rv = GFraMe_ret_failed, _ret);
	}
	
	irv = SDL_RWwrite(sv->file, data, sizeof(char), size);
	GFraMe_assertRV(irv == 1, "Failed to write value", rv = GFraMe_ret_failed,
		_ret);
	
	if (do_recache_size)
		GFraMe_save_get_file_size(sv);
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
}
/**
 * If data is NULL, the data size (on the file) is returned on the 'size'
 *parameter. Otherwise, it expects 'data' to be long enough and returns the
 *number of bytes read on 'size'.
 */
GFraMe_ret GFraMe_save_read_bytes(GFraMe_save *sv, char *id, char *data,
	int *size) {
	
	GFraMe_save_ctx ctx;
	GFraMe_ret rv;
	int irv;
	char id_buf[GFraMe_save_id_max_len];
	
	ctx.sv = sv;
	ctx.id = (char *)id_buf;
	
	rv = GFraMe_save_goto_id(&ctx, id);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "ID not found", rv = rv, _ret);
	GFraMe_assertRV(ctx.type == BYTEARRAY, "Can't convert type on load",
		rv = GFraMe_ret_failed, _ret);
	
	if (data == NULL) {
		rv = GFraMe_ret_ok;
		goto _setsize;
	}
	
	GFraMe_assertRV(*size >= ctx.objlen, "Buffer too small",
		rv = GFraMe_buffer_too_small, _setsize);
	
	irv = SDL_RWwrite(sv->file, data, sizeof(char), ctx.objlen);
	GFraMe_assertRV(irv == 1, "Failed to read value", rv = GFraMe_ret_failed,
		_setsize);
	
	rv = GFraMe_ret_ok;
_setsize:
	*size = ctx.objlen;
_ret:
	return rv;
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

static GFraMe_ret GFraMe_save_get_file_size(GFraMe_save *sv) {
	GFraMe_ret rv;
	
	//irv = SDL_RWseek(sv->file, SEEK_END, 0);
	//GFraMe_assertRV(irv >= 0, "Failed to seek", rv = GFraMe_ret_failed, _ret);
	//sv->size = (int)SDL_RWtell(sv->file);
	
	sv->size = (int)SDL_RWsize(sv->file);
	GFraMe_assertRV(sv->size >= 0, "Failed to seek", rv = GFraMe_ret_failed,
		_ret);
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
}
