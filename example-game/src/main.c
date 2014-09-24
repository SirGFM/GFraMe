/**
 * @src/main.c
 */
#include <GFraMe/GFraMe.h>
#include <GFraMe/GFraMe_audio_player.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_log.h>
#include "gameover.h"
#include "global.h"
#include "menustate.h"
#include "playstate.h"

#ifndef GFRAME_MOBILE
void setIcon();
#endif

int main(int argc, char *argv[]) {
	GFraMe_ret rv;
	
	// Init the framework
	rv = GFraMe_init(320, 240, 640, 480, "com.gfmgamecorner",
					 "BugSquasher", GFraMe_window_resizable, 60, 1, 0);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to init the framework", _exit);
	
#ifndef GFRAME_MOBILE
	setIcon();
#endif
	
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
#ifdef GFRAME_MOBILE
GFraMe_log("asd");
	GFraMe_log_close();
GFraMe_log("qwe");
#endif
	GFraMe_audio_player_clear();
	global_clear();
	GFraMe_quit();
	return rv;
}

#ifndef GFRAME_MOBILE
void setIcon() {
	#define alp	0x00,0x00,0x00,0x00
	#define bor	0xff,0x22,0x20,0x34
	#define blu	0xff,0xcb,0xdb,0xfc
	#define gra	0xff,0x84,0x7e,0x87
	#define lre	0xff,0xd9,0x57,0x63
	#define dre	0xff,0xac,0x32,0x32
	#define lin	0xff,0x45,0x28,0x3c
	#define whi	0xff,0xcb,0xdb,0xfc
	#define lgr 0xff,0x99,0xe5,0x50
	#define gre 0xff,0x6a,0xbe,0x30
	#define dgr 0xff,0x4b,0x69,0x2f
	#define eye	0xff,0x32,0x3c,0x39
	#define lye	0xff,0xee,0xc3,0x9a
	#define yel	0xff,0xd9,0xa0,0x66
	#define dye	0xff,0x8a,0x6f,0x30
	
	char pixels[16*16*4] = 
{
alp,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,alp,
bor,blu,blu,blu,blu,blu,blu,blu,blu,blu,blu,blu,blu,blu,blu,bor,
bor,gra,lre,lre,lin,dre,lin,lin,lin,lin,lin,lre,lre,lre,blu,bor,
bor,gra,lre,lre,lre,lre,lre,lre,lre,lre,lre,lin,lre,lre,blu,bor,
bor,gra,lre,eye,eye,gra,gra,dre,lre,lre,lre,lin,lre,lre,blu,bor,
bor,gra,eye,gra,gre,lgr,whi,gra,dre,dre,dre,lre,lre,dre,blu,bor,
bor,gra,eye,gra,gre,gre,whi,gra,dre,dre,dre,dre,lre,dre,blu,bor,
bor,gra,eye,dgr,gra,gre,lgr,gra,dre,dre,dre,dre,lre,dre,blu,bor,
bor,gra,eye,dgr,dgr,gra,gre,eye,dre,dre,dre,dre,lre,dre,blu,bor,
bor,gra,dre,eye,eye,eye,eye,dre,dre,dre,dre,dre,lre,dre,blu,bor,
bor,gra,dre,dre,dre,dre,dre,dre,dre,dre,dre,dre,lre,dre,blu,bor,
bor,gra,dre,dre,lin,lin,lin,lin,lin,lin,lin,lin,lin,lin,blu,bor,
bor,gra,dre,lin,dye,dye,dye,dye,dye,dye,dye,dye,yel,dye,blu,bor,
bor,gra,dre,yel,yel,yel,yel,yel,yel,yel,yel,yel,lye,yel,blu,bor,
bor,gra,gra,gra,gra,gra,gra,gra,gra,gra,gra,gra,gra,gra,gra,bor,
alp,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,bor,alp,
};
	
	GFraMe_set_icon(pixels);
}
#endif
