/* util.c -- misc utility functions
 *
 * Copyright (C) 2026 givethesourceplox, fgsfds, Andy Nguyen
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "util.h"
#include "config.h"

static pthread_mutex_t s_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_main_thread;
static int s_main_thread_set = 0;
static int s_compat_delay_ms = 0;

void debugPrintf_setMainThread(void)
{
  s_main_thread = pthread_self();
  s_main_thread_set = 1;
}

void debugPrintf_setCompatDelayMs(int ms)
{
  if (ms < 0)
    ms = 0;
  if (ms > 20)
    ms = 20;
  s_compat_delay_ms = ms;
}

static FILE *s_log_file = NULL;

#if DEBUG_LOG

static int s_nxlinkSock = -1;

static void initNxLink(void)
{
  if (R_FAILED(socketInitializeDefault()))
    return;
  s_nxlinkSock = nxlinkStdio();
  if (s_nxlinkSock < 0)
    socketExit();
}

static void deinitNxLink(void)
{
  if (s_nxlinkSock >= 0)
  {
    close(s_nxlinkSock);
    socketExit();
    s_nxlinkSock = -1;
  }
}

void userAppInit(void)
{
  initNxLink();
}

void userAppExit(void)
{
  deinitNxLink();
}

#endif

void debugPrintf_setFileLog(int enabled)
{
  pthread_mutex_lock(&s_log_mutex);
  if (enabled && !s_log_file)
  {
    s_log_file = fopen(LOG_NAME, "w");
    if (s_log_file)
      setvbuf(s_log_file, NULL, _IOLBF, 0x1000);
  }
  else if (!enabled && s_log_file)
  {
    fclose(s_log_file);
    s_log_file = NULL;
  }
  pthread_mutex_unlock(&s_log_mutex);
}

int debugPrintf_enabled(void)
{
  return DEBUG_LOG || s_log_file != NULL;
}

int debugPrintf(const char *text, ...)
{
  va_list list;
  const char *tag = "?";

  if (!debugPrintf_enabled())
  {
    // Logging used to be compiled in unconditionally, and its serialization is
    // currently masking a startup/gameplay race in the port. Keep the timing
    // workaround (lock + short sleep) without paying for formatting/output.
    if (s_compat_delay_ms > 0)
    {
      pthread_mutex_lock(&s_log_mutex);
      svcSleepThread((int64_t)s_compat_delay_ms * 1000000LL);
      pthread_mutex_unlock(&s_log_mutex);
    }
    return 0;
  }

  if (s_main_thread_set)
    tag = pthread_equal(pthread_self(), s_main_thread) ? "M" : "G";

  pthread_mutex_lock(&s_log_mutex);

  if (s_log_file)
  {
    fprintf(s_log_file, "[%s] ", tag);
    va_start(list, text);
    vfprintf(s_log_file, text, list);
    va_end(list);
    // flush every line so the tail survives a crash
    fflush(s_log_file);
  }

#if DEBUG_LOG
  printf("[%s] ", tag);
  va_start(list, text);
  vprintf(text, list);
  va_end(list);
#endif

  // File logging already serializes callers on real I/O; otherwise emulate a
  // small portion of that delay (see above).
  if (!s_log_file && s_compat_delay_ms > 0)
    svcSleepThread((int64_t)s_compat_delay_ms * 1000000LL);

  pthread_mutex_unlock(&s_log_mutex);
  return 0;
}

int ret0(void) { return 0; }

int ret1(void) { return 1; }

int retm1(void) { return -1; }
