#!/usr/bin/env bash
# ============================================================================
#  Build the Silent Storm engine -- Release (macOS, Apple silicon/x86).
#  Needs CMake, Ninja, pkg-config, libgsf and libpng (brew install cmake ninja
#  pkg-config libgsf libpng). Pinned SDL3/FFmpeg/bgfx/miniaudio dependencies are
#  prepared outside the repository, by default in ../ss-deps-mac.
# ============================================================================
set -euo pipefail
root="$(cd -- "$(dirname -- "$0")" && pwd)"
deps="${S2_DEPS:-$(dirname "$root")/ss-deps-mac}"
build="${root}/build-mac"
jobs="${S2_BUILD_JOBS:-$(sysctl -n hw.ncpu)}"

if [[ ! -f "$deps/install/lib/cmake/SDL3/SDL3Config.cmake" ||
      ! -f "$deps/install/lib/libavformat.dylib" || ! -f "$deps/miniaudio.h" ]]; then
  echo "[INFO] Fetching pinned dependency sources into $deps"
  python3 "$root/.github/scripts/prepare-deps.py" --root "$deps" --platform macos
  echo "[INFO] Building SDL3 and FFmpeg into $deps/install"
  S2_BUILD_JOBS="$jobs" bash "$root/.github/scripts/build-deps.sh" "$deps" macos
fi

cmake -S "$root" -B "$build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DS2_BUILD_GAME=ON \
  -DSDL3_DIR="$deps/install/lib/cmake/SDL3" \
  -DS2_BGFX_CMAKE_ROOT="$deps/bgfx.cmake" \
  -DS2_BGFX_DIR="$deps/bgfx" \
  -DS2_BIMG_DIR="$deps/bimg" \
  -DS2_BX_DIR="$deps/bx" \
  -DS2_BUILD_MEDIA_PROBE=ON -DS2_BUILD_MINIAUDIO_MUSIC_PROBE=ON \
  -DS2_FFMPEG_ROOT="$deps/install" -DS2_MINIAUDIO_INCLUDE_DIR="$deps" \
  -DS2_BUILD_HD_TEXTURES=OFF "$@"
cmake --build "$build" --parallel "$jobs"
echo
echo "[OK] Build finished -- see build-mac/Game"
echo "[HINT] Run Game from a separate copy of the original game data."
