#!/usr/bin/env bash
# Meson configure for xenpipe, a from-scratch software gallium driver.
# Run from repo root. Re-run with `--reconfigure` if build/ already exists.

meson setup --reconfigure build \
  -Dgallium-drivers=softpipe,xenpipe \
  -Dvulkan-drivers=[] \
  -Dglx=dri \
  -Dplatforms=x11,wayland \
  -Dbuildtype=debug \
  -Dllvm=disabled \
  -Degl=enabled \
  -Dglvnd=enabled \
  -Dprefix=/home/chvsnaveen12/gfxexp/mesa/install
