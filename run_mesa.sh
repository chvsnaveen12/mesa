#!/bin/bash
# Run any application using the xenpipe software rasterizer via DRI/EGL.
# Usage: ./run_mesa.sh <command> [args...]
# Example: ./run_mesa.sh glxgears
#          ./run_mesa.sh kitty -1
#          ./run_mesa.sh firefox --no-remote

B=/home/chvsnaveen12/gfxexp/mesa/build
# B=/home/chvsnaveen12/gfxexp/mesa-main/build
DRI=$B/src/gallium/targets/dri
GLX=$B/src/glx
EGL=$B/src/egl
GBM=$B/src/gbm
GBM_BACKEND=$B/src/gbm/backends/dri

# Bypass GLVND: preload our libEGL and libGLX directly.
# The symlink trick makes our libEGL_mesa.so appear as libEGL.so.1
# so apps that link to libEGL.so.1 get ours instead of GLVND.
ln -sf $EGL/libEGL_mesa.so.0.0.0 /tmp/libEGL.so.1 2>/dev/null

exec env \
  LD_PRELOAD="/tmp/libEGL.so.1:$GLX/libGLX_mesa.so.0.0.0" \
  LD_LIBRARY_PATH="$DRI:$GBM:$GBM_BACKEND" \
  LIBGL_DRIVERS_PATH="$DRI" \
  GBM_BACKENDS_PATH="$GBM_BACKEND" \
  GALLIUM_DRIVER=xenpipe \
  LIBGL_ALWAYS_SOFTWARE=true \
  "$@"
