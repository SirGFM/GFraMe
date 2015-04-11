/**
 * @file include/GFraMe/core/gfmBackend_bkend.h
 * 
 * Initializes and finalize a backend
 */
#ifndef __GFMBACKEND_BKEND_H__
#define __GFMBACKEND_BKEND_H__

#include <GFraMe/gfmError.h>

/**
 * Initialize a backend
 * 
 * @return GFMRV_OK, GFMRV_INTERNAL_ERROR, GFMRV_BACKEND_ALREADY_INITIALIZED
 */
gfmRV gfmBackend_init();

/**
 * Finalize a backend
 * 
 * @return GFMRV_OK, GFMRV_BACKEND_NOT_INITIALIZED
 */
gfmRV gfmBackend_finalize();

#endif

