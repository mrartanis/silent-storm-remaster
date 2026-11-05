# Компоненты Android / Android components

## Русский

APK содержит FFmpeg 8.1 под LGPL 2.1 или более поздней версией. Исходный архив FFmpeg входит в настольные архивы того же релиза по пути `dependencies/ffmpeg-8.1.tar.xz`. В APK тексты лицензий находятся в `assets/licenses`.

Для сборки с изменёнными библиотеками возьмите исходники движка по тегу релиза. [Скрипт загрузки](../.github/scripts/prepare-deps.py) и [скрипт Android-сборки](../.github/scripts/build-android-deps.sh) указывают точные версии и загружают их исходники. Внесите изменения в исходники библиотек перед их сборкой и повторите шаг сборки APK из [workflow](../.github/workflows/engine-release.yml). Свою сборку можно подписать своим ключом.

## English

The APK contains FFmpeg 8.1 under LGPL 2.1 or later. The desktop archives from the same release include the FFmpeg source at `dependencies/ffmpeg-8.1.tar.xz`. License texts are under `assets/licenses` in the APK.

To rebuild with modified libraries, check out the engine source at the release tag. The [download script](../.github/scripts/prepare-deps.py) and [Android build script](../.github/scripts/build-android-deps.sh) pin and fetch the corresponding library sources. Modify the library sources before compiling them and repeat the APK build step in the [workflow](../.github/workflows/engine-release.yml). You can sign your build with your own key.
