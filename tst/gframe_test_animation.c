/**
 * @file gframe_test_animation.c
 * 
 * Check if animation is OK
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
 * Screen's width
 */
#define SCR_W 80
/**
 * Screen's height
 */
#define SCR_H 70
/**
 * Sprite's (default) width
 */
#define SPR_H 8
/**
 * Sprite's (default) height
 */
#define SPR_W 8
/**
 * For how many milliseconds a animation should play
 */
#define TIME_PER_ANIM 3000

#define alp 0x00,0x00,0x00,0x00
#define lne 0x59,0x56,0x52,0xff
#define drk 0x84,0x7e,0x87,0xff
#define med 0x9b,0xad,0xb7,0xff
#define lgt 0xcb,0xdb,0xfc,0xff
unsigned char data[] = {
alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,
alp,lne,med,lgt,lgt,lgt,alp,alp,alp,alp,alp,alp,alp,alp,alp,alp,
alp,lne,med,lne,lgt,lne,alp,alp,alp,drk,med,lgt,lgt,lgt,alp,alp,
alp,lne,med,med,med,med,alp,alp,alp,lne,med,lne,lgt,lne,alp,alp,
alp,lne,drk,drk,drk,drk,alp,alp,alp,lne,drk,med,med,med,alp,alp,
alp,lne,med,lgt,lgt,lgt,alp,alp,alp,lne,med,drk,drk,drk,alp,alp,
alp,lne,med,med,lgt,med,alp,alp,alp,lne,med,med,med,med,alp,alp,
alp,alp,lne,lne,alp,lne,alp,alp,alp,alp,lne,lne,alp,lne,alp,alp
};

/**
 * Sprites for testing collision
 */
static GFraMe_sprite s;
/**
 * First animation the player should play
 */
static GFraMe_animation anim1;
/**
 * Frames for animation 1
 */
static int animData1[] = {0, 1};
/**
 * Second animation the player should play
 */
static GFraMe_animation anim2;
/**
 * Frames for animation 2
 */
static int animData2[] = {0};
/**
 * Third animation the player should play
 */
static GFraMe_animation anim3;
/**
 * Frames for animation 3
 */
static int animData3[] = {1};
/**
 * Spriteset used by the sprites
 */
static GFraMe_spriteset sset8x8;
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
 * Main function.
 * 
 * @param argc Number of arguments
 * @param argv The actual arguments
 * @return Error code
 */
int main (int argc, char *argv[]) {
    GFraMe_ret rv;
    int time;
    
    // Mark assets as not needing clean up
    didInitAssets = 0;
    
    // Init the framework
    rv = GFraMe_init(SCR_W, SCR_H, WND_W, WND_H, "com.gfmgamecorner",
        "AnimationTest", GFraMe_window_resizable, 0, 60, 0, 0);
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework",
        __ret);
    
    // Init the assets
    rv = init_assets();
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init assets", __ret);
    
    // Init the sprite and its animations
    GFraMe_sprite_init(&s, (SCR_W - SPR_W) / 2, (SCR_H - SPR_H) / 2, SPR_W,
        SPR_H, &sset8x8, 0, 0);
    GFraMe_animation_init(&anim1, 8, animData1, sizeof(animData1)/sizeof(int), 1);
    GFraMe_animation_init(&anim2, 0, animData2, sizeof(animData2)/sizeof(int), 0);
    GFraMe_animation_init(&anim3, 0, animData3, sizeof(animData3)/sizeof(int), 0);
    GFraMe_sprite_set_animation(&s, &anim1, 0);
    
    // Init the framework update and draw clock
    GFraMe_event_init(60, 60);
    
    // Run the main loop
    time = 0;
    running = 1;
    while (running) {
        event_handler();
        GFraMe_event_update_begin();
            // Update the sprites
            GFraMe_sprite_update(&s, GFraMe_event_elapsed);
            
            // Check if the animation should change (do so each 2 seconds)
            time += GFraMe_event_elapsed;
            if (time >= TIME_PER_ANIM * 3) {
                GFraMe_sprite_set_animation(&s, &anim1, 0);
                GFraMe_log("Setting animation: dynamic, frame 0 & 1");
                // Reset the time
                time -= TIME_PER_ANIM * 3;
            }
            else if (time >= TIME_PER_ANIM * 2 && time <= TIME_PER_ANIM * 2 + GFraMe_event_elapsed
                && s.anim != &anim3) {
                GFraMe_sprite_set_animation(&s, &anim3, 0);
                GFraMe_log("Setting animation: static, frame 1");
            }
            else if (time >= TIME_PER_ANIM && time <= TIME_PER_ANIM + GFraMe_event_elapsed
                && s.anim != &anim2) {
                GFraMe_sprite_set_animation(&s, &anim2, 0);
                GFraMe_log("Setting animation: static, frame 0");
            }
        GFraMe_event_update_end();
        GFraMe_event_draw_begin();
            // Draw the sprites
            GFraMe_sprite_draw(&s);
        GFraMe_event_draw_end();
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
    
    // Load the texture
    rv = GFraMe_texture_load(&atlas, SPR_W * 2, SPR_H * 2, data);
    GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", __ret);
    GFraMe_spriteset_init(&sset8x8, &atlas, SPR_W, SPR_H);
    
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
    GFraMe_event_end();
}

