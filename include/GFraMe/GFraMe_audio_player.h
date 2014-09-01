/**
 * @include/GFraMe/GFraMe_audio_player.h
 */
#ifndef __GFRAME_AUDIO_PLAYER_H_
#define __GFRAME_AUDIO_PLAYER_H_

#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL_audio.h>

GFraMe_ret GFraMe_audio_player_init();
void GFraMe_audio_player_clear();
SDL_AudioSpec* GFraMe_audio_player_get_spec();
void GFraMe_audio_player_play_bgm(GFraMe_audio *aud, double volume);
void GFraMe_audio_player_push(GFraMe_audio *aud, double volume);

#endif

