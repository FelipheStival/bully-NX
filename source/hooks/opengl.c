/* opengl.c -- OpenGL and shader generator hooks and patches
 *
 * Copyright (C) 2021 fgsfds, Andy Nguyen
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "../config.h"
#include "../util.h"
#include "../so_util.h"

// Global EGL objects - accessible from imports.c for EGL wrapper functions
EGLDisplay g_egl_display = EGL_NO_DISPLAY;
EGLSurface g_egl_surface = EGL_NO_SURFACE;
EGLContext g_egl_context = EGL_NO_CONTEXT;

// Internal aliases for convenience
#define display g_egl_display
#define surface g_egl_surface
#define context g_egl_context

void NVEventEGLSwapBuffers(void)
{
  eglSwapBuffers(display, surface);
}

void NVEventEGLMakeCurrent(void)
{
}

void NVEventEGLUnmakeCurrent(void)
{
}

int NVEventEGLInit(void)
{
  EGLint numConfigs = 0;
  EGLConfig eglConfig;

  const EGLint contextAttribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE};

  const EGLint configAttribs[] = {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 24,
      EGL_STENCIL_SIZE, 8,
      EGL_NONE};

  NWindow *win = nwindowGetDefault();

  // Mesa 26 for Switch can run GL on nvc0 (native) or zink (over NVK).
  if (config.gl_driver[0] && strcmp(config.gl_driver, "auto") != 0)
    setenv("MESA_SWITCH_GL_DRIVER", config.gl_driver, 1);
  // glthread moves driver work to its own thread. egl-switch enables it by
  // default, but the engine hands the context between threads, so it stays an
  // opt-in (gl_thread 1) until it has been played through.
  setenv("MESA_GLTHREAD", config.gl_thread ? "true" : "false", 1);
  setenv("MESA_SWITCH_GLTHREAD", config.gl_thread ? "1" : "0", 1);
  debugPrintf("EGL: gl_thread=%d\n", config.gl_thread);
  // keep more freed GPU buffers for reuse (default 128 MB) so area transitions
  // recycle instead of reallocating; the heap has ~3 GB to spare
  setenv("NOUVEAU_HORIZON_BO_CACHE_MB", "256", 0);
  // Mesa keeps compiled shaders on disk when it has somewhere to put them; the
  // Switch has no HOME, so point it next to the .nro. Later runs then skip the
  // shader compiles that cause hitches the first time a scene is seen.
  if (config.shader_cache)
  {
    char cache_dir[PATH_MAX];
    if (getcwd(cache_dir, sizeof(cache_dir) - 16))
    {
      strcat(cache_dir, "/shader_cache");
      mkdir(cache_dir, 0777);
      setenv("MESA_SHADER_CACHE_DIR", cache_dir, 1);
      debugPrintf("EGL: shader cache at %s\n", cache_dir);
    }
  }
  else
  {
    setenv("MESA_SHADER_CACHE_DISABLE", "true", 1);
  }
  debugPrintf("EGL: gl_driver=%s\n", config.gl_driver[0] ? config.gl_driver : "(auto)");

  nwindowSetDimensions(win, screen_width, screen_height);

  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (!display)
  {
    debugPrintf("EGL: Could not connect to display: %08x\n", eglGetError());
    return 0;
  }

  eglInitialize(display, NULL, NULL);

  if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE)
  {
    debugPrintf("EGL: Could not set API: %08x\n", eglGetError());
    return 0;
  }

  eglChooseConfig(display, configAttribs, &eglConfig, 1, &numConfigs);
  if (numConfigs <= 0)
  {
    debugPrintf("EGL: No matching config: %08x\n", eglGetError());
    return 0;
  }

  surface = eglCreateWindowSurface(display, eglConfig, win, NULL);
  if (!surface)
  {
    debugPrintf("EGL: Could not create surface: %08x\n", eglGetError());
    return 0;
  }

  context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, contextAttribs);
  if (!context)
  {
    debugPrintf("EGL: Could not create context: %08x\n", eglGetError());
    return 0;
  }

  eglMakeCurrent(display, surface, surface, context);
  if (eglSwapInterval(display, 1) == EGL_FALSE)
    debugPrintf("EGL: Could not set swap interval: %08x\n", eglGetError());

  debugPrintf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
  debugPrintf("GL_VERSION: %s\n", glGetString(GL_VERSION));
  debugPrintf("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));

  return 1; // success
}

void patch_opengl(void)
{
  // --- NVEventEGL* symbols DO NOT EXIST in 64-bit libGame.so ---
  // hook_arm64(so_find_addr("_Z14NVEventEGLInitv"), (uintptr_t)NVEventEGLInit);
  // hook_arm64(so_find_addr("_Z21NVEventEGLMakeCurrentv"), (uintptr_t)NVEventEGLMakeCurrent);
  // hook_arm64(so_find_addr("_Z23NVEventEGLUnmakeCurrentv"), (uintptr_t)NVEventEGLUnmakeCurrent);
  // hook_arm64(so_find_addr("_Z21NVEventEGLSwapBuffersv"), (uintptr_t)NVEventEGLSwapBuffers);
}

void deinit_opengl(void)
{
  if (display)
  {
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context)
      eglDestroyContext(display, context);
    if (surface)
      eglDestroySurface(display, surface);
    eglTerminate(display);
  }
}
