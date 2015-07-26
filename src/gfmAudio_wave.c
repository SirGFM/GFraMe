/**
 * @file src/include/GFraMe_int/gfmAudio_wave.h
 * 
 * Module to parse a WAVE audio file
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmAudio_wave.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Format of the waveform */
enum enWAVEFormatTag {
    /** PCM */
    WAVE_FORMAT_PCM        = 0x0001,
    /** IEEE float */
    WAVE_FORMAT_IEEE_FLOAT = 0x0003,
    /** 8-bit ITU-T G.711 A-law */
    WAVE_FORMAT_ALAW       = 0x0006,
    /** 8-bit ITU-T G.711 µ-law */
    WAVE_FORMAT_MULAW      = 0x0007,
    /** Determined by SubFormat */
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE
};
typedef enum enWAVEFormatTag waveFormatTag;

/** The format of the stored wave file */
struct stWAVEFormat {
    waveFormatTag format;
    /** Number of interleaved channels */
    int numChannels;
    /** Sample's frequency (in Hertz) */
    int sampleRate;
    /** Frequency in bytes per second */
    int byteRate;
    /** How many bytes are needed per sample (in case the bits-per-samples is
     *  misaligned) */
    int bytesPerSample;
    /** How many bits are needed per sample */
    int bitsPerSample;
};
typedef struct stWAVEFormat waveFormat;

/** RIFF files are comprised of chunks/sub chunks with this 'format' */
struct stRIFFChunk {
    /** Chunk ID (always 4 bytes) */
    char pId[4];
    /** Chunk size (4 bytes integer) */
    int size;
    /** Chunk data (of size bytes) */
    char *pData;
};
typedef struct stRIFFChunk riffChunk;

/******************************************************************************/
/*                                                                            */
/* Static functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Converts a little-endian buffer to an '16-bits integer'
 */
#define gfmAudio_getHalfWordLE(pBuf) \
    (int)(    ( ((pBuf)[0]      ) & 0x000000ff) \
            | ( ((pBuf)[1] << 8 ) & 0x0000ff00) )

/**
 * Converts a little-endian buffer to an '32-bits integer'
 */
#define gfmAudio_getWordLE(pBuf) \
    (int)(    ( ((pBuf)[0]      ) & 0x000000ff) \
            | ( ((pBuf)[1] << 8 ) & 0x0000ff00) \
            | ( ((pBuf)[2] << 16) & 0x00ff0000) \
            | ( ((pBuf)[3] << 24) & 0xff000000) )

/**
 * Read a chunk's id and size from a file
 * 
 * @param  pCtx The chunk read
 * @param  pFp  The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
static gfmRV gfmAudio_readRIFFChunkHeader(riffChunk *pCtx, FILE *pFp) {
    char pBuf[4];
    gfmRV rv;
    int irv, count;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    
    // Read the chunk's ID (4 bytes)
    count = 4;
    irv = fread(pCtx->pId, count, sizeof(char), pFp);
    ASSERT(irv == count, GFMRV_READ_ERROR);
    
    // Read the chunk's size (4 bytes)
    count = 4;
    irv = fread(pBuf, count, sizeof(char), pFp);
    ASSERT(irv == count, GFMRV_READ_ERROR);
    // Convert the chunk's size (it's in little-endian)
    pCtx->size = gfmAudio_getWordLE(pBuf);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Read the wave's first chunk, check that it's valid and get its size
 * 
 * @param  pSize The 'master' chunk size (minux the 4 'file type' bytes)
 * @param  pFp   The file pointer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR,
 *               GFMRV_FUNCTION_FAILED
 */
static gfmRV gfmAudio_readMasterChunk(int *pSize, FILE *pFp) {
    char pBuf[4];
    gfmRV rv;
    int irv, count;
    riffChunk chunk;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSize, GFMRV_ARGUMENTS_BAD);
    
    // Read the first chunk (it's ID must be RIFF)
    rv = gfmAudio_readRIFFChunkHeader(&chunk, pFp);
    ASSERT_NR(rv == GFMRV_OK);
    // Check the ID
    ASSERT(chunk.pId[0] == 'R' && chunk.pId[1] == 'I' && chunk.pId[2] == 'F' &&
            chunk.pId[3] == 'F', GFMRV_FUNCTION_FAILED);
    // Check that there are at least 4 more byte (that must contain "WAVE")
    ASSERT(chunk.size >= 4, GFMRV_FUNCTION_FAILED);
    // Read those next 4 bytes
    count = 4;
    irv = fread(pBuf, count, sizeof(char), pFp);
    ASSERT(irv == count, GFMRV_READ_ERROR);
    // Check that those bytes actually read "WAVE"
    ASSERT(pBuf[0] == 'W' && pBuf[1] == 'A' && pBuf[2] == 'V' && pBuf[3] == 'E',
            GFMRV_FUNCTION_FAILED);
    
    // Return the retrieved size (after removing 4 bytes)
    *pSize = chunk.size - 4;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Read the format of the wave data
 * 
 * @param  pFormat The retrieved format
 * @param  pFp     The file pointer
 * @param  size    How many bytes there are in the format
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                 GFMRV_READ_ERROR
 */
static gfmRV gfmAudio_readWaveFormat(waveFormat *pFormat, FILE *pFp, int size) {
    char *pBuf;
    gfmRV rv;
    int irv;
    
    // Set default values
    pBuf = 0;
    
    // Sanitize arguments
    ASSERT(pFormat, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    // Those are the only possible sizes for the data format (it's on the docs)
    ASSERT(size == 16 || size == 18 || size == 40, GFMRV_ARGUMENTS_BAD);
    
    // Alloc enough bytes to read the message
    pBuf = (char*)malloc(sizeof(char)*size);
    ASSERT(pBuf, GFMRV_ALLOC_FAILED);
    
    // Read the data format from the file
    irv = fread(pBuf, size, sizeof(char), pFp);
    ASSERT(irv == size, GFMRV_READ_ERROR);
    
    // Read from the buffer into a format struct (so it can be returned)
    pFormat->format         = gfmAudio_getHalfWordLE(pBuf + 0);
    pFormat->numChannels    = gfmAudio_getHalfWordLE(pBuf + 2);
    pFormat->sampleRate     = gfmAudio_getWordLE(pBuf + 4);
    pFormat->byteRate       = gfmAudio_getWordLE(pBuf + 8);
    pFormat->bytesPerSample = gfmAudio_getHalfWordLE(pBuf + 12);
    pFormat->bitsPerSample  = gfmAudio_getHalfWordLE(pBuf + 14);
    
    rv = GFMRV_OK;
__ret:
    if (pBuf) {
        free(pBuf);
    }
    
    return rv;
}

/******************************************************************************/
/*                                                                            */
/* Public functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Check if an audio file is encoded as WAVE
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isWave(FILE *pFp) {
    gfmRV rv;
    int size;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    // Rewind the file
    rewind(pFp);
    
    // Try to read the master chunk (will succeed if it's an wave file)
    rv = gfmAudio_readMasterChunk(&size, pFp);
    ASSERT(rv == GFMRV_OK, GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

gfmRV gfmAudio_loadWave(char **ppBuf, int *pLen, FILE *pFp) {
    gfmRV rv;
    int size;
    waveFormat format;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppBuf, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppBuf), GFMRV_ARGUMENTS_BAD);
    ASSERT(pLen, GFMRV_ARGUMENTS_BAD);
    // Rewind the file
    rewind(pFp);
    
    // Read the master chunk and retrieve its size
    rv = gfmAudio_readMasterChunk(&size, pFp);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Clear the format, before it's read
    memset(&format, 0x0, sizeof(waveFormat));
    
    // Finish reading the file
    while (size > 0) {
        riffChunk chunk;
        
        // Read the next chunk
        rv = gfmAudio_readRIFFChunkHeader(&chunk, pFp);
        ASSERT_NR(rv == GFMRV_OK);
        // The chunk header is 8 bytes long, so...
        size -= 8;
        
        // Check which chunk was read...
        if (strcmp(chunk.pId, "fmt ") == 0) {
            // Read the file format
            rv = gfmAudio_readWaveFormat(&format, pFp, chunk.size);
            ASSERT_NR(rv == GFMRV_OK);
            // TODO Check that the format is valid
        }
        else if (strcmp(chunk.pId, "LIST") == 0) {
            // This type of chunk may be ignored
            fseek(pFp, chunk.size, SEEK_CUR);
        }
        else if (strcmp(chunk.pId, "data") == 0) {
            // TODO Read the bytes
            // TODO Convert it to the desired format
            // TODO Store it in the buffer that will be returned
        }
        else {
            // Got an invalid chunk
            ASSERT(0, GFMRV_READ_ERROR);
        }
        
        // Update how many bytes were read
        size -= chunk.size;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

