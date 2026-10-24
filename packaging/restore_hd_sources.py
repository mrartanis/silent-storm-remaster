"""Restore frozen authoring PNGs on demand, preserving every original path."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def restore(manifest_path, root, verify_only=False):
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    root = root.resolve()
    paths = {}
    for name, oid in manifest['paths'].items():
        target = (root / name).resolve()
        if not target.is_relative_to(root / 'assets/terrain-hd') or target.suffix.lower() != '.png':
            raise ValueError(f'Invalid source path: {name}')
        paths.setdefault(oid, []).append(target)
    count = 0
    for part in manifest['archives']:
        if Path(part['file']).name != part['file']:
            raise ValueError('Invalid source archive path')
        archive = manifest_path.parent / part['file']
        if not archive.is_file() or archive.stat().st_size != part['bytes']:
            raise RuntimeError('Source archive missing or still an LFS pointer. Run: '
                               'git lfs pull --include="data/hd-sources/*" --exclude=""')
        with archive.open('rb') as stream:
            if hashlib.file_digest(stream, 'sha256').hexdigest() != part['sha256']:
                raise ValueError(f'Source archive checksum mismatch: {archive}')
        with zipfile.ZipFile(archive) as zipped:
            wanted = {oid: entry for oid, entry in manifest['objects'].items()
                      if entry['archive'] == part['file']}
            if set(zipped.namelist()) != {entry['member'] for entry in wanted.values()}:
                raise ValueError(f'Unexpected source archive members: {archive}')
            for oid, entry in wanted.items():
                data = zipped.read(entry['member'])
                if len(data) != entry['bytes'] or hashlib.sha256(data).hexdigest() != entry['sha256']:
                    raise ValueError(f'Source image checksum mismatch: {entry["member"]}')
                if not verify_only:
                    for target in paths[oid]:
                        if target.exists():
                            with target.open('rb') as stream:
                                if hashlib.file_digest(stream, 'sha256').hexdigest() == entry['sha256']:
                                    continue
                            raise RuntimeError(f'Refusing to overwrite modified PNG: {target}')
                        target.parent.mkdir(parents=True, exist_ok=True)
                        temporary = target.with_name(target.name + '.restoring')
                        temporary.write_bytes(data)
                        temporary.replace(target)
                count += 1
        print(f'{part["file"]}: verified {len(wanted)} unique PNGs')
    if count != manifest['unique_file_count'] or len(manifest['paths']) != manifest['file_count']:
        raise ValueError('Incomplete source manifest')
    print(f'{"Verified" if verify_only else "Restored"} {manifest["file_count"]} PNG paths ({count} unique images)')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=ROOT / 'data/hd-sources/manifest.json')
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    try:
        restore(args.manifest, args.root, args.verify_only)
    except (OSError, ValueError, RuntimeError, KeyError, zipfile.BadZipFile) as error:
        parser.exit(1, f'Source restoration failed: {error}\n')
