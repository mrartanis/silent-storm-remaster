# HD textures for builds and authoring

[Русский](hd-authoring.md) · [Build from source](build.en.md) · [Install for play](install.en.md#add-hd-textures)

## Prepare the prebuilt pack for CMake

Install Git LFS, Python 3.11+, and 7-Zip. Run these commands from the repository root.

```sh
git lfs install
git lfs pull --include="data/hd-runtime/*" --exclude=""
python packaging/prepare_hd.py
```

The script verifies the archives and extracts them to `data/res-hd`. With `S2_BUILD_HD_TEXTURES=ON`, CMake places the pack next to the engine and reuses the cache. On the same drive, the cache and build may use hard links to the same files. Edit the sources and rebuild the pack in a separate directory.

## Restore the source textures for authoring

The archives in `data/hd-sources` contain the PNGs. The manifest maps them to their original paths and records their checksums. Exact prompts and metadata are stored in [assets/terrain-hd](../assets/terrain-hd).

```sh
git lfs pull --include="data/hd-sources/*" --exclude=""
python packaging/restore_hd_sources.py --verify-only
python packaging/restore_hd_sources.py
```

Git ignores restored PNGs. To restore them to a separate directory, pass `--root /absolute/path`. Follow the [HD release guide](releasing.en.md#hd-pack) to update the prebuilt pack.
