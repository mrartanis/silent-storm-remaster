# Publish releases

[Русский](releasing.md) · [Home](../README.en.md)

## Engine

1. Tag the commit with a tag beginning with `v` and push the tag to the repository. Use a hyphen in the tag for a prerelease.
2. Wait for [Engine CI and release](../.github/workflows/engine-release.yml) to finish. The workflow builds, checks, and publishes the three desktop archives and the Android APK with `SHA256SUMS`.
3. Check the assets and installation links on the [Releases page](https://github.com/mrartanis/silent-storm-remaster/releases). If an upload stops at a draft, rerun the workflow.

To build archives without publishing, run **Engine CI and release** manually for the branch. Download the archives from that run.

When changing network formats or replication in a way that breaks compatibility, increment the protocol version on the second line of [network-version.txt](../network-version.txt). The third line identifies the game data family.

## HD pack

1. Prepare the approved textures using the [source restoration guide](hd-authoring.en.md). Install Python 3.11+, Pillow, and 7-Zip.
2. Run the commands from the repository root. Use a fresh staging folder and replace `vNEXT` with the new version.

```powershell
python assets/terrain-hd/build_pack.py --output ../hd-release-stage/res-hd
python assets/terrain-hd/validate_pack.py --pack ../hd-release-stage/res-hd --no-update-queue
Copy-Item LICENSE.md ../hd-release-stage/HD-LICENSE.md
Set-Location ../hd-release-stage
7z a -t7z -mx=9 -m0=Delta:4 -m1=LZMA2:d=1024m:fb=273:mf=bt4:mc=1024 -ms=on -mmt=2 -mtm=off -v1900m silent-storm-hd-vNEXT.7z 'res-hd/Textures*.res' res-hd/manifest.json HD-LICENSE.md
7z t silent-storm-hd-vNEXT.7z.001
```

3. Create `hd-vNEXT-index.json` using the [index format example](../data/hd-runtime/hd-v0.1-index.json). Record the tag, volume names and sizes, and SHA-256 hashes for the volumes, manifest, and `res-hd` payloads. Verify the hashes against the extracted archive and prepare `SHA256SUMS`.
4. Create a draft with the tag `hd-vNEXT` in `mrartanis/silent-storm-remaster`. Upload every volume, the index, the license, and `SHA256SUMS`. Publish it as a stable release with `--latest=false` so the latest release link continues to point to the engine.
