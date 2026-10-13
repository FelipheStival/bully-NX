/* config.h -- global configuration and config file handling
 *
 * Copyright (C) 2021 fgsfds, Andy Nguyen, givethesourceplox
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

// The 64-bit Bully runtime streams much heavier mesh/audio/script data than the
// original Vita-era ports this loader started from. Give newlib a larger heap
// budget so medium-sized refcounted mesh buffers don't fail late during area
// transitions and cutscene setup.
#define MEMORY_MB 1024

// Address space kept for the mapped libBully.so image. Everything else in the
// process heap goes to newlib, since the game, Mesa and OpenAL all allocate
// from there and a fixed 1 GB cap ran out during long sessions/missions.
#define SO_HEAP_RESERVE_MB 128

#define SO_NAME "libBully.so"
#define CONFIG_NAME "config.txt"
#define LOG_NAME "debug.log"

// Compile-time: mirror log output to stdout/nxlink (development builds only).
#define DEBUG_LOG 0

// actual screen size
extern int screen_width;
extern int screen_height;

typedef struct {
  int screen_width;
  int screen_height;
  int clarity;
  int shadows;
  int trilinear_filter;
  int disable_mipmaps;
  int nintendo_button_glyphs;
  int timing_workaround_ms;
  int debug_log;
  int shader_cache;
  int cpu_boost;
  int gl_thread;
  int audio_quality;
  int phone_mode;
  int anisotropy;
  char gl_driver[0x20];
  char mod_file[0x100];
} Config;

extern Config config;

int read_config(const char *file);
int write_config(const char *file);

#endif
