#ifndef __VGMSTREAM_MUSIC_H__
#define __VGMSTREAM_MUSIC_H__

#include <windows.h>
#include <dsound.h>
#include <vgmstream.h>
#include <math.h>
#include <process.h>

#include "../log.h"
#include "../types.h"

void vgm_music_init();
void vgm_play_music(char* midi, uint id);
void vgm_stop_music();
void vgm_cross_fade_music(char* midi, uint id, int time);
void vgm_pause_music();
void vgm_resume_music();
bool vgm_music_status();
void vgm_set_master_music_volume(int volume);
void vgm_set_music_volume(int volume);
void vgm_set_music_volume_trans(int volume, int step);
void vgm_set_music_tempo(unsigned char tempo);

#endif // !__VGMSTREAM_MUSIC_H__
