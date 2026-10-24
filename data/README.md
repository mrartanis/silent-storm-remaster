# Frozen HD textures

`hd-runtime/` contains the **unchanged hd-v0.1 release archive** and its published
SHA-256 index. The two `.7z` volumes are Git LFS objects. The default LFS fetch
filter downloads only these runtime files; authoring archives remain pointers.

Install Git LFS and 7-Zip (`7zz` or `7z`; the standard Windows installation is
also detected). From the repository root:

```powershell
git lfs install
git lfs pull --include="data/hd-runtime/*" --exclude=""
python packaging/prepare_hd.py
```

This verifies and extracts the release into ignored `data/res-hd/`. Ordinary
Game builds install that ready-to-use pack beside the executable, using hard
links on the same filesystem and copies otherwise. Subsequent builds inspect
a small index and file sizes/timestamps; they do not regenerate textures or
unpack unchanged archives. Changed cache files trigger SHA-256 verification.
Do not edit the generated `.res` files: a build output may share its file with
the local cache through a hard link. The game only reads them.

`S2_BUILD_HD_TEXTURES=OFF` skips HD preparation entirely, as before. Engine
release CI keeps HD disabled and does not need LFS or 7-Zip.
The manual **Verify frozen HD storage** workflow checks the default LFS selection,
extracts and verifies the published runtime, exercises CMake cache reuse, and
tests corruption recovery without fetching authoring PNGs.

## Authoring sources

All formerly tracked PNG paths, including original, raw, calibrated, rejected
and review images, are preserved in `hd-sources/`. Exact duplicate contents
are stored once. Each ZIP is independent, smaller than 2 GB and tracked by LFS.
`manifest.json` maps every original repository path to its archived content
and records SHA-256 hashes for all PNGs and ZIPs. ZIP entries are stored without
recompressing already compressed PNG data. Metadata and authoring tools remain
ordinary Git files.

Fetch and restore sources only when working on the artwork:

```powershell
git lfs pull --include="data/hd-sources/*" --exclude=""
python packaging/restore_hd_sources.py --verify-only
python packaging/restore_hd_sources.py
```

Restored PNGs are ignored by Git. Existing modified PNGs are never overwritten.
For a separate restoration tree, add `--root /absolute/path/to/destination`.
The source archives are a frozen snapshot: changes to restored PNGs do not
automatically update the published archive or the runtime release.

Rebuilding from sources is an explicit authoring operation requiring Pillow:

```powershell
python assets/terrain-hd/build_pack.py --output ../hd-authoring-build/res-hd
python assets/terrain-hd/validate_pack.py --pack ../hd-authoring-build/res-hd --no-update-queue
```

Publish a new validated runtime archive and index to update the frozen pack.
Keep the release's original source revision hashes as provenance, even after
the repository history is rewritten. They refer to the pre-migration history.
That history and the original published releases remain available in
[Silent-Storm-Reconstruction](https://github.com/mrartanis/Silent-Storm-Reconstruction).
The full BFG old/new object map is in
[`packaging/history-rewrite-map.txt`](../packaging/history-rewrite-map.txt).
