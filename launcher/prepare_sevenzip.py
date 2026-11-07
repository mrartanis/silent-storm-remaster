"""Bundle official standalone HD archive tools with matching source and notices."""

import argparse
import json
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tarfile
import tempfile

from prepare_innoextract import download


VERSION = '26.06'
BASE = 'https://github.com/ip7z/7zip/releases/download/' + VERSION + '/'
ARCHIVES = {
    'win32': ('7z2606-extra.7z', '396c180f12d2247c24b0b1fcd482c6fc2a2700f61211ffede1061dd971f5e104'),
    'linux': ('7z2606-linux-x64.tar.xz', '2b539cf067b2ad48e1baa5093dbaa8818b73542d1222a0ffde9dc5f2922ea62e'),
    'darwin': ('7z2606-mac.tar.xz', 'b6f17920b4240940461dac8ef4009891881ac44d2a5ead625ede2ec8472185dd'),
    'source': ('7z2606-src.tar.xz', 'a4e374c8f864f51df48c2cfd0e1413ec3b2ea030deae852d6a655402ebb61ac4'),
    'bootstrap': ('7zr.exe', 'e80db2dad715a6c5adabe292d31ed54e11f4e9b99ca349827b50573b175891bc'),
}
DEFAULT_OUTPUT = Path(__file__).resolve().parents[1] / 'build/launcher-sevenzip'


def prepare(output):
    if sys.platform not in ('win32', 'linux', 'darwin'):
        raise ValueError(f'Unsupported launcher platform: {sys.platform}')
    if sys.platform != 'darwin' and platform.machine().lower() not in ('amd64', 'x86_64'):
        raise ValueError('Windows and Linux launchers currently target x86_64')
    output.mkdir(parents=True, exist_ok=True)
    licenses = output / 'license'
    licenses.mkdir(exist_ok=True)
    sources = output / 'source'
    sources.mkdir(exist_ok=True)
    source_name, source_hash = ARCHIVES['source']
    source = sources / source_name
    download(BASE + source_name, source, source_hash)
    with tarfile.open(source) as bundle:
        for member in bundle.getmembers():
            name = Path(member.name).name
            if member.isfile() and name.lower().startswith(('license', 'copying', 'lgpl', 'unrarlicense')):
                (licenses / name).write_bytes(bundle.extractfile(member).read())
    if not any(licenses.iterdir()):
        raise ValueError('7-Zip redistribution notices are missing')
    name, digest = ARCHIVES[sys.platform]
    executable = output / ('7za.exe' if sys.platform == 'win32' else '7zz')
    with tempfile.TemporaryDirectory(prefix='sevenzip-build-') as temporary:
        temporary = Path(temporary)
        archive = temporary / name
        download(BASE + name, archive, digest)
        if sys.platform == 'win32':
            bootstrap_name, bootstrap_hash = ARCHIVES['bootstrap']
            bootstrap = temporary / bootstrap_name
            download(BASE + bootstrap_name, bootstrap, bootstrap_hash)
            subprocess.run([str(bootstrap), 'x', '-y', '-bso0', '-o' + str(temporary / 'extra'), str(archive)], check=True)
            shutil.copy2(temporary / 'extra/x64/7za.exe', executable)
        else:
            with tarfile.open(archive) as bundle:
                executable.write_bytes(bundle.extractfile('7zz').read())
    executable.chmod(0o755)
    result = subprocess.run([str(executable), 'i'], capture_output=True, text=True, check=True)
    if VERSION not in result.stdout:
        raise ValueError('Unexpected bundled HD extractor version')
    metadata = {'version': VERSION, 'platform': sys.platform, 'machine': platform.machine(),
                'archive': name, 'sha256': digest, 'source_archive': source_name, 'source_sha256': source_hash,
                'launcher_commit': subprocess.check_output(
                    ['git', '-C', str(Path(__file__).resolve().parents[1]), 'rev-parse', 'HEAD'], text=True).strip()}
    (output / 'BUILD.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8')
    print(f'Prepared bundled HD extractor {VERSION}: {executable}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    prepare(parser.parse_args().output)
