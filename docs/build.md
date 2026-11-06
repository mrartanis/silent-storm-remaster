# Сборка из исходников

[English](build.en.md) · [Главная](../README.md)

Установите CMake 3.21+ и Python 3.11+. Выполняйте команды из корня репозитория и подставьте свой путь к зависимостям. Для запуска подготовьте отдельную копию игровых данных по [инструкции по установке](install.md#установите-движок).

## Windows x64

Установите Visual Studio 2022 с компонентом **Разработка классических приложений на C++**, MSVC v143 и Windows SDK. Подготовьте SDL3, bgfx, bimg, bx, FFmpeg и miniaudio по шагам Windows в [релизном workflow](../.github/workflows/engine-release.yml).

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

Движок и библиотеки появятся в `build/Release/`. Для сборки с отладочной информацией замените `Release` на `RelWithDebInfo`.

## Linux x64

Установите Clang 18, Ninja, pkg-config и пакеты разработки libgsf, libpng, zlib, X11, XInput2 и OpenGL. Следующие команды подготовят зависимости и соберут `build-linux/Game`.

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

Для кросс-сборки подготовьте Clang 18, ARM64 GCC, ARM64 sysroot с библиотеками и `shaderc` для компьютера сборки. Используйте отдельный каталог сборки и [LinuxArm64Clang.cmake](../cmake/Toolchains/LinuxArm64Clang.cmake). Пошаговый пример подготовки sysroot и настройки CMake есть в [Build-Stage6-Linux.sh](../diagnostics/Build-Stage6-Linux.sh).

## macOS на Apple silicon

Установите инструменты Xcode и зависимости через Homebrew. Скрипт подготовит зависимости в соседней папке `ss-deps-mac` и соберёт `build-mac/Game`.

```sh
xcode-select --install
brew install cmake ninja pkg-config libgsf libpng
./build-mac.sh
```

## Android ARM64

Используйте [Android-задачу релизного workflow](../.github/workflows/engine-release.yml). Она задаёт версии JDK, SDK, NDK и CMake и запускает сборку APK.

## Запуск и HD

Запускайте собранный `Game` или `Game.exe` с папкой оригинальных данных в качестве рабочей. Движку нужны библиотеки и шрифты из сборки. На Linux добавьте каталог общих библиотек SDL3 и FFmpeg в `LD_LIBRARY_PATH`.

Чтобы CMake добавлял готовый HD-пак в настольную сборку, подготовьте [локальный кеш HD](hd-authoring.md) и задайте `-DS2_BUILD_HD_TEXTURES=ON`.
