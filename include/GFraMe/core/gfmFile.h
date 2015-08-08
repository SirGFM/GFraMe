/**
 * @file include/GFraMe/core/gfmFile.h
 * 
 * Generic file interface; It should be used to abstract opening assests (which
 * might be compressed, on mobile) and opening a file (e.g., the Log) on the
 * default path (e.g., ~/.local/share/<company>/<game> or
 * %APPDATA%\<company>\<game>)
 */
#ifndef __GFMFILE_STRUCT__
#define __GFMFILE_STRUCT__

/** 'Export' the file type */
typedef struct stGFMFile gfmFile;

#endif /* __GFMFILE_STRUCT__ */

#ifndef __GFMFILE_H__
#define __GFMFILE_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>

/**
 * Alloc a new gfmFile struct
 * 
 * @param  ppFile The alloc'ed file
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmFile_getNew(gfmFile **ppFile, gfmCtx *pCtx);

/**
 * Close and free a gfmFile
 * 
 * @param  ppCtx The file
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFile_free(gfmFile **ppCtx);

/**
 * Open a file on the game's local path. The changes with the OS:
 *   - android: ???
 *   - linux: ~/.local/share/<company>/<game>/
 *   - windows: %APPDATA%\<company>\<game>\
 * 
 * @param  pCtx    The file struct
 * @param  pPath   The filename
 * @param  pathLen Length of the filename
 */
gfmRV gfmFile_openLocal(gfmFile *pCtx, char *pPath, int pathLen);

/**
 * Open a file on the game's local path. The changes with the OS:
 *   - android: ???
 *   - linux: ~/.local/share/<company>/<game>/
 * The path must be a statically allocat'ed buffer! (i.e., either
 * 'gfmFile_openFileStatic(pCtx, "some-file-name);' or
 * 'char filename[] = "...";')
 * 
 * @param  pCtx  The file struct
 * @param  pPath The filename
 */
#define gfmFile_openFileStatic(pCtx, pPath) \
    gfmFile_openFile(pCtx, pPath, sizeof(pPath) - 1)

/**
 * Open an asset file; The file is expected to be found on an 'assets'
 * directory, which must be found on the same directory as the game's binary
 * 
 * @param  pCtx    The file struct
 * @param  pPath   The filename
 * @param  pathLen Length of the filename
 */
gfmRV gfmFile_openAsset(gfmFile *pCtx, char *pPath, int pathLen);

/**
 * Open an asset file; The file is expected to be found on an 'assets'
 * directory, which must be found on the same directory as the game's binary;
 * The path must be a statically allocat'ed buffer! (i.e., either
 * 'gfmFile_openAssetStatic(pCtx, "some-file-name);' or
 * 'char filename[] = "...";')
 * 
 * @param  pCtx    The file struct
 * @param  pPath The filename
 */
#define gfmFile_openAssetStatic(pCtx, pPath) \
    gfmFile_openFile(pCtx, pPath, sizeof(pPath) - 1)

/**
 * Close a file
 * 
 * @param  pCtx The file struct
 */
gfmRV gfmFile_close(gfmFile *pCtx);

/**
 * Retrieve the file's size
 * 
 * @param  pSize The file size (in bytes)
 * @param  pCtx  The file struct
 */
gfmRV gfmFile_getSize(int *pSize, gfmFile *pCtx);

/**
 * Read a character from the file
 * 
 * @param  pVal The character
 * @param  pCtx The file
 */
gfmRV gfmFile_readChar(char *pVal, gfmFile *pCtx);

/**
 * Write a character to the file
 * 
 * @param  pVal The character
 * @param  pCtx The file
 */
gfmRV gfmFile_writeChar(gfmFile *pCtx, char val);

/**
 * Roll back a character (similar to stdio's ungetc); The last read character
 * (if needed) must be retrieved from the file context
 * 
 * @param  pCtx The file
 */
gfmRV gfmFile_unreadChar(gfmFile *pCtx);

/**
 * Read 2 bytes (i.e., half a 32 bits word) into an integer; The value is
 * expected to be in little-endian format
 * 
 * @param  pVal The half word
 * @param  pCtx The file
 */
gfmRV gfmFile_readHalfWord(int *pVal, gfmFile *pCtx);

/**
 * Read 2 bytes (i.e., half a 32 bits word) into an integer; The value will
 * be written in little-endian format
 * 
 * @param  pCtx The file
 * @param  val  The half word
 */
gfmRV gfmFile_writeHalfWord(gfmFile *pCtx, int val);

/**
 * Read 4 bytes (i.e., a 32 bits integer) into an integer; The value is
 * expected to be in little-endian format
 * 
 * @param  pVal The word
 * @param  pCtx The file
 */
gfmRV gfmFile_readWord(int *pVal, gfmFile *pCtx);

/**
 * Read 4 bytes (i.e., a 32 bits integer) into an integer; The value will be
 * written in little-endian format
 * 
 * @param  pCtx The file
 * @param  val  The word
 */
gfmRV gfmFile_writeWord(gfmFile *pCtx, int val);

/**
 * Read a stream of bytes from the file; If the EOF is reached before reading
 * the desired number of bytes, this function must return successfuly
 * 
 * @param  pVal     A array of bytes of numBytes length
 * @param  pLen     How many bytes were actually read from the file
 * @param  pCtx     The file
 * @param  numBytes How many bytes should be read
 */
gfmRV gfmFile_readBytes(char *pVal, int *pLen, gfmFile *pCtx, int numBytes);

/**
 * Read a stream of bytes from the file; If the EOF is reached before reading
 * the desired number of bytes, this function must return successfuly
 * 
 * @param  pCtx The file
 * @param  pVal A array of bytes of numBytes length
 * @param  len  How many bytes were actually read from the file
 */
gfmRV gfmFile_writeBytes(gfmFile *pCtx, char *pVal, int len);

#endif /* __GFMFILE_H__ */

