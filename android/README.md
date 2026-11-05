# Android ARM64

Основной стенд — Redmi Note 12 4G. Сборка использует SDK 36, NDK r28c,
CMake 3.31.6, JDK 17 и Gradle 8.12 из поставки SDL3.

## Проверка платформы

Подготовьте зависимости существующим скриптом `.github/scripts/prepare-deps.py`
для Linux. Передайте абсолютный путь к каталогу через `S2_DEPS` и задайте
`ANDROID_HOME` для установленного SDK.

```sh
bash "$S2_DEPS/SDL3-3.4.16/android-project/gradlew" -p android \
  -Ps2Deps="$S2_DEPS" assembleDebug
adb install -r android/build/outputs/apk/debug/SilentStormAndroid-debug.apk
adb shell am start -n org.silentstorm.remaster.probe/org.libsdl.app.SDLActivity
```

Сине-серый экран подтверждает вывод кадра, зелёный показывает жест камеры.
В журнале SDL видны результат инициализации Vulkan, форматы текстур, касания и
удержания. Кнопка «Назад» завершает проверку.

## Сборка игры

Добавьте `-Ps2Game` и `-Ps2Shaderc=/absolute/path/to/shaderc` к команде сборки.
`shaderc` должен быть собран для компьютера, на котором запускается сборка.
В `$S2_DEPS/android-arm64` нужны ARM64-библиотеки FFmpeg, GNU libiconv, libpng, zlib и
libgsf с его зависимостями. `bin/pkg-config-android` в этом каталоге должен
искать `.pc` в каталоге целевых библиотек. Можно передать локальные исходники
FreeType 2.14.3 через `-Ps2Freetype=/absolute/path/to/freetype`.

Для отладочного запуска установите APK и импортируйте оригинальные данные
с компьютера. `S2_GAME_DATA` указывает на каталог оригинальной игры.

```sh
adb shell mkdir -p /data/local/tmp/s2-import
adb push "$S2_GAME_DATA/game.db" "$S2_GAME_DATA/start.cfg" "$S2_GAME_DATA/tree.mma" \
  "$S2_GAME_DATA/cfg" "$S2_GAME_DATA/res" "$S2_GAME_DATA/scripts" /data/local/tmp/s2-import/
adb shell run-as org.silentstorm.remaster mkdir -p files/game-upload
adb shell 'set -o pipefail; cd /data/local/tmp/s2-import &&
  tar -cf - game.db start.cfg tree.mma cfg res scripts |
  run-as org.silentstorm.remaster tar -xf - -C files/game-upload'
adb shell 'run-as org.silentstorm.remaster sh -c "test ! -e files/game && mv files/game-upload files/game"'
```

Для сетевой игры нужен `network-version.txt` из репозитория в `files/game`.
Сохранения и настройки хранятся отдельно, в `files`.

## Управление

- Касание выбирает цель или нажимает кнопку интерфейса.
- Отпускание после удержания на карте открывает контекстное меню.
- Перетаскивание карты двигает камеру. Сведение и разведение двух пальцев
  меняет масштаб, поворот двух пальцев разворачивает камеру.
- Ход завершается существующей кнопкой на панели боя.
