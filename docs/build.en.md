# Build from source

[Русский](build.md) · [Home](../README.en.md)

Install CMake 3.21+ and Python 3.11+. Run the commands from the repository root and substitute your dependency paths. To run the engine, prepare a separate copy of the game data using the [installation guide](install.en.md#install-the-engine).

## Windows x64

Install Visual Studio 2022 with the **Desktop development with C++** workload, MSVC v143, and the Windows SDK. Prepare SDL3, bgfx, bimg, bx, FFmpeg, and miniaudio using the Windows steps in the [release workflow](../.github/workflows/engine-release.yml).

```powershell
$deps = 'C:/s2-deps'
cmake -S . -B build -A x64 `
  "-DSDL3_DIR=$deps/SDL3-3.4.16/cmake" `
  "-DS2_BGFX_CMAKE_ROOT=$deps/bgfx.cmake" `
  "-DS2_BGFX_DIR=$deps/bgfx" "-DS2_BIMG_DIR=$deps/bimg" `
  "-DS2_FFMPEG_ROOT=$deps/install" "-DS2_MINIAUDIO_INCLUDE_DIR=$deps" `
  -DS2_BUILD_MEDIA_PROBE=ON -DS2_BUILD_MINIAUDIO_MUSIC_PROBE=ON `
  -DS2_ENABLE_NATIVE_MUSIC=ON -DS2_ENABLE_NATIVE_SFX=ON -DS2_ENABLE_NATIVE_VIDEO=ON `
  -DS2_BUILD_HD_TEXTURES=OFF
cmake --build build --config Release --parallel
```

Find the engine and libraries in `build/Release/`. Use `RelWithDebInfo` in place of `Release` to build with debug information.

## Linux x64

Install Clang 18, Ninja, pkg-config, and the development packages for libgsf, libpng, zlib, X11, XInput2, and OpenGL. These commands prepare the dependencies and build `build-linux/Game`.

```sh
deps=/opt/s2-deps
python3 .github/scripts/prepare-deps.py --root "$deps" --platform linux
bash .github/scripts/build-deps.sh "$deps" linux

cmake -S . -B build-linux -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang-18 -DCMAKE_CXX_COMPILER=clang++-18 \
  -DS2_BUILD_GAME=ON -DSDL3_DIR="$deps/install/lib/cmake/SDL3" \
  -DS2_BGFX_CMAKE_ROOT="$deps/bgfx.cmake" \
  -DS2_BGFX_DIR="$deps/bgfx" -DS2_BIMG_DIR="$deps/bimg" \
  -DS2_FFMPEG_ROOT="$deps/install" -DS2_MINIAUDIO_INCLUDE_DIR="$deps" \
  -DS2_BUILD_MEDIA_PROBE=ON -DS2_BUILD_MINIAUDIO_MUSIC_PROBE=ON \
  -DBGFX_WITH_WAYLAND=OFF -DS2_BUILD_HD_TEXTURES=OFF
cmake --build build-linux --parallel
```

## Linux ARM64

To cross-compile, prepare Clang 18, ARM64 GCC, an ARM64 sysroot with its libraries, and a host build of `shaderc`. Use a separate build directory and [LinuxArm64Clang.cmake](../cmake/Toolchains/LinuxArm64Clang.cmake). See [Build-Stage6-Linux.sh](../diagnostics/Build-Stage6-Linux.sh) for the sysroot setup and CMake configuration.

## macOS on Apple silicon

Install the Xcode command-line tools and the Homebrew dependencies. The script prepares dependencies in the neighboring `ss-deps-mac` directory and builds `build-mac/Game`.

```sh
xcode-select --install
brew install cmake ninja pkg-config libgsf libpng
./build-mac.sh
```

## Android ARM64

Use the Android job in the [release workflow](../.github/workflows/engine-release.yml). It specifies the JDK, SDK, NDK, and CMake versions and builds the APK.

## Run the engine with game data

Run `Game` or `Game.exe` with the original game data folder as its working directory. Use the runtime libraries and fonts from the build. On Linux, add the directory with the SDL3 and FFmpeg shared libraries to `LD_LIBRARY_PATH`.

To include the prebuilt HD pack in a desktop build, prepare the [local HD cache](hd-authoring.en.md) and configure CMake with `-DS2_BUILD_HD_TEXTURES=ON`.
