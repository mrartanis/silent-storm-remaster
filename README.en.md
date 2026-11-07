# Silent Storm Remaster

[Русский](README.md) · English

A modern engine for **Silent Storm 1.2**, based on [Silent Storm Reconstruction](https://github.com/met-nikita/Silent-Storm-Reconstruction) and [Nival's original source code](https://github.com/nival/Silent-Storm).

## Installation and launch

1. You'll need a copy of **Silent Storm Gold Edition** purchased on Steam or GOG, with the base game's **1.2** data.
2. Download the launcher for your platform, extract the archive, and open the application. [Windows x64](https://github.com/mrartanis/silent-storm-remaster/releases/latest/download/silent-storm-launcher-windows-x64.zip) · [Linux x64](https://github.com/mrartanis/silent-storm-remaster/releases/latest/download/silent-storm-launcher-linux-x64.tar.gz) · [macOS ARM64](https://github.com/mrartanis/silent-storm-remaster/releases/latest/download/silent-storm-launcher-macos-arm64.zip)
3. Select the base Silent Storm folder, or click **Import GOG installer…** and select the full offline `setup_….exe`, keeping all accompanying `.bin` files beside it. The extractor is included in the launcher; no separate installation is needed.
4. Download the engine and the optional HD pack directly in the launcher. It handles installation automatically.
5. Click **Play**. You're magnificent.

## Features

- A 64-bit engine for Windows, Linux, macOS, and Android that reads the original game data.
- Widescreen support, scalable UI, vector fonts, and a modern renderer.
- HD textures on desktop and touch controls on Android.
- A [desktop launcher](docs/launcher.en.md) for installing and updating the engine and HD pack, with game data import from offline GOG installers.
- Two-player tactical matches with an option to [play against an LLM agent](docs/agent-play.en.md).
- Native playback of the original audio and cutscenes, plus a new implementation of facial animation.

| Original textures | HD textures |
| :---: | :---: |
| <a href="assets/screenshots/courtyard-original.webp"><img src="assets/screenshots/courtyard-original.webp" width="400" alt="Courtyard with original textures"></a> | <a href="assets/screenshots/courtyard-hd.webp"><img src="assets/screenshots/courtyard-hd.webp" width="400" alt="Courtyard with HD textures"></a> |

## Releases

- [Latest engine release](https://github.com/mrartanis/silent-storm-remaster/releases/latest) · Windows x64, Linux x64, macOS ARM64, Android ARM64.
- [All releases](https://github.com/mrartanis/silent-storm-remaster/releases) · The engine releases include the launcher. The HD pack is released separately.

## Documentation

[Desktop installation and HD textures](docs/install.en.md) · [Android](docs/android.en.md) · [Multiplayer](docs/multiplayer.en.md) · [Build from source](docs/build.en.md) · [Publish releases](docs/releasing.en.md) · [License](LICENSE.md)
