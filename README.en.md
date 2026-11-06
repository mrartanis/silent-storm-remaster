# Silent Storm Remaster

[Русский](README.md) · English

A modern engine for **Silent Storm 1.2**, based on [Silent Storm Reconstruction](https://github.com/met-nikita/Silent-Storm-Reconstruction) and [Nival's original source code](https://github.com/nival/Silent-Storm).

## Installation and launch

1. You'll need a copy of **Silent Storm Gold Edition** purchased on Steam or GOG, with the base game's **1.2** data.
2. [Download the launcher](https://github.com/mrartanis/silent-storm-remaster/actions/workflows/launcher-release.yml?query=is%3Asuccess). Sign in to GitHub and pick the `launcher-package-…` for your platform under **Artifacts** in the latest successful build. Extract the downloaded ZIP and the archive inside it, then open the launcher.
3. Select the base Silent Storm folder. Download the engine and the optional HD pack. You'll need 7-Zip to extract the HD pack.
4. Click **Play**. You're magnificent.

## Features

- A 64-bit engine for Windows, Linux, macOS, and Android that reads the original game data.
- Widescreen support, scalable UI, vector fonts, and a modern renderer.
- HD textures on desktop and touch controls on Android.
- A [desktop launcher](docs/launcher.en.md) for installing and updating the engine and HD pack.
- Two-player tactical matches with an option to [play against an LLM agent](docs/agent-play.en.md).
- Native playback of the original audio and cutscenes, plus a new implementation of facial animation.

| Original textures | HD textures |
| :---: | :---: |
| <a href="assets/screenshots/courtyard-original.webp"><img src="assets/screenshots/courtyard-original.webp" width="400" alt="Courtyard with original textures"></a> | <a href="assets/screenshots/courtyard-hd.webp"><img src="assets/screenshots/courtyard-hd.webp" width="400" alt="Courtyard with HD textures"></a> |

## Releases

- [Latest engine release](https://github.com/mrartanis/silent-storm-remaster/releases/latest) · Windows x64, Linux x64, macOS ARM64, Android ARM64.
- [All releases](https://github.com/mrartanis/silent-storm-remaster/releases) · The HD pack and desktop launcher are released separately.

## Documentation

[Desktop installation and HD textures](docs/install.en.md) · [Android](docs/android.en.md) · [Multiplayer](docs/multiplayer.en.md) · [Build from source](docs/build.en.md) · [Publish releases](docs/releasing.en.md) · [License](LICENSE.md)
