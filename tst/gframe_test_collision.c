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

static GFraMe_sprite s1, s2;
static GFraMe_spriteset sset;
static GFraMe_texture atlas;
static int dstX, dstY, didInitAssets, running, test;

// Define some variables needed by the events module
GFraMe_event_setup();

#define WND_W 320
#define WND_H 240
#define SPR_H 8
#define SPR_W 8
#define SPR_C 2
#define BPC 4 // Bytes Per Color

enum {
    TEST_INI        = 0,
    TEST_LEFT       = 0,
    TEST_RIGHT      = 1,
    TEST_UP         = 2,
    TEST_DOWN       = 3,
    TEST_LEFT_16x8  = 4,
    TEST_RIGHT_16x8 = 5,
    TEST_UP_16x8    = 6,
    TEST_DOWN_16x8  = 7,
    TEST_END        = 8
};

struct test_params {
    int x;
    int y;
    int w;
    int h;
    int vx;
    int vy;
    int dx;
    int dy;
    char *label;
};

static GFraMe_ret init_assets();
static void clean_assets();

static void event_handler();

int main (int argc, char *argv[]) {
    GFraMe_ret rv;
    int i;
    struct test_params tests[TEST_END];
    
    didInitAssets = 0;
    
    i = TEST_INI;
    while (i < TEST_END) {
        struct test_params *t;
        
        t = &tests[i];
        
        switch (i) {
            case TEST_LEFT: {
                t->w = SPR_W;
                t->h = SPR_H;
                t->x = (WND_W - t->w) / 2 - SPR_W * 4;
                t->y = (WND_H - t->h) / 2;
                t->vx = t->w * 10;
                t->vy = 0;
                t->dx = (WND_W - t->w) /2 - t->w;
                t->dy = (WND_H - t->h) /2;
                t->label = "       TEST_LEFT        ---";
            } break;
            case TEST_RIGHT: {
                t->w = SPR_W;
                t->h = SPR_H;
                t->x = (WND_W - t->w) / 2 + SPR_W * 4;
                t->y = (WND_H - t->h) / 2;
                t->vx = -t->w * 10;
                t->vy = 0;
                t->dx = (WND_W - t->w) / 2 + SPR_W;
                t->dy = (WND_H - t->h) / 2;
                t->label = "      TEST_RIGHT        ---";
            } break;
            case TEST_UP: {
                t->w = SPR_W;
                t->h = SPR_H;
                t->x = (WND_W - t->w) / 2;
                t->y = (WND_H - t->h) / 2 - SPR_H * 4;
                t->vx = 0;
                t->vy = t->h * 10;
                t->dx = (WND_W - t->w) / 2;
                t->dy = (WND_H - t->h) / 2 - t->h;
                t->label = "        TEST_UP         ---";
            } break;
            case TEST_DOWN: {
                t->w = SPR_W;
                t->h = SPR_H;
                t->x = (WND_W - t->w) / 2;
                t->y = (WND_H - t->h) / 2 + SPR_H * 4;
                t->vx = 0;
                t->vy = -t->h * 10;
                t->dx = (WND_W - t->w) / 2;
                t->dy = (WND_H - t->h) / 2 + SPR_H;
                t->label = "       TEST_DOWN        ---";
            } break;
            case TEST_LEFT_16x8: {
                t->w = SPR_W * 2;
                t->h = SPR_H;
                t->x = (WND_W - t->w) / 2 - SPR_W * 4;
                t->y = (WND_H - t->h) / 2;
                t->vx = t->w * 10;
                t->vy = 0;
                t->dx = (WND_W - t->w) / 2 - t->w;
                t->dy = (WND_H - t->h) / 2;
                t->label = " TEST_LEFT (16x8 X 8x8) ---";
            } break;
            case TEST_RIGHT_16x8: {
                t->w = SPR_W * 2;
                t->h = SPR_H;
                t->x = (WND_W - t->w) / 2 + SPR_W * 4;
                t->y = (WND_H - t->h) / 2;
                t->vx = -t->w * 10;
                t->vy = 0;
                t->dx = (WND_W - t->w) / 2 + SPR_W;
                t->dy = (WND_H - t->h) / 2;
                t->label = "TEST_RIGHT (16x8 X 8x8) ---";
            } break;
            case TEST_UP_16x8: {
                t->w = SPR_W;
                t->h = SPR_H * 2;
                t->x = (WND_W - t->w) / 2;
                t->y = (WND_H - t->h) / 2 - SPR_H * 4;
                t->vx = 0;
                t->vy = t->h * 10;
                t->dx = (WND_W - t->w) / 2;
                t->dy = (WND_H - t->h) / 2 - t->h;
                t->label = "  TEST_UP (8x16 X 8x8)  ---";
            } break;
            case TEST_DOWN_16x8: {
                t->w = SPR_W;
                t->h = SPR_H * 2;
                t->x = (WND_W - t->w) / 2;
                t->y = (WND_H - t->h) / 2 + SPR_H * 4;
                t->vx = 0;
                t->vy = -t->h * 10;
                t->dx = (WND_W - t->w) / 2;
                t->dy = (WND_H - t->h) / 2 + SPR_H;
                t->label = " TEST_DOWN (8x16 X 8x8) ---";
            } break;
            default : {
                t->w = 0;
                t->h = 0;
                t->x = 0;
                t->y = 0;
                t->vx = 0;
                t->vy = 0;
                t->dx = 0;
                t->dy = 0;
                t->label = "         UNKNOW         ---";
            }
        }
        i++;
    }
    
    // Init the framework
    rv = GFraMe_init(WND_W, WND_H, WND_W, WND_H, "com.gfmgamecorner",
        "CollisionTest", GFraMe_window_resizable, 0, 60, 0, 0);
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", __ret);
    
    rv = init_assets();
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init assets", __ret);
    
    GFraMe_event_init(60, 60);
    
    running = 1;
    test = TEST_INI;
    while (test < TEST_END) {
        struct test_params *t;
        
        t = &tests[test];
        
        GFraMe_sprite_init(&s1, t->x, t->y, t->w, t->h, &sset, 0, 0);
        s1.cur_tile = 0;
        dstX = t->dx;
        dstY = t->dy;
        GFraMe_sprite_get_object(&s1)->vx = t->vx;
        GFraMe_sprite_get_object(&s1)->vy = t->vy;
        
        GFraMe_sprite_init(&s2, (WND_W - SPR_W) / 2, (WND_H - SPR_H) / 2, SPR_W,
            SPR_H, &sset, 0, 0);
        s2.cur_tile = 1;
        
        // Run the main loop
        running = 1;
        while (running) {
            event_handler();
            GFraMe_event_update_begin();
                GFraMe_sprite_update(&s1, GFraMe_event_elapsed);
                GFraMe_sprite_update(&s2, GFraMe_event_elapsed);
                
                rv = GFraMe_object_overlap(GFraMe_sprite_get_object(&s1),
                    GFraMe_sprite_get_object(&s2), GFraMe_second_fixed);
                
                if (rv == GFraMe_ret_ok) {
                    int resX, resY;
                    
                    resX = GFraMe_sprite_get_object(&s1)->x;
                    resY = GFraMe_sprite_get_object(&s1)->y;
                    
                    GFraMe_log("%s", t->label);
                    if (resX != dstX || resY != dstY)
                        GFraMe_log("     Test failed!");
                    else
                        GFraMe_log("   Test succedded!");
                    GFraMe_log("----------------------- ---");
                    running = 0;
                }
            GFraMe_event_update_end();
            GFraMe_event_draw_begin();
                GFraMe_sprite_draw(&s1);
                GFraMe_sprite_draw(&s2);
            GFraMe_event_draw_end();
        }
        
        test++;
    }
    
__ret:
    
    clean_assets();
    GFraMe_quit();
    return rv;
}

static GFraMe_ret init_assets() {
    GFraMe_ret rv;
    int i;
    unsigned char pixels[SPR_W * SPR_H * SPR_C * BPC];
    
    // Initialize the pixels buffer
    i = 0;
    while (i < SPR_W * SPR_H * SPR_C * BPC) {
        if (i % (SPR_W * SPR_C * BPC) < SPR_W * BPC) {
            pixels[i + 0] = 0xff; // R
            pixels[i + 1] = 0x00; // G
            pixels[i + 2] = 0x00; // B
            pixels[i + 3] = 0xff; // A
        }
        else {
            pixels[i + 0] = 0x00; // R
            pixels[i + 1] = 0x00; // G
            pixels[i + 2] = 0xff; // B
            pixels[i + 3] = 0xff; // A
        }
        i += BPC;
    }
    
    // Load the texture
    rv = GFraMe_texture_load(&atlas, SPR_W * SPR_C, SPR_H, pixels);
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", __ret);
    GFraMe_spriteset_init(&sset, &atlas, SPR_W, SPR_H);
    
    didInitAssets = 1;
    rv = GFraMe_ret_ok;
__ret:
    return rv;
}

static void clean_assets() {
    if (!didInitAssets)
        return;
    GFraMe_texture_clear(&atlas);
}

static void event_handler() {
    GFraMe_event_begin();
        GFraMe_event_on_timer();
        GFraMe_event_on_quit();
            GFraMe_log("Received quit!");
            running = 0;
            test = TEST_END;
    GFraMe_event_end();
}

