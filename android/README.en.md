# Silent Storm Remaster for Android

[Русский](README.md)

## Install

1. Download `silent-storm-engine-<version>-android-arm64.apk` from the [engine releases](https://github.com/mrartanis/silent-storm-remaster/releases). Use an ARM64 device with Android 9 or later and Vulkan. The port was tested on the Redmi Note 12 4G.
2. Install the APK. Android may ask you to allow app installation from the app that opened the file. With the phone connected, you can also run `adb install -r silent-storm-engine-<version>-android-arm64.apk`, using the filename you downloaded.
3. Find your installed copy of **Silent Storm 1.2** from [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) or [GOG](https://www.gog.com/en/game/silent_storm_gold). In Gold Edition, choose the base Silent Storm game. Look for the folder containing `game.db`, `start.cfg`, `tree.mma`, and the `cfg`, `res`, and `scripts` directories.

The APK contains the engine and fonts. Android uses the original textures.

## Copy the game data

Copy the listed files and complete directories to the phone's internal shared storage. Place `game.db` directly inside `files`.

```text
Android/data/org.silentstorm.remaster/files/
  game.db
  start.cfg
  tree.mma
  cfg/
  res/
  scripts/
```

Android may restrict file managers' access to `Android/data`. To copy from a computer, install [Android Platform Tools](https://developer.android.com/tools/releases/platform-tools), enable USB debugging on the phone, and authorize the computer. Open a terminal in the original game folder and run the following commands.

```sh
adb devices
adb shell mkdir -p /sdcard/Android/data/org.silentstorm.remaster/files
adb push game.db start.cfg tree.mma cfg res scripts /sdcard/Android/data/org.silentstorm.remaster/files/
```

After copying, open Silent Storm on the phone. For multiplayer, also copy [network-version.txt](../network-version.txt) from the matching engine version to `files`.

Release APKs use the same signing key. Install a new release over the old one to keep your data. An earlier debug APK may have a different signature. Android requires you to uninstall such a build before installing a release. Back up any saves you want to keep before uninstalling the debug build.

## Controls and saves

- Tap to select a target or press a button. Hold on the map to open the context menu while your finger is still down.
- Drag the map to pan the camera. Pinch or twist with two fingers to zoom or rotate. Slide two fingers vertically to tilt.
- The **ALT** button toggles ground item highlighting.
- Minimizing the game or turning off the screen updates the separate **Background autosave** slot. If Android closes the game process, open that slot from **Load Game**. Otherwise, reopening the app resumes the current session.

## Build and test

The [release workflow](../.github/workflows/engine-release.yml) builds the ARM64 APK on Ubuntu with JDK 17, SDK 36, NDK r28c, and CMake 3.31.6. The [dependency fetch script](../.github/scripts/prepare-deps.py) and the [Android build script](../.github/scripts/build-android-deps.sh) pin and build its dependencies.

GitHub Actions runs gesture tests and checks the contents of the APK. An x86-64 Android emulator runs a separate test APK without retail game data. It checks Vulkan presentation, taps, a long press before finger release, and a visible frame after returning from Home in the same process. Missions, autosaves with real data, and performance are tested on physical devices.
