/**
 * @src/main.c
 */
#include <GFraMe/GFraMe_audio_player.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_screen.h>
#include "gameover.h"
#include "global.h"
#include "menustate.h"
#include "playstate.h"

int main(int argc, char *argv[]) {
	GFraMe_ret rv;
	// Init the framework
	rv = GFraMe_init(320, 240, 640, 480, "Bug Squasher",
					 GFraMe_window_resizable
					 , 60);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", _exit);
	// Init the audio player
	rv = GFraMe_audio_player_init();
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the audio", _exit);
	// Load the textures
	rv = global_init();
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to create global stuff", _exit);
	
	GFraMe_audio_player_play_bgm(&gl_song, 0.625);
	
	// TODO clear events (there may be a few frames issued)
	// Run the main loop
	gl_running = 1;
	while (gl_running) {
		ms_loop();
		ps_loop();
		gos_loop();
	}
	
_exit:
	GFraMe_audio_player_clear();
	global_clear();
	GFraMe_quit();
	return rv;
}

