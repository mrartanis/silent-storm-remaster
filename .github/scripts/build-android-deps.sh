#!/usr/bin/env bash
set -euo pipefail

deps=$(realpath "$1")
ndk="$ANDROID_HOME/ndk/28.2.13676358"
tools="$ndk/toolchains/llvm/prebuilt/linux-x86_64/bin"
jobs=${S2_BUILD_JOBS:-4}
prefix="$deps/android-arm64"
vcpkg_revision=29718cfec805208b4e2d80ed91e681143b488dd7

fetch() {
  local name=$1 url=$2 digest=$3
  if [[ ! -f "$deps/$name" ]]; then
    curl --fail --location --retry 3 "$url" -o "$deps/$name.part"
    mv "$deps/$name.part" "$deps/$name"
  fi
  printf '%s  %s\n' "$digest" "$deps/$name" | sha256sum --check
}

fetch "vcpkg-$vcpkg_revision.tar.gz" \
  "https://codeload.github.com/microsoft/vcpkg/tar.gz/$vcpkg_revision" \
  6ef2a33ff23c6a10d46365c58cefe0fc4d39025f6f771d22fcef19e9b5c98a14
fetch libgsf-1.14.60.tar.xz https://download.gnome.org/sources/libgsf/1.14/libgsf-1.14.60.tar.xz \
  83e12c36a099a8a7019bf425e2c24af66eeb2ef5874251e694431a07d9805dae
fetch libiconv-1.19.tar.gz https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.19.tar.gz \
  88dd96a8c0464eca144fc791ae60cd31cd8ee78321e67397e25fc095c4a19aa6
fetch freetype-2.14.3.tar.xz https://download.savannah.gnu.org/releases/freetype/freetype-2.14.3.tar.xz \
  36bc4f1cc413335368ee656c42afca65c5a3987e8768cc28cf11ba775e785a5f

for archive in "vcpkg-$vcpkg_revision.tar.gz" libgsf-1.14.60.tar.xz libiconv-1.19.tar.gz freetype-2.14.3.tar.xz; do
  name=${archive%.tar.*}
  [[ -d "$deps/$name" ]] || tar -xf "$deps/$archive" -C "$deps"
done

vcpkg="$deps/vcpkg-$vcpkg_revision"
export ANDROID_NDK_HOME="$ndk" ANDROID_NDK_ROOT="$ndk" VCPKG_MAX_CONCURRENCY="$jobs"
"$vcpkg/bootstrap-vcpkg.sh" -disableMetrics
"$vcpkg/vcpkg" install --triplet arm64-android --host-triplet x64-linux \
  glib libpng 'libxml2[core,iconv,zlib]'
mkdir -p "$prefix/bin"
cp -a "$vcpkg/installed/arm64-android/include" "$vcpkg/installed/arm64-android/lib" "$prefix/"
cat > "$prefix/bin/pkg-config-android" <<'WRAPPER'
#!/bin/sh
prefix=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
export PKG_CONFIG_LIBDIR="$prefix/lib/pkgconfig"
unset PKG_CONFIG_PATH
exec pkg-config --static "$@"
WRAPPER
chmod +x "$prefix/bin/pkg-config-android"

export CC="$tools/aarch64-linux-android28-clang" CXX="$tools/aarch64-linux-android28-clang++"
export AR="$tools/llvm-ar" NM="$tools/llvm-nm" RANLIB="$tools/llvm-ranlib" STRIP="$tools/llvm-strip"
export CFLAGS='-O2 -fPIC' PKG_CONFIG="$prefix/bin/pkg-config-android"
for name in libiconv-1.19 libgsf-1.14.60; do
  mkdir -p "$deps/build-$name"
  options=(--disable-nls)
  if [[ $name == libgsf-* ]]; then
    options+=(--disable-introspection --disable-gtk-doc)
  fi
  (
    cd "$deps/build-$name"
    "$deps/$name/configure" --host=aarch64-linux-android --prefix="$prefix" \
      --disable-shared --enable-static "${options[@]}"
    make -j "$jobs"
    make install
  )
done

mkdir -p "$deps/build-ffmpeg-android"
(
  cd "$deps/build-ffmpeg-android"
  "$deps/ffmpeg-8.1/configure" --prefix="$prefix" --target-os=android --arch=aarch64 \
    --enable-cross-compile --cc="$CC" --cxx="$CXX" --ar="$AR" --nm="$NM" --ranlib="$RANLIB" --strip="$STRIP" \
    --enable-pic --disable-shared --enable-static --disable-autodetect --disable-programs \
    --disable-doc --disable-network --disable-encoders --disable-muxers --disable-avdevice --disable-avfilter
  make -j "$jobs"
  make install
)

# shaderc runs on the Linux builder, separate from the ARM64 target libraries.
unset CC CXX AR NM RANLIB STRIP CFLAGS PKG_CONFIG
cmake -S "$deps/bgfx.cmake" -B "$deps/build-shaderc" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DBGFX_DIR="$deps/bgfx" -DBIMG_DIR="$deps/bimg" -DBX_DIR="$deps/bx" \
  -DBGFX_WITH_WAYLAND=OFF -DBGFX_BUILD_EXAMPLES=OFF -DBGFX_BUILD_TESTS=OFF -DBGFX_INSTALL=OFF \
  -DBGFX_BUILD_TOOLS_BIN2C=OFF -DBGFX_BUILD_TOOLS_GEOMETRY=OFF -DBGFX_BUILD_TOOLS_TEXTURE=OFF
cmake --build "$deps/build-shaderc" --target shaderc --parallel "$jobs"
install -m 755 "$(find "$deps/build-shaderc" -type f -name shaderc -print -quit)" "$deps/shaderc"

notices="$deps/android-notices"
mkdir -p "$notices"
for entry in SDL3-3.4.16/LICENSE.txt bgfx/LICENSE bimg/LICENSE bx/LICENSE \
             freetype-2.14.3/docs/FTL.TXT ffmpeg-8.1/COPYING.LGPLv2.1 \
             libgsf-1.14.60/COPYING libiconv-1.19/COPYING.LIB; do
  install -D -m 644 "$deps/$entry" "$notices/$entry"
done
for license in "$vcpkg"/installed/arm64-android/share/*/copyright; do
  install -D -m 644 "$license" "$notices/vcpkg/$(basename "$(dirname "$license")")/copyright"
done
