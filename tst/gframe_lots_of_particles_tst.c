/**
 * @file tst/gframe_lots_of_particles_tst.c
 * 
 * Simple test with group
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmText.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

// Set the game's FPS
#define WNDW     160
#define WNDH     120


int main(int argc, char *argv[]) {
    gfmCtx *pCtx;
    gfmGroup *pGrp;
    gfmRV rv;
    gfmSprite *pSpr, *pPlayer;
    gfmSpriteset *pSset4, *pSset8;
    gfmVideoBackend vbk;
    int iTex;
    int quit, left, right, up, down;
    int ttl;

    int fps, fullscreen, particles, simple, vsync;
    
    // Initialize every variable
    pCtx = 0;
    pGrp = 0;
    pPlayer = 0;
    ttl = 2000;

    /* Set default values */
    fps = 60;
    fullscreen = 0;
    particles = 60;
    simple = 0;
    vsync = 0;
    vbk = GFM_VIDEO_SDL2;

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
                    "'gframe_lots_of_particles_tst --help' for usage!\n"); \
            return 1; \
      } \
      else
#define GET_NUM(num) \
  do { \
    char *pNum; \
    int tmp; \
    pNum = argv[i + 1]; \
    tmp = 0; \
    while (*pNum != '\0') { \
        tmp = tmp * 10 + (*pNum) - '0'; \
        pNum++; \
    } \
    num = tmp; \
  } while (0)

            IS_PARAM("--help", "-h") {
                printf("Test FPS when rendering lots of particles\n"
                        "\n"
                        "Usage: gframe_lots_of_particles_tst [--vsync | -v] "
                                "[--backend | -b <vbk_type>]\n"
                        "                                    [--fps | -f <FPS>]"
                                " [--particles | -p <PPF>]\n"
                        "                                    [--simple | -s] "
                                "[--fullscreen | -F]\n"
                        "\n"
                        "Description:\n"
                        "\n"
                        "  This test spawns a number of particles every frame. "
                                "The spawner may be moved\n"
                        "with a gamepad's left stick, its D-Pad, the arrow "
                                "keys or WASD.\n"
                        "\n"
                        "  'ESC' quits\n"
                        "\n"
                        "  At the upper-left corner some info is displayed:\n"
                        "    'U XXX/XXX XXXX' - In order, how many frames were "
                                "updated on the last\n"
                        "                    second, expected "
                                "updates-per-second rate (i.e., FPS),\n"
                        "                    time spent on each update loop\n"
                        "    'D XXX/XXX XXXX' - In order, how many frames were "
                                "rendered on the last\n"
                        "                    second, expected draws-per-second "
                                "rate (i.e., FPS),\n"
                        "                    time spent on each draw loop\n"
                        "    'B XXXXX'  - Number of batched draws on the last "
                                "frame\n"
                        "    'N XXXXX' - Number of sprites rendered on the "
                                "last frame\n"
                        "\n"
                        "Options:\n"
                        "    --vsync | -v\n"
                        "        Enables VSYNC (default: disabled)\n"
                        "\n"
                        "    --backend | -b <vbk_type>\n"
                        "        Select which video backend to be used, "
                                "vbk_type may be either SDL2 or\n"
                        "        OpenGL. The OpenGL backend depends on "
                                "OpenGL 3.1, since it uses\n"
                        "        instanced rendering.\n"
                        "\n"
                        "        vbk_type = SDL2 | OpenGL (default: SDL2)\n"
                        "\n"
                        "    --fps | -f <FPS>\n"
                        "        Select the desired FPS (default: 60)\n"
                        "\n"
                        "        Tip: Check whether your system can run at 120 "
                                "FPS (VSYNC may have to be\n"
                        "        disabled, if your monitor doesn't refresh at "
                                "120Hz)\n"
                        "\n"
                        "    --particles | -p <PPF>\n"
                        "        How many particles per frame may be spawned "
                                "(default: 60)\n"
                        "\n"
                        "    --simple | -s\n"
                        "        Don't use sin() and cos() on the update loop "
                                "(may increase update rate)\n"
                        "        (default: disabled)\n"
                        "\n"
                        "    --fullscreen | -F\n"
                        "        Make the test run in fullscreen\n");
                return 0;
            }
            IS_PARAM_WARGS("--backend", "-b") {
                if (strcmp(argv[i + 1], "SDL2") == 0) {
                    vbk = GFM_VIDEO_SDL2;
                }
                else if (strcmp(argv[i + 1], "OpenGL") == 0) {
                    vbk = GFM_VIDEO_GL3;
                }

                i++;
            }
            IS_PARAM("--vsync", "-v") {
                vsync = 1;
            }
            IS_PARAM_WARGS("--fps", "-f") {
                GET_NUM(fps);

                i++;
            }
            IS_PARAM_WARGS("--particles", "-p") {
                GET_NUM(particles);

                i++;
            }
            IS_PARAM("--simple", "-s") {
                simple = 1;
            }
            IS_PARAM("--fullscreen", "-F") {
                fullscreen = 1;
            }

#undef IS_PARAM
#undef IS_PARAM_WARG

            i++;
        }
    }
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);

    /* Select the video backend */
    rv = gfm_setVideoBackend(pCtx, vbk);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_input");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    if (!fullscreen) {
        rv = gfm_initGameWindow(pCtx, WNDW, WNDH, 640, 480, 1, vsync);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        rv = gfm_initGameFullScreen(pCtx, WNDW, WNDH, 0, 0, vsync);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // Create the inputs
    rv = gfm_addVirtualKey(&quit, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, quit, gfmKey_esc);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfm_addVirtualKey(&left, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, left, gfmKey_left);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, left, gfmKey_a);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfm_addVirtualKey(&right, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, right, gfmKey_right);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, right, gfmKey_d);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfm_addVirtualKey(&up, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, up, gfmKey_up);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, up, gfmKey_w);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfm_addVirtualKey(&down, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, down, gfmKey_down);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, down, gfmKey_s);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "rainbow_atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    // Set it as the default
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the spritesets
    rv = gfm_createSpritesetCached(&pSset8, pCtx, iTex, 8/*tw*/, 8/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_createSpritesetCached(&pSset4, pCtx, iTex, 4/*tw*/, 4/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initalize the FPS counter
    rv = gfm_initFPSCounter(pCtx, pSset8, 0/*firstTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create a 'player'
    rv = gfmSprite_getNew(&pPlayer);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize it
    rv = gfmSprite_init(pPlayer, WNDW / 2, WNDH / 2, 8/*w*/, 8/*h*/, pSset8,
            -4/*offX*/, -4/*offY*/, 0, 0);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setFrame(pPlayer, '*' - '!');
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the group
    rv = gfmGroup_getNew(&pGrp);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the group attributes
    rv = gfmGroup_setDefSpriteset(pGrp, pSset4);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmGroup_setDefDimensions(pGrp, 4/*width*/, 4/*height*/, 0/*offX*/,
        0/*offY*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Create the group's sprite
    rv = gfmGroup_preCache(pGrp, 0/*initLen*/, 12288/*maxLen*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Those can be set after caching everything, since they are global
    rv = gfmGroup_setDeathOnTime(pGrp, ttl);
    ASSERT_NR(rv == GFMRV_OK);
    // And those are set when the sprite is recycled, so...
    rv = gfmGroup_setDefVelocity(pGrp, 0/*vx*/, -175/*vy*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmGroup_setDefAcceleration(pGrp, 0/*vx*/, 100/*vy*/);
    ASSERT_NR(rv == GFMRV_OK);
    if (simple) {
        // Set the draw order (BOOO... BORING!!!)
        rv = gfmGroup_setDrawOrder(pGrp, gfmDrawOrder_linear);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        // Set the draw order (FUN!!!)
        rv = gfmGroup_setDrawOrder(pGrp, gfmDrawOrder_oldestFirst);
        ASSERT_NR(rv == GFMRV_OK);
    }
    // Uncommenting enable the collision list
    //rv = gfmGroup_setCollisionQuality(pGrp,
    //        gfmCollisionQuality_collideEverything);
    //ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, fps, fps);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, fps);
    if (rv == GFMRV_FPS_TOO_HIGH) {
        rv = gfm_setRawFPS(pCtx, fps);
    }
    ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        while (gfm_isUpdating(pCtx) == GFMRV_TRUE) {
            float vx, vy;
            gfmInput *pInput;
            gfmInputState kquit;
            int i, nquit, x, y;
            
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_getInput(&pInput, pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            rv = gfmInput_getKeyState(&kquit, &nquit, pInput, quit);
            ASSERT_NR(rv == GFMRV_OK);
            if (kquit & gfmInput_justReleased) {
                rv = gfm_setQuitFlag(pCtx);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            rv = gfmInput_getGamepadAnalog(&vx, &vy, pInput, 0/*port*/,
                    gfmController_leftAnalog);
            ASSERT_NR(rv == GFMRV_OK);
            
            if (vx*vx + vy*vy > 0.3*0.3) {
                rv = gfmSprite_setVelocity(pPlayer, vx * 100.0f, vy * 100.0f);
            }
            else {
                gfmInputState kleft, kright, kup, kdown;
                int nleft, nright, nup, ndown;

                rv = gfmInput_getKeyState(&kleft, &nleft, pInput, left);
                ASSERT_NR(rv == GFMRV_OK);
                rv = gfmInput_getKeyState(&kright, &nright, pInput, right);
                ASSERT_NR(rv == GFMRV_OK);
                rv = gfmInput_getKeyState(&kup, &nup, pInput, up);
                ASSERT_NR(rv == GFMRV_OK);
                rv = gfmInput_getKeyState(&kdown, &ndown, pInput, down);
                ASSERT_NR(rv == GFMRV_OK);

                if (kleft & gfmInput_pressed) {
                    vx = -100.0f;
                }
                else if (kright & gfmInput_pressed) {
                    vx = 100.0f;
                }
                else {
                    vx = 0.0f;
                }
                if (kup & gfmInput_pressed) {
                    vy = -100.0f;
                }
                else if (kdown & gfmInput_pressed) {
                    vy = 100.0f;
                }
                else {
                    vy = 0.0f;
                }

                rv = gfmSprite_setVelocity(pPlayer, vx, vy);
            }
            ASSERT_NR(rv == GFMRV_OK);
            
            
            // Get the sprite's position
            rv = gfmSprite_getPosition(&x, &y, pPlayer);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Recycle a few sprites
            i = 0;
            while (i < particles) {
                // Update each particle time to live
                rv = gfmGroup_setDeathOnTime(pGrp, ttl);
                ASSERT_NR(rv == GFMRV_OK);
                ttl -= 250;
                if (ttl <= 0) {
                    ttl = 2000;
                }
                
                rv = gfmGroup_recycle(&pSpr, pGrp);
                ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_GROUP_MAX_SPRITES);
                if (rv == GFMRV_OK) {
                    static float ang = 0.0f;
                    int vx, vy;

                    rv = gfmGroup_setPosition(pGrp, x, y);
                    ASSERT_NR(rv == GFMRV_OK);
                    rv = gfmGroup_setFrame(pGrp, i % 7);
                    ASSERT_NR(rv == GFMRV_OK);

                    vx = 50 * cosf(ang);
                    vy = 50 * sinf(ang);
                    ang += 3.1415926 / 180.0f;

                    rv = gfmGroup_setVelocity(pGrp, vx, vy);
                    ASSERT_NR(rv == GFMRV_OK);
                }
                i++;
            }
            
            // Update group
            rv = gfmGroup_update(pGrp, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            // Update the player's physics
            rv = gfmSprite_update(pPlayer, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
        
        // Draw stuff
        while (gfm_isDrawing(pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Draw group
            rv = gfmGroup_draw(pGrp, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            // Draw the 'player'
            rv = gfmSprite_draw(pPlayer, pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            /* Draw last frame's render info */
            rv = gfm_drawRenderInfo(pCtx, pSset8, 0, 16, 0);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }
    
    rv = GFMRV_OK;
__ret:
    gfmSprite_free(&pPlayer);
    gfmGroup_free(&pGrp);
    gfm_free(&pCtx);
    
    return rv;
}

