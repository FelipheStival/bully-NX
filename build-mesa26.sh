#!/bin/sh
# Builds bully_nx.nro against Mesa 26 (ghcr.io/autorunhq/switch-dev).
# The devkitpro/devkita64 image only ships Mesa 20.1, which does not boot this port.
set -e

cd "$(dirname "$0")"
WORK="$(mktemp -d)"
mkdir -p "$WORK/bully_nx"
cp -R Makefile icon.jpg source "$WORK/bully_nx/"

MESA_LIBS='-Wl,--start-group -lEGL -lGLESv2 -lglapi -lvulkan -lnvk -lnak -lnak_rs -lnil -lnir -lvtn -lcompiler -lcompiler_c_helpers -lvulkan_util -lnouveau_horizon -lnouveau_ws -lnouveau_mme -lnvidia_headers_c -lmesa_util_c11 -lblake3 -lmesa_util -lmesa_util_simd -lxmlconfig -lexpat -lxxhash -lzstd -lz -lstdc++ -lnx -lm -Wl,--end-group'

docker run --rm -v "$WORK:/project" --workdir /project/bully_nx \
  ghcr.io/autorunhq/switch-dev:latest \
  sh -lc "make -j\$(nproc) MESA_LIBS='$MESA_LIBS'"

cp "$WORK/bully_nx/bully_nx.nro" ./bully_nx-mesa26.nro
cp "$WORK/bully_nx/bully_nx.elf" ./bully_nx-mesa26.elf
rm -rf "$WORK"
echo "built ./bully_nx-mesa26.nro"
