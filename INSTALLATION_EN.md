# Install Silent Storm Remaster on desktop

[Русский](INSTALLATION_RU.md) · [Android guide](android/README.en.md)

## Get the engine and original game data

1. Download the engine archive for your platform from [Releases](https://github.com/mrartanis/silent-storm-remaster/releases). The archive names end in `windows-x64.zip`, `linux-x64.tar.gz`, or `macos-arm64.tar.gz`.
2. Find your installed copy of **Silent Storm 1.2**, such as the base game in [Steam Gold Edition](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/). In Steam, choose **Properties → Installed Files → Browse**. Copy `game.db`, `start.cfg`, `tree.mma`, and the complete `cfg`, `res`, and `scripts` directories into a new game folder.
3. Extract the contents of the engine archive into that folder. Keep the supplied libraries, fonts, and `network-version.txt` with the engine. The launcher and `game.db` should be in the same folder.

## Run the game

- **Windows 10/11 x64.** Start `Play.cmd`. The engine archive includes the MSVC runtime.
- **Linux x86-64.** Start `./Play.sh`. The build targets Ubuntu 24.04 or a compatible system with glibc 2.39 or later. Install your GPU's Vulkan driver and, on Ubuntu, the runtime packages with `sudo apt install libgsf-1-114 libpng16-16t64 zlib1g libx11-6 libxi6 libgl1 libvulkan1`.
- **macOS 15+ on Apple silicon.** Open `Play.command` in Finder. The build uses Metal and includes its runtime libraries. If macOS blocks the first run, open **System Settings → Privacy & Security** and allow the app.

## Optional HD textures

Download all parts of the [HD pack](https://github.com/mrartanis/Silent-Storm-Reconstruction/releases/tag/hd-v0.1) to one folder. Open the first `.7z.001` part in 7-Zip and extract it into the game folder. The result is a `res-hd` directory beside `game.db`. The extracted pack takes about 10 GB. Use **HD Textures** in the game's graphics settings to switch between the original and HD textures.

Check downloads against the release's `SHA256SUMS`. Use `Get-FileHash <archive> -Algorithm SHA256` on Windows or `sha256sum --check SHA256SUMS` on Linux with the checksum file and downloads in the same folder.

## Saves, multiplayer, and updates

Saves and settings live in `%LOCALAPPDATA%\Silent Storm Reconstruction` on Windows, `$XDG_DATA_HOME/silent-storm-reconstruction` (usually `~/.local/share/silent-storm-reconstruction`) on Linux, and `~/Library/Application Support/Silent Storm Reconstruction` on macOS. The `S2_USER_DATA_DIR` environment variable overrides that location.

For multiplayer, keep the matching `network-version.txt` beside `game.db`. Start with the same engine release and game data on each computer. To update, extract the new engine files over the old files, including the new `network-version.txt`. When updating the HD pack, remove the old `res-hd` directory before extracting the replacement.
