# Install on desktop

[Русский](install.md)

## Install the engine

1. Open the [latest release](https://github.com/mrartanis/silent-storm-remaster/releases/latest) and download the archive for your platform.
2. Find your installed copy of **Silent Storm 1.2** from Steam or GOG. In Gold Edition, use the base Silent Storm game. In Steam, open **Properties → Installed Files → Browse** to find the game folder.
3. Create a separate folder for the remaster. Copy `game.db`, `start.cfg`, `tree.mma`, and the complete `cfg`, `res`, and `scripts` folders into it.
4. Extract the contents of the folder in the engine archive alongside `game.db`. Keep all supplied libraries, fonts, and `network-version.txt` with the engine.

| Platform | Archive | Launch |
| --- | --- | --- |
| Windows 10/11 x64 | `*-windows-x64.zip` | Open `Play.cmd`. |
| Linux x64, Ubuntu 24.04 or a compatible system with glibc 2.39+ | `*-linux-x64.tar.gz` | Run `./Play.sh` from the game folder. |
| macOS 15+ on Apple silicon | `*-macos-arm64.tar.gz` | Open `Play.command` in Finder. |

On Linux, install the Vulkan driver for your GPU. On Ubuntu 24.04, install these system libraries as well.

```sh
sudo apt install libgsf-1-114 libpng16-16t64 zlib1g libx11-6 libxi6 libgl1 libvulkan1
```

## Add HD textures

Close the game. Install Python 3.11+ and [7-Zip](https://www.7-zip.org/). Downloading and extracting the current pack takes about 12 GB of free space. Open a terminal in the game folder. If your engine archive does not include `prepare_hd.py`, save the [installer script](https://raw.githubusercontent.com/mrartanis/silent-storm-remaster/HEAD/packaging/prepare_hd.py) there.

```powershell
py -3 prepare_hd.py --game-dir .
```

On Linux and macOS, use `python3 prepare_hd.py --game-dir .`. The script finds the latest stable HD release, downloads every volume, verifies the checksums, and installs `res-hd`. Use **HD Textures** in the game's graphics settings to switch texture sets.

## Updates and saves

Close the game and extract the new engine archive over the installed files, including `network-version.txt`. Run the installer again to update the HD pack.

| Platform | Saves and settings |
| --- | --- |
| Windows | `%LOCALAPPDATA%\Silent Storm Reconstruction` |
| Linux | `$XDG_DATA_HOME/silent-storm-reconstruction` or `~/.local/share/silent-storm-reconstruction` |
| macOS | `~/Library/Application Support/Silent Storm Reconstruction` |

Set the `S2_USER_DATA_DIR` environment variable to use a different folder for saves and settings.

Use the graphics settings to change resolution, window mode, anti-aliasing, and VSync. Open **Display and Interface** to change UI scaling and vector font settings.
