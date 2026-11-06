# Install on Android

[Русский](android.md) · [Desktop installation](install.en.md) · [Home](../README.en.md)

Use an ARM64 phone or tablet with Android 9 or later and Vulkan support. The game uses the original textures.

1. Download the file ending in `android-arm64.apk` from the [latest release](https://github.com/mrartanis/silent-storm-remaster/releases/latest) and install it.
2. On your computer, find your installed copy of **Silent Storm 1.2**. In Gold Edition, use the base Silent Storm game.
3. Install [Android Platform Tools](https://developer.android.com/tools/releases/platform-tools), turn on USB debugging, connect the device, and authorize the computer.
4. Open a terminal in the original game folder and copy the game data.

   ```sh
   adb shell mkdir -p /sdcard/Android/data/org.silentstorm.remaster/files
   adb push game.db start.cfg tree.mma cfg res scripts /sdcard/Android/data/org.silentstorm.remaster/files/
   ```

5. Open **Silent Storm** on the device. For [multiplayer](multiplayer.en.md), copy `network-version.txt` from the desktop archive of the same release into the same folder.

## Controls

- Tap a target to select it. Press and hold on the map to open the context menu.
- During a video, cutscene, or dialogue, hold one finger down for two seconds to skip. The ring under your finger shows how long to keep holding.
- Drag with one finger to pan the camera. Pinch with two fingers to zoom, twist to rotate, or move both fingers up or down to tilt.
- Tap **ALT** to toggle ground item highlighting.
- Minimizing the game or turning off the screen updates the **Background autosave** slot.

[Android component licenses](../android/THIRD_PARTY.md)
