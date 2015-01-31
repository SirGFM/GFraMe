/**
 * @file gframe_test_collision.c
 * 
 * Check if collision behaves correctly
 */
#include <GFraMe/GFraMe.h>
#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_texture.h>

/**
 * Window's width
 */
#define WND_W 320
/**
 * Window's height
 */
#define WND_H 240
/**
 * Sprite's (default) width
 */
#define SPR_H 8
/**
 * Sprite's (default) height
 */
#define SPR_W 8
/**
 * Horizontal sprite (tiles) count
 */
#define SPR_HC 4
/**
 * Vertical sprite (tiles) count
 */
#define SPR_VC 4
/**
 * Bytes per color (packet in RGBA format)
 */
#define BPC 4

/**
 * Anonymous enumeration for tests
 */
enum {
    TEST_LEFT,
    TEST_RIGHT,
    TEST_UP,
    TEST_DOWN,
    TEST_LEFT_16x8,
    TEST_LEFT_16x8_B,
    TEST_LEFT_16x8_A,
    TEST_RIGHT_16x8,
    TEST_RIGHT_16x8_B,
    TEST_RIGHT_16x8_A,
    TEST_UP_16x8,
    TEST_UP_16x8_L,
    TEST_UP_16x8_R,
    TEST_DOWN_16x8,
    TEST_DOWN_16x8_L,
    TEST_DOWN_16x8_R,
    TEST_LEFT_8x16,
    TEST_LEFT_8x16_B,
    TEST_LEFT_8x16_A,
    TEST_RIGHT_8x16,
    TEST_RIGHT_8x16_B,
    TEST_RIGHT_8x16_A,
    TEST_UP_8x16,
    TEST_UP_8x16_L,
    TEST_UP_8x16_R,
    TEST_DOWN_8x16,
    TEST_DOWN_8x16_L,
    TEST_DOWN_8x16_R,
    TEST_END,
    TEST_INI = TEST_LEFT
};
enum col_side {
    COL_LEFT,
    COL_RIGHT,
    COL_UP,
    COL_DOWN,
    COL_NONE
};

/**
 * Store sprite1's initial state
 */
struct test_params {
    GFraMe_spriteset *sset1;
    GFraMe_spriteset *sset2;
    int tile1;
    int tile2;
    int x1;
    int y1;
    int w1;
    int h1;
    int x2;
    int y2;
    int w2;
    int h2;
    int vx;
    int vy;
    int dx;
    int dy;
    char *label;
};

/**
 * Sprites for testing collision
 */
static GFraMe_sprite s1, s2;
/**
 * Spriteset used by the sprites
 */
static GFraMe_spriteset sset8x8;
/**
 * Spriteset used by the sprites
 */
static GFraMe_spriteset sset16x8;
/**
 * Spriteset used by the sprites
 */
static GFraMe_spriteset sset8x16;
/**
 * Spriteset used by the sprites
 */
static GFraMe_spriteset sset16x16;
/**
 * Texture used by the sprites
 */
static GFraMe_texture atlas;
/**
 * Whether the assets where initialized
 */
static int didInitAssets;
/**
 * Keep the main loop running
 */
static int running;
/**
 * Keep track of the current test
 */
static int test;
/**
 * Store every possible test configuration
 */
struct test_params tests[TEST_END];

// Define some variables needed by the events module
GFraMe_event_setup();

/**
 * Initialize the assets
 * 
 * return GFraMe error code
 */
static GFraMe_ret init_assets();
/**
 * Clean up assets
 */
static void clean_assets();
/**
 * Check which event ocurred (necessary for stepping the timer and quitting)
 */
static void event_handler();
/**
 * Initialize the tests parameters
 */
static void init_tests();

/**
 * Easily initialize a test
 * 
 * @param t Structure that will hold the test params
 * @param w1 Moving sprite's width
 * @param h1 Moving sprite's height
 * @param w2 Static sprite's width
 * @param h2 Static sprite's height
 * @param ox Moving sprite's horizontal position offset
 * @param oy Moving sprite's vertical position offset
 * @param c Side that the collision will happenon the fixed sprite
 */
static void init_test(struct test_params *t, int w1, int h1, int w2, int h2,
    int ox, int oy, enum col_side c);

/**
 * Main function.
 * 
 * @param argc Number of arguments
 * @param argv The actual arguments
 * @return Error code
 */
int main (int argc, char *argv[]) {
    GFraMe_ret rv;
    
    // Mark assets as not needing clean up
    didInitAssets = 0;
    
    // Init the framework
    rv = GFraMe_init(WND_W, WND_H, WND_W, WND_H, "com.gfmgamecorner",
        "CollisionTest", GFraMe_window_resizable, 0, 60, 0, 0);
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", __ret);
    
    // Init the assets
    rv = init_assets();
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init assets", __ret);
    
    // Initialize the tests
    init_tests();
    
    // Init the framework update and draw clock
    GFraMe_event_init(60, 60);
    
    // Run every test
    running = 1;
    test = TEST_INI;
    while (test < TEST_END) {
        struct test_params *t;
        
        // Get the current test configuration and the both players' params
        t = &tests[test];
        
        GFraMe_sprite_init(&s1, t->x1, t->y1, t->w1, t->h1, t->sset1, 0, 0);
        s1.cur_tile = t->tile1;
        GFraMe_sprite_get_object(&s1)->vx = t->vx;
        GFraMe_sprite_get_object(&s1)->vy = t->vy;
        
        GFraMe_sprite_init(&s2, t->x2, t->y2, t->w2, t->h2, t->sset2, 0, 0);
        s2.cur_tile = t->tile2;
        
        // Run the main loop
        running = 1;
        while (running) {
            event_handler();
            GFraMe_event_update_begin();
                // Update both sprites
                GFraMe_sprite_update(&s1, GFraMe_event_elapsed);
                GFraMe_sprite_update(&s2, GFraMe_event_elapsed);
                
                // Collide s1 with s2, but only move (i.e., separate) s1, on
                // collision
                rv = GFraMe_object_overlap(GFraMe_sprite_get_object(&s1),
                    GFraMe_sprite_get_object(&s2), GFraMe_second_fixed);
                
                // If they collided, check the result and go to the next one
                if (rv == GFraMe_ret_ok) {
                    int resX, resY;
                    
                    resX = GFraMe_sprite_get_object(&s1)->x;
                    resY = GFraMe_sprite_get_object(&s1)->y;
                    
                    GFraMe_log("%s %s", 
                        (resX != t->dx || resY != t->dy)?
                            ("[FAIL]"):(" [OK] "), t->label);
                    running = 0;
                }
            GFraMe_event_update_end();
            GFraMe_event_draw_begin();
                // Draw the sprites
                GFraMe_sprite_draw(&s1);
                GFraMe_sprite_draw(&s2);
            GFraMe_event_draw_end();
        }
        
        test++;
    }
    
__ret:
    
    // Clean up everything
    clean_assets();
    GFraMe_quit();
    return rv;
}

/**
 * Initialize the assets
 * 
 * return GFraMe error code
 */
static GFraMe_ret init_assets() {
    GFraMe_ret rv;
    int i;
    unsigned char pixels[SPR_W * SPR_H * SPR_HC * SPR_VC * BPC];
    
    // Initialize the pixels buffer
    i = 0;
    while (i < SPR_W * SPR_H * SPR_HC * SPR_VC * BPC) {
        // Create a red tile ...
        if (i % (SPR_W * SPR_HC * BPC) < SPR_W * SPR_HC * BPC / 2) {
            pixels[i + 0] = 0xff; // R
            pixels[i + 1] = 0x00; // G
            pixels[i + 2] = 0x00; // B
            pixels[i + 3] = 0xff; // A
        }
        // ... and a blue one
        else {
            pixels[i + 0] = 0x00; // R
            pixels[i + 1] = 0x00; // G
            pixels[i + 2] = 0xff; // B
            pixels[i + 3] = 0xff; // A
        }
        i += BPC;
    }
    
    // Load the texture
    rv = GFraMe_texture_load(&atlas, SPR_W * SPR_HC, SPR_H * SPR_VC, pixels);
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", __ret);
    GFraMe_spriteset_init(&sset8x8, &atlas, SPR_W, SPR_H);
    GFraMe_spriteset_init(&sset16x8, &atlas, SPR_W * 2, SPR_H);
    GFraMe_spriteset_init(&sset8x16, &atlas, SPR_W, SPR_H * 2);
    GFraMe_spriteset_init(&sset16x16, &atlas, SPR_W * 2, SPR_H * 2);
    
    didInitAssets = 1;
    rv = GFraMe_ret_ok;
__ret:
    return rv;
}

/**
 * Clean up assets
 */
static void clean_assets() {
    if (!didInitAssets)
        return;
    GFraMe_texture_clear(&atlas);
}

/**
 * Check which event ocurred (necessary for stepping the timer and quitting)
 */
static void event_handler() {
    GFraMe_event_begin();
        GFraMe_event_on_timer();
        GFraMe_event_on_quit();
            GFraMe_log("Received quit!");
            running = 0;
            test = TEST_END;
    GFraMe_event_end();
}

/**
 * Initialize the tests parameters
 */
static void init_tests() {
    int i;
    
    i = TEST_INI;
    while (i < TEST_END) {
        struct test_params *t;
        
        t = &tests[i];
        
        switch (i) {
            case TEST_LEFT: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H, 0, 0, COL_LEFT);
                t->label = "8x8 X 8x8 - moving left to right, centered";
            } break;
            case TEST_RIGHT: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H, 0, 0, COL_RIGHT);
                t->label = "8x8 X 8x8 - moving right to left, centered";
            } break;
            case TEST_UP: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H, 0, 0, COL_UP);
                t->label = "8x8 X 8x8 - moving downward, centered";
            } break;
            case TEST_DOWN: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H, 0, 0, COL_DOWN);
                t->label = "8x8 X 8x8 - moving upward, centered";
            } break;
            case TEST_LEFT_16x8: {
                init_test(t, SPR_W, SPR_H * 2, SPR_W, SPR_H, 0, 0, COL_LEFT);
                t->label = "8x16 X 8x8 - moving left to right, centered";
            } break;
            case TEST_RIGHT_16x8: {
                init_test(t, SPR_W, SPR_H * 2, SPR_W, SPR_H, 0, 0, COL_RIGHT);
                t->label = "8x16 X 8x8 - moving right to left, centered";
            } break;
            case TEST_UP_16x8: {
                init_test(t, SPR_W * 2, SPR_H, SPR_W, SPR_H, 0, 0, COL_UP);
                t->label = "16x8 X 8x8 - moving downward, centered";
            } break;
            case TEST_DOWN_16x8: {
                init_test(t, SPR_W * 2, SPR_H, SPR_W, SPR_H, 0, 0, COL_DOWN);
                t->label = "16x8 X 8x8  - moving upward, centered";
            } break;
            case TEST_LEFT_16x8_B: {
                init_test(t, SPR_W, SPR_H * 2, SPR_W, SPR_H, 0, SPR_H, COL_LEFT);
                t->label = "8x16 X 8x8 - moving left to right, bellow";
            } break;
            case TEST_LEFT_16x8_A: {
                init_test(t, SPR_W, SPR_H * 2, SPR_W, SPR_H, 0, -SPR_H, COL_LEFT);
                t->label = "8x16 X 8x8 - moving left to right, above";
            } break;
            case TEST_RIGHT_16x8_B: {
                init_test(t, SPR_W, SPR_H * 2, SPR_W, SPR_H, 0, SPR_W, COL_RIGHT);
                t->label = "8x16 X 8x8 - moving right to left, bellow";
            } break;
            case TEST_RIGHT_16x8_A: {
                init_test(t, SPR_W, SPR_H * 2, SPR_W, SPR_H, 0, -SPR_W, COL_RIGHT);
                t->label = "8x16 X 8x8 - moving right to left, above";
            } break;
            case TEST_UP_16x8_L: {
                init_test(t, SPR_W * 2, SPR_H, SPR_W, SPR_H, -SPR_W, 0, COL_UP);
                t->label = "16x8 X 8x8 - moving downward, to the left";
            } break;
            case TEST_UP_16x8_R: {
                init_test(t, SPR_W * 2, SPR_H, SPR_W, SPR_H, SPR_W, 0, COL_UP);
                t->label = "16x8 X 8x8 - moving downward, to the right";
            } break;
            case TEST_DOWN_16x8_L: {
                init_test(t, SPR_W * 2, SPR_H, SPR_W, SPR_H, -SPR_W, 0, COL_DOWN);
                t->label = "16x8 X 8x8  - moving upward, to the left";
            } break;
            case TEST_DOWN_16x8_R: {
                init_test(t, SPR_W * 2, SPR_H, SPR_W, SPR_H, SPR_W, 0, COL_DOWN);
                t->label = "16x8 X 8x8  - moving upward, to the right";
            } break;
            case TEST_LEFT_8x16: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H * 2, 0, 0, COL_LEFT);
                t->label = "8x8 X 8x16 - moving left to right, centered";
            } break;
            case TEST_LEFT_8x16_B: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H * 2, 0, SPR_H, COL_LEFT);
                t->label = "8x8 X 8x16 - moving left to right, bellow";
            } break;
            case TEST_LEFT_8x16_A: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H * 2, 0, -SPR_H, COL_LEFT);
                t->label = "8x8 X 8x16 - moving left to right, above";
            } break;
            case TEST_RIGHT_8x16: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H * 2, 0, 0, COL_RIGHT);
                t->label = "8x8 X 8x16 - moving right to left, centered";
            } break;
            case TEST_RIGHT_8x16_B: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H * 2, 0, SPR_H, COL_RIGHT);
                t->label = "8x8 X 8x16 - moving right to left, bellow";
            } break;
            case TEST_RIGHT_8x16_A: {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H * 2, 0, -SPR_H, COL_RIGHT);
                t->label = "8x8 X 8x16 - moving right to left, above";
            } break;
            case TEST_UP_8x16: {
                init_test(t, SPR_W, SPR_H, SPR_W * 2, SPR_H, 0, 0, COL_UP);
                t->label = "8x8 X 16x8 - moving downward, centered";
            } break;
            case TEST_UP_8x16_L: {
                init_test(t, SPR_W, SPR_H, SPR_W * 2, SPR_H, -SPR_W, 0, COL_UP);
                t->label = "8x8 X 16x8 - moving downward, to the left";
            } break;
            case TEST_UP_8x16_R: {
                init_test(t, SPR_W, SPR_H, SPR_W * 2, SPR_H, SPR_W, 0, COL_UP);
                t->label = "8x8 X 16x8 - moving downward, to the right";
            } break;
            case TEST_DOWN_8x16: {
                init_test(t, SPR_W, SPR_H, SPR_W * 2, SPR_H, 0, 0, COL_DOWN);
                t->label = "8x8 X 16x8 - moving upward, centered";
            } break;
            case TEST_DOWN_8x16_L: {
                init_test(t, SPR_W, SPR_H, SPR_W * 2, SPR_H, -SPR_W, 0, COL_DOWN);
                t->label = "8x8 X 16x8 - moving upward, to the left";
            } break;
            case TEST_DOWN_8x16_R: {
                init_test(t, SPR_W, SPR_H, SPR_W * 2, SPR_H, SPR_W, 0, COL_DOWN);
                t->label = "8x8 X 16x8 - moving upward, to the right";
            } break;
            default : {
                init_test(t, SPR_W, SPR_H, SPR_W, SPR_H, 0, 0, COL_NONE);
                t->label = "         UNKNOW         ---";
            }
        }
        i++;
    }
}

/**
 * Set the spriteset and tile according to the tile's width and height.
 * 
 * @param sset Returned spriteset
 * @param tile Returned tile
 * @param w Sprite's width
 * @param h Sprite's height
 */
static void get_spriteset(GFraMe_spriteset **sset, int *tile, int w, int h) {
    if (w == 8 && h == 8) {
        *sset = &sset8x8;
        if (tile) *tile = 2;
    }
    else if (w == 16 && h == 8) {
        *sset = &sset16x8;
        if (tile) *tile = 1;
    }
    else if (w == 8 && h == 16) {
        *sset = &sset8x16;
        if (tile) *tile = 2;
    }
    else if (w == 16 && h == 16) {
        *sset = &sset16x16;
        if (tile) *tile = 1;
    }
}

/**
 * Easily initialize a test
 * 
 * @param t Structure that will hold the test params
 * @param w1 Moving sprite's width
 * @param h1 Moving sprite's height
 * @param w2 Static sprite's width
 * @param h2 Static sprite's height
 * @param ox Moving sprite's horizontal position offset
 * @param oy Moving sprite's vertical position offset
 * @param c Side that the collision will happenon the fixed sprite
 */
static void init_test(struct test_params *t, int w1, int h1, int w2, int h2,
    int ox, int oy, enum col_side c) {
    // The the sprites dimensions
    t->w1 = w1;
    t->w2 = w2;
    t->h1 = h1;
    t->h2 = h2;
    
    // Also, set the spriteset and tile accordingly
    get_spriteset(&t->sset1, NULL, w1, h1);
    t->tile1 = 0;
    get_spriteset(&t->sset2, &t->tile2, w2, h2);
    
    // Center both sprites
    t->x1 = (WND_W - t->w1) / 2;
    t->y1 = (WND_H - t->h1) / 2;
    t->x2 = (WND_W - t->w2) / 2;
    t->y2 = (WND_H - t->h2) / 2;
    
    switch (c) {
        case COL_LEFT: {
            t->x1 -= w2 * 4 + ox;
            t->y1 += oy;
            t->vx = w2 * 4 + ox;
            t->vy = 0;
            t->dx = t->x2 - w1;
            t->dy = t->y1;
        } break;
        case COL_RIGHT: {
            t->x1 += w2 * 4 + ox;
            t->y1 += oy;
            t->vx = -w2 * 4 + ox;
            t->vy = 0;
            t->dx = t->x2 + w2;
            t->dy = t->y1;
        } break;
        case COL_UP: {
            t->x1 += ox;
            t->y1 -= h2 * 4 + oy;
            t->vx = 0;
            t->vy = h2 * 4 + oy;
            t->dx = t->x1;
            t->dy = t->y2 - h1;
        } break;
        case COL_DOWN: {
            t->x1 += ox;
            t->y1 += h2 * 4 + oy;
            t->vx = 0;
            t->vy = -h2 * 4 + oy;
            t->dx = t->x1;
            t->dy = t->y2 + h2;
        } break;
        default: {
            t->x1 = 0;
            t->y1 = 0;
            t->x2 = 0;
            t->y2 = 0;
            t->vx = 0;
            t->vy = 0;
        }
    }
}

