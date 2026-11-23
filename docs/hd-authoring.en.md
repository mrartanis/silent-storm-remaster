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

The seven PNG ZIP archives are stored in the separate [assets-hd-sources-v0.1 release](https://github.com/mrartanis/silent-storm-remaster/releases/tag/assets-hd-sources-v0.1), not Git LFS. They total 12.54 GiB and are needed only for texture authoring, not playing or ordinary engine builds. The manifest in `data/hd-sources` maps PNGs to their original paths and records their checksums. `release.json` pins the release and manifest SHA-256. Exact prompts and metadata are stored in [assets/terrain-hd](../assets/terrain-hd).

```sh
python packaging/restore_hd_sources.py --download --verify-only
python packaging/restore_hd_sources.py
```

The script downloads missing archives only with `--download`, caches them in `data/hd-sources`, and checks sizes and the SHA-256 hashes of every ZIP and PNG. Without `--download` it never uses the network. You can download the ZIPs manually or use a backup: `python packaging/restore_hd_sources.py --archive-dir /path/to/archives --verify-only`. Corrupt ZIPs are not replaced automatically; move them aside before downloading again.

Git ignores downloaded ZIPs and restored PNGs. Modified PNGs are never overwritten. To restore them to a separate directory, pass `--root /absolute/path`. Follow the [HD release guide](releasing.en.md#hd-pack) to update the prebuilt pack.
