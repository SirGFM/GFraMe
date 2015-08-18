/**
 * @file tst/gframe_parser_tst.c
 * 
 * Simple parser tst
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>

int main(int  argc, char *argv[]) {
    gfmCtx *pCtx;
    gfmParser *pParser;
    gfmRV rv;
    
    // Set default values
    pCtx = 0;
    pParser = 0;
    
    // Initialize the game's context
    rv = gfm_getNew(&pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_parser_tst");
    ASSERT(rv == GFMRV_OK, rv);
    
    // Initialize the parser
    rv = gfmParser_getNew(&pParser);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmParser_initStatic(pParser, pCtx, "jjat-sample-objs.gfm");
    ASSERT(rv == GFMRV_OK, rv);
    
    // Parse every object on the file
    while (1) {
        rv = gfmParser_parseNext(pParser);
        ASSERT(rv == GFMRV_OK || rv == GFMRV_PARSER_FINISHED, rv);
        if (rv == GFMRV_PARSER_FINISHED) {
            break;
        }
    }
      
    rv = GFMRV_OK;
__ret:
    gfmParser_free(&pParser);
    gfm_free(&pCtx);
    
    return rv;
}

