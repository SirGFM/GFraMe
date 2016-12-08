/**
 * @file tst/gframe_print_texture_bytes.c
 *
 * Loads a texture and print its bytes.
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmVideo_bmp.h>

#include <GFraMe_int/gfmCtx_struct.h>

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv []) {
    gfmCtx *pCtx;
    gfmFile *pFile;
    char *texFile, *pData;
    int colorkey, didLoad, i, width, height;
    gfmRV rv;

    // Initialize every variable
    pCtx = 0;
    texFile = 0;
    pFile = 0;
    pData = 0;
    didLoad = 0;
    colorkey = 0xFF00FF;

    /* Check argc/argv */
    if (argc > 1) {
        int i;

        i = 1;
        while (i < argc) {
#define IS_PARAM(l_cmd, s_cmd) \
  if (strcmp(argv[i], l_cmd) == 0 || strcmp(argv[i], s_cmd) == 0)

#define IS_PARAM_WARGS(l_cmd, s_cmd) \
  if (strcmp(argv[i], l_cmd) == 0 || strcmp(argv[i], s_cmd) == 0) \
      if (argc <= i + 1) { \
            printf("Expected parameter but got nothing! Run " \
                    "'gframe_print_texture_bytes --help' for usage!\n"); \
            return 1; \
      } \
      else
#define GET_NUM(num) \
  do { \
    char *pNum; \
    int tmp; \
    pNum = argv[i + 1]; \
    tmp = 0; \
    /* TODO Support hex */ \
    while (*pNum != '\0') { \
        tmp = tmp * 10 + (*pNum) - '0'; \
        pNum++; \
    } \
    num = tmp; \
  } while (0)

            IS_PARAM("--help", "-h") {
                printf("Prints a texture's bytes (encoded as 32 bits, 0xRR, "
                            "0xGG, 0xBB, 0xAA)\n"
                        "\n"
                        "Usage: gframe_print_texture_bytes --input | -i "
                            "<INPUT_FILE>\n"
                        "                                  [--colorkey | -c "
                            "<COLOR>]\n"
                        "\n"
                        "Description:\n"
                        "\n"
                        "  This test loads a texture from the assets directory "
                            "and prints its content as\n  a C array.\n"
                        "\n"
                        "Options:\n"
                        "    --input | -i <INPUT_FILE>\n"
                        "        Texture to be loaded. Must be on a 'assets/' "
                            "directory\n"
                        "\n"
                        "    --colorkey | -c <COLOR>\n"
                        "        Color to be considered as alpha, in RGB, 24 "
                            "bits\n");
                return 0;
            }
            IS_PARAM_WARGS("--input", "-i") {
                texFile = argv[i + 1];
                i++;
            }
            IS_PARAM_WARGS("--colorkey", "-c") {
                GET_NUM(colorkey);
                i++;
            }
#undef IS_PARAM
#undef IS_PARAM_WARG

            i++;
        }
    }

    if (texFile == 0) {
        printf("No input file was specified! Run 'gframe_print_texture_bytes "
                "'--help' for usage!\n");
        return GFMRV_ARGUMENTS_BAD;
    }

    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_print_texture_bytes");
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfmFile_getNew(&pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmFile_openAsset(pFile, pCtx, texFile, strlen(texFile), 0/*isText*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    rv = gfmVideo_isBmp(pFile, pCtx->pLog);
    if (rv == GFMRV_TRUE) {
        rv = gfmVideo_loadFileAsBmp(&pData, &width, &height, pFile, pCtx->pLog,
                colorkey);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        didLoad = 1;
    }
    /* TODO Support other formats */
    ASSERT_LOG(didLoad == 1, GFMRV_TEXTURE_UNSUPPORTED, pCtx->pLog);

    printf("static const int _width = %i;\n", width);
    printf("static const int _height = %i;\n", height);
    printf("static const char _data[%i] = {\n", width * height * 4);
    i = 0;
    while (i < width * height) {
        printf("    0x%02X, 0x%02X, 0x%02X, 0x%02X,\n"
                , (int)pData[i * 4 + 0] & 0xFF, (int)pData[i * 4 + 1] & 0xFF
                , (int)pData[i * 4 + 2] & 0xFF, (int)pData[i * 4 + 3] & 0xFF);
        i++;
    }
    printf("};\n");

    rv = GFMRV_OK;
__ret:
    if (pData) {
        free(pData);
    }
    if (pFile) {
        gfmFile_free(&pFile);
    }
    gfm_free(&pCtx);

    return rv;
}

