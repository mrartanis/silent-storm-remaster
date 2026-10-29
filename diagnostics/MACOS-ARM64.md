# Native macOS ARM64 bring-up

Verified on 2026-10-25 on Apple silicon, macOS 26.6.2 (25G83), Apple clang
21.0.0 / command line tools SDK 26.5. `Game` is a Mach-O ARM64 executable;
bgfx reports `Metal, GPU 106b:03f1`. This is not a Rosetta or Wine run.

## Build and repeat

```sh
brew install cmake ninja pkg-config libgsf libpng
./build-mac.sh
ctest --test-dir build-mac --output-on-failure --timeout 300 --parallel 4
```

Initial bring-up result: **132/132 tests passed**, including all five renderer/input/
display/focus/graphics tests, in 91.51 seconds. Network session and package
path tests additionally passed five consecutive runs each. The existing
`diagnostics/EnginePackageTests.py` suite passed all three tests.

The wrapper downloads the existing pinned SDL3 3.4.16, FFmpeg 8.1,
bgfx/bimg/bx and miniaudio sources into `../ss-deps-mac` (or `S2_DEPS`).
All engine builds, copied include-normalized sources, generated shaders and
tests reuse the ignored `build-mac/` directory. FreeType stays under that
build's FetchContent directory. HD is disabled: no texture generation or
archive restoration is performed.

The executable has local dependency paths in its load commands. This is a
developer build, not a relocatable `.app` bundle or a macOS release archive.

## Runtime smoke test

Original Silent Storm v1.2 data from an external volume was used without
changing the source installation. The single scratch runtime is
`build-mac/runtime/`: copies of `game.db`, `tree.mma`, `start.cfg`, `cfg/` and
`scripts/`, with links to the built fonts and resources. A local resource
cache is `build-mac/res-local/`; external-volume metadata lookups were slow
during map loading. Saves/configuration/screenshots were isolated beneath
`build-mac/runtime/user-data/` with `S2_USER_DATA_DIR`.

From the scratch runtime directory:

```sh
S2_USER_DATA_DIR="$PWD/user-data" ../Game -harness -harness-active -windowed -800
```

Checked through the existing `_harness_cmd.txt` channel:

- Original Bink intro playback and the animated 3D main menu.
- `console mainmenu`, `uiclick tutorial`: DB-backed tutorial world, character,
  terrain, buildings, UI and Russian text rendered through Metal.
- `screenshot`: native Retina readback was 1600x1200 for the 800x600 window.
- `save mac-arm-smoke`, `load mac-arm-smoke`: generated `game.sav` and
  `restart.sav`, restored interfaces, and returned to the tutorial scene.
  Log contains `LOAD-DESERIALIZE-COMPLETE` and `LOAD-SLOT-DONE`.
- `quit`: clean shutdown, process exit confirmed.

After the final incremental build, the same executable was launched again
against the local resources with `-loadslot` targeting `mac-arm-smoke`:
restoration completed, another screenshot was captured, and harness `quit`
returned exit code 0. No test Game process was left running.

Local evidence (intentionally not committed): `build-mac/evidence/game.log`,
`mac-menu.png`, `mac-tutorial.png`, `mac-tutorial-reloaded.png`, build/test
logs, and `build-mac/runtime/_saveload.log`.

## Platform details and limits

- Cocoa supplies the native NSWindow; bgfx uses Metal and shaderc emits MSL.
- ld64 gets `-force_load`/`-dead_strip` rather than GNU archive groups and
  section collection flags. Symbol pins include the Mach-O underscore.
- libc++ no longer provides `std::ptr_fun`; string transforms use lambdas.
  Original legacy source encoding is preserved.
- Case-insensitive APFS tests compare file identity, not spelling, and do not
  attempt to create two files differing only in case. TCP-close tests allow
  asynchronous FIN delivery instead of assuming one poll is sufficient.
- Pixel-exact GUI regressions request `S2_NO_HIGHDPI=1`; the actual game and
  `SDLPlatformRetinaModeTests` keep Retina enabled. Cocoa GUI tests are serial
  and minimize/restore transitions have a longer deadline. Borderless tests
  require the entire display, not rounded usable bounds or a menu-bar strip.
- Default user data uses `~/Library/Application Support/Silent Storm Reconstruction`;
  the existing absolute `S2_USER_DATA_DIR` override is preserved.

This does not claim a completed campaign, physical audio-device/listening
coverage, macOS-to-Windows multiplayer gameplay, x86 macOS, or distribution
packaging. Existing resource/schema/UI diagnostics from the supplied original
data are not silently suppressed by this port.

## Develop rebase, 2026-10-29

Rebased onto `9693f9c8` (`origin/develop`). The existing build directory and
dependency installation were reused. Both CMake conflict resolutions preserve
the new develop tests and the Lua VM's additional shared-log dependency.

Compatibility adjustments for the expanded regression suite:

- Apply the Retina test environment and Cocoa serialization to tests labelled
  `native-display`, preserving their existing environment entries.
- Use a POSIX include spelling in `RPGBullet.cpp` for its portable target.
- Reserve alias-test color pages 4 GiB apart on Darwin, rather than requesting
  a second address above 128 GiB. The two real pointers still share their low
  32 bits, so the regression condition is unchanged; mmap remains hint-only.

Rebase build/test logs are in `build-mac/evidence/rebase/`.

The final native build succeeds. CTest: **287/288 passed** (78.61 seconds),
including the new renderer regressions and focus/restore test. The remaining
failure is `SDLPlatformInputTests`: Cocoa leaves the window at 800x600 after
a successful borderless request instead of setting the fullscreen flag and
desktop dimensions. Its assertions remain enabled; this run is not claimed
as fully green. The packaging Python suite also passes (3/3).

## Borderless fix, 2026-10-29

The remaining Cocoa failure above is resolved. SDL's default native Spaces
path uses an asynchronous `toggleFullScreen:` request; its blocking helper can
report acceptance even when the window never entered the requested Space.
The game now sets the public `SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES` hint to
`0` before video initialization, selecting SDL's synchronous desktop-sized
borderless window instead. This intentionally does not create a separate
animated macOS fullscreen Space. Exclusive display mode selection remains
available. `SetMode` also rejects a mismatched actual Cocoa fullscreen flag.

The former usable-bounds tolerance and 10-second polling workaround were
removed from the input regression. It now immediately requires the fullscreen
flag, a non-exclusive mode and exact desktop dimensions, followed by a correctly
restored window size.

Verification:

- **289/289 CTest passed** (95.30 seconds), including renderer display modes,
  18 minimize/restore cycles and the new `SDLPlatformRetinaModeTests`.
- Input and native Retina mode tests each passed five consecutive runs.
- With HiDPI enabled, the Retina test checks three window sizes and repeated
  borderless/return transitions: logical desktop 1728x1117, backing pixels
  3456x2234, density 2.0, including engine display metrics.
- Actual Game/Metal main menu: `-borderless` produced a 3456x2234 screenshot;
  `setvar gfx_fullscreen = 0` + `gfx_update` restored an 800x600 window with
  1600x1200 backing pixels; mode 2 restored 3456x2234. Captures contain the
  rendered game scene. Harness `quit` returned exit code 0.

Logs, the reusable smoke driver and all three game screenshots are under
`build-mac/evidence/borderless/`; no test game was left running.
