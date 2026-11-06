# Builds, CI, and releases

[Русский](releases.md) · [Task index](README.en.md)

- Check commands and pinned dependencies against the [build guide](../docs/build.en.md) and [release workflow](../.github/workflows/engine-release.yml). An engine tag builds optimized Windows x64, Linux x64, macOS ARM64, and Android ARM64 packages. Run the workflow manually to verify a build without publishing.
- Test the affected platforms. Linux CI uses software Vulkan. macOS window tests run sequentially. For Android, check gestures, Vulkan, a visible frame after resuming, and the contents of the signed APK. Suspending writes a separate autosave. Resuming a live process continues the current session.
- Engine archives contain binaries, dependencies, fonts, and guides. Original game data, saves, and HD are separate. Update [network-version.txt](../network-version.txt) when compatibility changes. Verify archive contents, checksums, and CI before announcing a release.
- Normal desktop builds reuse the prebuilt HD cache. Set `-DS2_BUILD_HD_TEXTURES=OFF` for builds and CI without HD. Follow the [HD guide](../docs/hd-authoring.en.md) to prepare the cache, restore sources, or rebuild the pack explicitly. During authoring, preserve exact prompts, UVs, alpha, materials, and texture scale. Reuse accepted textures and update shared metadata sequentially.
- Publish HD separately from the engine with `--latest=false`. The [installer](../packaging/prepare_hd.py) selects the newest stable `hd-*` release and verifies the volumes and extracted files against its index. See the [release guide](../docs/releasing.en.md) for the index format and publication procedure.
