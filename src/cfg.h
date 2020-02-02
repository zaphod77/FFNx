/* 
 * FFNx - Complete OpenGL replacement of the Direct3D renderer used in 
 * the original ports of Final Fantasy VII and Final Fantasy VIII for the PC.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * cfg.h - configuration variable definitions
 */

#ifndef _CFG_H_
#define _CFG_H_

#include <confuse.h>

#include "types.h"
#include "log.h"
#include "globals.h"
#include "compile_cfg.h"

extern char *mod_path;
extern cfg_bool_t use_external_movie;
extern cfg_bool_t use_external_music;
extern cfg_bool_t save_textures;
extern char *traced_texture;
extern char *vert_source;
extern char *frag_source;
extern char *yuv_source;
extern char *post_source;
extern cfg_bool_t enable_postprocessing;
extern cfg_bool_t trace_all;
extern cfg_bool_t trace_movies;
extern cfg_bool_t trace_fake_dx;
extern cfg_bool_t trace_direct;
extern cfg_bool_t trace_files;
extern cfg_bool_t trace_loaders;
extern cfg_bool_t trace_lights;
extern cfg_bool_t vertex_log;
extern cfg_bool_t show_fps;
extern cfg_bool_t show_stats;
extern long window_size_x;
extern long window_size_y;
extern long window_pos_x;
extern long window_pos_y;
extern cfg_bool_t preserve_aspect;
extern cfg_bool_t fullscreen;
extern long refresh_rate;
extern cfg_bool_t prevent_rounding_errors;
extern long internal_size_x;
extern long internal_size_y;
extern cfg_bool_t enable_vsync;
extern long field_framerate;
extern long battle_framerate;
extern long worldmap_framerate;
extern long menu_framerate;
extern long chocobo_framerate;
extern long condor_framerate;
extern long submarine_framerate;
extern long gameover_framerate;
extern long credits_framerate;
extern long snowboard_framerate;
extern long highway_framerate;
extern long coaster_framerate;
extern long battleswirl_framerate;
extern cfg_bool_t use_new_timer;
extern cfg_bool_t linear_filter;
extern cfg_bool_t transparent_dialogs;
extern cfg_bool_t mdef_fix;
extern cfg_bool_t fancy_transparency;
extern cfg_bool_t compress_textures;
extern long texture_cache_size;
extern cfg_bool_t use_pbo;
extern cfg_bool_t use_mipmaps;
extern cfg_bool_t skip_frames;
extern cfg_bool_t more_ff7_debug;
extern cfg_bool_t show_applog;
extern cfg_bool_t direct_mode;
extern cfg_bool_t show_missing_textures;
extern cfg_bool_t ff7_popup;
extern cfg_bool_t info_popup;
extern char *load_library;
extern cfg_bool_t opengl_debug;
extern cfg_bool_t movie_sync_debug;
extern cfg_bool_t force_cache_purge;
extern char *renderer_backend;

void read_cfg();

#endif
