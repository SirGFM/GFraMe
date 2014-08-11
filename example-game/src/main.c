/**
 * @src/main.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_screen.h>
#include "global.h"
#include "playstate.h"

int main(int argc, char *argv[]) {
	GFraMe_ret rv;
	// Init the framework
	rv = GFraMe_init(320, 240, 640, 480, "Bug Squasher",
					 GFraMe_window_resizable
					 , 60);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", _exit);
	// Load the textures
	rv = global_init();
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to create global stuff", _exit);
	
	// TODO clear events (there may be a few frames issued)
	// Run the main loop
	gl_running = 1;
	while (gl_running) {
		ps_loop();
	}
	
_exit:
	global_clear();
	GFraMe_quit();
	return rv;
}

