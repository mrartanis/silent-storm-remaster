# Silent Storm Remaster desktop launcher

[Русский](launcher.md) · [Manual installation](install.en.md)

The launcher downloads the engine from stable GitHub releases for Windows 10/11
x64, Linux x64 (Ubuntu 24.04 or compatible), and macOS 15+ on Apple silicon.
The Linux interface supports native Wayland and X11.
Under **Game data**, select the folder of your Steam or GOG copy of Silent Storm
1.2. It must contain `game.db`, `start.cfg`, `tree.mma`, `cfg`, `res` and
`scripts`. For Gold Edition, select the main Silent Storm folder.
On Windows, **Find game** looks for Steam and GOG installations while no game
folder has been selected.

1. Select **Download** next to Engine, then **Play**.
2. For HD textures, install [7-Zip](https://www.7-zip.org/) and select **Download**
   next to HD textures. The launcher accepts `7zz`, `7z` or `7za` on `PATH`
   and finds the standard Windows 7-Zip installation or `brew install sevenzip`
   on macOS. The current pack needs
   about 12 GB of free space. Toggle HD in the game's graphics settings.
3. When a new version is available, select **Update**. **Repair** reinstalls the
   current engine and preserves the HD pack. **Verify** checks the HD files and
   reinstalls a damaged pack. Installed games can be played offline.

**Saves** opens the game's usual save directory. **Game log** opens the output
from the latest launch. **Installation folder** opens the engine directory,
with the installed HD pack under `res-hd`. Before engine installation it
opens the launcher data folder. By default the launcher checks for updates
on startup and the game opens in borderless fullscreen.
On Linux, follow the [engine installation guide](install.en.md)
for system libraries and Vulkan. On Ubuntu 24.04, install the libraries
for your display session to open the launcher.

```sh
# Wayland
sudo apt install libegl1 libwayland-client0 libwayland-cursor0 libwayland-egl1 libxkbcommon0
# X11
sudo apt install libegl1 libgl1 libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-xinerama0 libxcb-xinput0 libxkbcommon-x11-0
```

## Run from the repository

Use Python 3.11+ for your platform and architecture. From the repository root,
run these commands in a virtual environment.

```sh
python -m pip install -r launcher/requirements.txt
python launcher/launcher.py
```

The engine, HD pack and launcher preferences live in the application data folder.
Pass `--storage "/path/to/launcher"` to select a different drive. The interface
follows the system language, with `--language en` and `--language ru` overrides.

## Build a distributable

Run these commands from the repository root on each target OS.

```sh
python -m pip install -r launcher/requirements-build.txt
python -m PyInstaller launcher/launcher.spec
```

Distribute the complete `dist/SilentStormLauncher` folder. On macOS, use
`dist/SilentStormLauncher.app`. Preserve symbolic links in Linux and macOS
archives.

## Verify and publish on GitHub

1. Run [Desktop launcher](../.github/workflows/launcher-release.yml) manually once
   it is on the default branch. To verify a branch before merging, use a
   `launcher-check/…` branch name. The workflow packages all three platforms,
   opens the launcher from each extracted archive, checks the window,
   reads release metadata and installs a tiny local archive with SHA-256
   verification. Linux is checked separately under X11 and native Wayland.
2. For a release, tag the verified commit `launcher-vX.Y.Z` and push the tag.
   After all checks pass, the workflow creates a draft with three archives
   and `SHA256SUMS`.
3. Review the draft and publish it with **Set as the latest release** cleared.
   The CI macOS build uses an ad hoc signature. Prepare Developer ID signing
   and Apple notarization for Gatekeeper distribution.
