"""Restore frozen authoring PNGs on demand, preserving every original path."""
import argparse
import hashlib
from http.client import HTTPException
import json
from pathlib import Path
import sys
from urllib.parse import quote
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'packaging'))
from verified_download import download as verified_download


def release_url(manifest_path):
    release = json.loads(manifest_path.with_name('release.json').read_text(encoding='utf-8'))
    with manifest_path.open('rb') as stream:
        if hashlib.file_digest(stream, 'sha256').hexdigest() != release['manifestSha256']:
            raise ValueError('Source manifest checksum mismatch')
    repository = release['repository']
    tag = release['tag']
    if (release['schema'] != 1 or len(repository.split('/')) != 2 or
            any(not name or name in ('.', '..') for name in repository.split('/')) or
            not tag.startswith('assets-hd-sources-')):
        raise ValueError('Invalid source release metadata')
    return f'https://github.com/{quote(repository, safe="/")}/releases/download/{quote(tag, safe="")}'


def restore(manifest_path, root, verify_only=False, download=False, archive_dir=None):
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    root = root.resolve()
    archive_dir = (archive_dir or manifest_path.parent).resolve()
    paths = {}
    for name, oid in manifest['paths'].items():
        target = (root / name).resolve()
        if not target.is_relative_to(root / 'assets/terrain-hd') or target.suffix.lower() != '.png':
            raise ValueError(f'Invalid source path: {name}')
        paths.setdefault(oid, []).append(target)
    for part in manifest['archives']:
        name = part['file']
        if Path(name).name != name or name in ('', '.', '..') or '\\' in name or ':' in name:
            raise ValueError('Invalid source archive path')
    base_url = release_url(manifest_path) if download else None
    count = 0
    for part in manifest['archives']:
        archive = archive_dir / part['file']
        pointer = (archive.is_file() and archive.stat().st_size < 1024 and
                   archive.read_bytes().startswith(b'version https://git-lfs.github.com/spec/v1\n'))
        if download and (not archive.exists() or pointer):
            archive_dir.mkdir(parents=True, exist_ok=True)
            verified_download(f'{base_url}/{quote(part["file"], safe="")}', archive,
                              part['sha256'], timeout=180)
        if not archive.is_file() or (pointer and not download):
            raise RuntimeError('Source archive missing or still an LFS pointer. Run: '
                               'python packaging/restore_hd_sources.py --download --verify-only '
                               '(or supply a local backup with --archive-dir PATH)')
        if archive.stat().st_size != part['bytes']:
            raise ValueError(f'Source archive size mismatch: {archive}')
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
    parser.add_argument('--download', action='store_true',
                        help='Download missing ZIPs from the pinned source release (12.54 GiB for v0.1)')
    parser.add_argument('--archive-dir', type=Path,
                        help='Use another directory for downloaded ZIPs or an offline backup')
    args = parser.parse_args()
    try:
        restore(args.manifest, args.root, args.verify_only, args.download, args.archive_dir)
    except (OSError, ValueError, RuntimeError, KeyError, zipfile.BadZipFile, HTTPException) as error:
        parser.exit(1, f'Source restoration failed: {error}\n')
