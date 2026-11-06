"""Install the latest HD release for a game, or prepare the frozen build cache.

No PNG decoding or texture generation is performed by this command.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parent.parent
STATE = '.prepared-hd.json'
RELEASES = 'https://api.github.com/repos/mrartanis/silent-storm-remaster/releases'


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def write_json(path, value):
    temporary = path.with_name(path.name + '.tmp')
    temporary.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')
    temporary.replace(path)


def payloads(index):
    result = {}
    for name, sha in index['payloadSha256'].items():
        path = Path(name)
        if path.parent == Path('res-hd'):
            if path.name != 'manifest.json' and not (
                    path.name.startswith('Textures') and path.suffix == '.res'):
                raise ValueError(f'Unexpected HD payload: {name}')
            result[path.name] = sha
    if result.get('manifest.json') != index['manifestSha256'] or 'Textures.res' not in result:
        raise ValueError('Incomplete HD release index')
    return result


def file_states(directory, names):
    return {name: [p.stat().st_size, p.stat().st_mtime_ns]
            for name in names for p in [directory / name]}


def valid(directory, expected, signature):
    try:
        # Extra texture shards can override valid resources from the new pack.
        if {p.name for p in directory.glob('Textures*.res')} != set(expected) - {'manifest.json'}:
            return False
        states = file_states(directory, expected)
        marker = directory / STATE
        if marker.is_file():
            saved = json.loads(marker.read_text(encoding='utf-8'))
            if saved == {'index_sha256': signature, 'files': states}:
                return True
        if any(digest(directory / name) != sha for name, sha in expected.items()):
            return False
        write_json(marker, {'index_sha256': signature, 'files': states})
        return True
    except (OSError, ValueError, KeyError):
        return False


def seven_zip(explicit=None):
    candidates = [explicit] if explicit else ['7zz', '7z', '7za', r'C:\Program Files\7-Zip\7z.exe']
    for name in candidates:
        if name and (found := shutil.which(str(name))):
            return found
    raise RuntimeError('Install 7-Zip (7zz/7z), or pass --sevenzip PATH. It is needed only to unpack HD.')


def install_files(source, destination, expected, signature):
    destination.mkdir(parents=True, exist_ok=True)
    # Only remove obsolete shards owned and identified by the previous installer.
    extra = {p.name for p in destination.glob('Textures*.res')} - set(expected)
    if extra:
        try:
            previous = json.loads((destination / STATE).read_text(encoding='utf-8'))['files']
        except (OSError, ValueError, KeyError):
            previous = {}
        for name in extra:
            path = destination / name
            if name not in previous or file_states(destination, [name])[name] != previous[name]:
                raise RuntimeError(f'Unmanaged old HD shard: {path}. Move the old pack aside first.')
        for name in extra:
            (destination / name).unlink()
    for name, sha in expected.items():
        target = destination / name
        original = source / name
        if target.exists() and (target.samefile(original) or digest(target) == sha):
            continue
        temporary = target.with_name(target.name + '.installing')
        if temporary.exists():
            temporary.unlink()
        try:
            os.link(original, temporary)
        except OSError:
            shutil.copy2(original, temporary)
        temporary.replace(target)
    write_json(destination / STATE,
               {'index_sha256': signature, 'files': file_states(destination, expected)})


def prepare(index_path, cache, output=None, extractor=None):
    index = json.loads(index_path.read_text(encoding='utf-8'))
    expected = payloads(index)
    signature = digest(index_path)
    cache = cache.resolve()
    if not valid(cache, expected, signature):
        executable = seven_zip(extractor)
        for part in index['parts']:
            name = part['file']
            if Path(name).name != name:
                raise ValueError(f'Invalid archive name: {name}')
            path = index_path.parent / name
            if not path.is_file() or path.stat().st_size != part['bytes']:
                raise RuntimeError('HD archive is missing or is an LFS pointer. Run: '
                                   'git lfs pull --include="data/hd-runtime/*" --exclude=""')
            if digest(path) != part['sha256']:
                raise RuntimeError(f'HD archive checksum mismatch: {path}')
        cache.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='hd-unpack-', dir=cache.parent) as temporary:
            subprocess.run([executable, 'x', '-y', '-bd', '-bso0',
                            '-o' + temporary, str(index_path.parent / index['parts'][0]['file'])],
                           check=True)
            unpacked = Path(temporary) / 'res-hd'
            if not valid(unpacked, expected, signature):
                raise RuntimeError('Extracted HD payload does not match the release SHA-256 index')
            install_files(unpacked, cache, expected, signature)
        print(f'{cache}: installed {index["release"]} ({index["textureCount"]} textures)')
    else:
        print(f'{cache}: {index["release"]} up to date')
    if output is not None and output.resolve() != cache:
        output = output.resolve()
        if not valid(output, expected, signature):
            install_files(cache, output, expected, signature)
            print(f'{output}: linked/copied ready-to-use HD pack')


def github_open(url):
    return urlopen(Request(url, headers={'User-Agent': 'silent-storm-hd-installer'}), timeout=60)


def latest_hd_release():
    latest = None
    page = 1
    while True:
        with github_open(f'{RELEASES}?per_page=100&page={page}') as response:
            releases = json.load(response)
        for release in releases:
            if (release['tag_name'].startswith('hd-') and not release['draft'] and
                    not release['prerelease'] and release['published_at'] and
                    (latest is None or release['published_at'] > latest['published_at'])):
                latest = release
        if len(releases) < 100:
            break
        page += 1
    if latest is None:
        raise RuntimeError('No stable HD release found')
    return latest


def download(asset, destination):
    print(f'Downloading {destination.name} ({asset["size"]:,} bytes)', flush=True)
    with github_open(asset['browser_download_url']) as response, destination.open('wb') as target:
        shutil.copyfileobj(response, target, length=1024 * 1024)
    if destination.stat().st_size != asset['size']:
        raise RuntimeError(f'Incomplete download: {destination.name}')


def install_latest(game_dir, extractor=None):
    game_dir = game_dir.resolve()
    if not (game_dir / 'game.db').is_file():
        raise RuntimeError(f'Select the game folder containing game.db: {game_dir}')
    target = game_dir / 'res-hd'
    backup = game_dir / '.res-hd-previous'
    lock = game_dir / '.res-hd-installing'
    try:
        lock.mkdir()
    except FileExistsError:
        raise RuntimeError(f'An HD installation is already using {lock}') from None
    try:
        # Recover the previous pack if an earlier replacement stopped between renames.
        if backup.exists() and not target.exists():
            backup.rename(target)
        if target.is_symlink() or (target.exists() and not target.is_dir()):
            raise RuntimeError(f'Expected an HD directory: {target}')
        release = latest_hd_release()
        tag = release['tag_name']
        if not re.fullmatch(r'hd-[A-Za-z0-9][A-Za-z0-9._-]*', tag):
            raise ValueError(f'Invalid HD release tag: {tag}')
        assets = {asset['name']: asset for asset in release['assets']}
        index_name = f'{tag}-index.json'
        with tempfile.TemporaryDirectory(prefix='hd-download-', dir=game_dir) as temp:
            staging = Path(temp)
            index_path = staging / index_name
            download(assets[index_name], index_path)
            index = json.loads(index_path.read_text(encoding='utf-8'))
            if index['release'] != tag:
                raise ValueError(f'HD index does not match release {tag}')
            if not index['parts']:
                raise ValueError(f'HD index lists no archive volumes: {tag}')
            if valid(target, payloads(index), digest(index_path)):
                print(f'{target}: {tag} up to date')
                return
            if backup.exists():
                raise RuntimeError(f'Move the previous HD pack out of {backup} first')

            executable = seven_zip(extractor)
            for part in index['parts']:
                name = part['file']
                if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9._-]*\.7z(?:\.\d+)?', name):
                    raise ValueError(f'Invalid HD archive name: {name}')
                asset = assets[name]
                if asset['size'] != part['bytes']:
                    raise ValueError(f'HD archive size differs from the release index: {name}')
                download(asset, staging / name)

            prepared = staging / 'res-hd'
            prepare(index_path, prepared, extractor=executable)
            if target.exists():
                target.rename(backup)
            try:
                prepared.rename(target)
            except BaseException:
                if backup.exists():
                    backup.rename(target)
                raise
            if backup.exists():
                shutil.rmtree(backup)
            print(f'{target}: installed {tag}')
    finally:
        lock.rmdir()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-dir', type=Path, help='download and install the latest HD release here')
    parser.add_argument('--index', type=Path, default=ROOT / 'data/hd-runtime/hd-v0.1-index.json')
    parser.add_argument('--cache', type=Path, default=ROOT / 'data/res-hd')
    parser.add_argument('--output', type=Path)
    parser.add_argument('--sevenzip')
    args = parser.parse_args()
    try:
        if args.game_dir is not None:
            install_latest(args.game_dir, args.sevenzip)
        else:
            prepare(args.index, args.cache, args.output, args.sevenzip)
    except (OSError, ValueError, KeyError, RuntimeError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'HD preparation failed: {error}\n')
