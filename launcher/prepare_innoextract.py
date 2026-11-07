"""Prepare the native GOG extractor for PyInstaller; never a runtime download."""

import argparse
import hashlib
import json
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tarfile
import tempfile
from urllib.request import urlopen
import zipfile


VERSION = '1.9'
ARCHIVES = {
    'win32': ('innoextract-1.9-windows.zip', '6989342c9b026a00a72a38f23b62a8e6a22cc5de69805cf47d68ac2fec993065'),
    'linux': ('innoextract-1.9-linux.tar.xz', '008efe5011476ccc4aae17c3e22038b5a1bc5c7aad2b9d4d869537bf3874d21f'),
    'source': ('innoextract-1.9.tar.gz', '6344a69fc1ed847d4ed3e272e0da5998948c6b828cb7af39c6321aba6cf88126'),
}
DEFAULT_OUTPUT = Path(__file__).resolve().parents[1] / 'build/launcher-innoextract'


def download(url, destination, sha256):
    with urlopen(url, timeout=90) as response, destination.open('wb') as output:
        shutil.copyfileobj(response, output)
    actual = hashlib.file_digest(destination.open('rb'), 'sha256').hexdigest()
    if actual != sha256:
        raise ValueError(f'Unexpected SHA-256 for {url}: {actual}')


def prepare(output):
    output.mkdir(parents=True, exist_ok=True)
    licenses = output / 'license'
    licenses.mkdir(exist_ok=True)
    executable = output / ('innoextract.exe' if sys.platform == 'win32' else 'innoextract')
    metadata = {'innoextract': VERSION, 'platform': sys.platform, 'machine': platform.machine(),
                'launcher_commit': subprocess.check_output(
                    ['git', '-C', str(Path(__file__).resolve().parents[1]), 'rev-parse', 'HEAD'], text=True).strip()}
    with tempfile.TemporaryDirectory(prefix='innoextract-build-') as temporary:
        temporary = Path(temporary)
        if sys.platform in ('win32', 'linux'):
            if sys.platform == 'linux' and platform.machine().lower() not in ('x86_64', 'amd64'):
                raise ValueError('The Linux launcher currently targets x86_64')
            name, digest = ARCHIVES[sys.platform]
            archive = temporary / name
            download('https://constexpr.org/innoextract/files/' + name, archive, digest)
            metadata.update(archive=name, sha256=digest)
            if sys.platform == 'win32':
                with zipfile.ZipFile(archive) as bundle:
                    executable.write_bytes(bundle.read('innoextract.exe'))
                    for member in bundle.namelist():
                        if member.startswith('license/') and not member.endswith('/'):
                            (licenses / Path(member).name).write_bytes(bundle.read(member))
            else:
                with tarfile.open(archive) as bundle:
                    executable.write_bytes(bundle.extractfile('innoextract-1.9-linux/bin/amd64/innoextract').read())
                    for member in bundle.getmembers():
                        if member.name.startswith('innoextract-1.9-linux/license/') and member.isfile():
                            (licenses / Path(member.name).name).write_bytes(bundle.extractfile(member).read())
        elif sys.platform == 'darwin':
            # Homebrew is a build dependency only. PyInstaller collects and
            # relocates this Mach-O executable's transitive dylib dependencies.
            prefix = Path(subprocess.check_output(['brew', '--prefix', 'innoextract'], text=True).strip())
            shutil.copy2(prefix / 'bin/innoextract', executable)
            formulae = ['innoextract', *subprocess.check_output(
                ['brew', 'deps', '--formula', '--installed', 'innoextract'], text=True).split()]
            metadata['homebrew'] = json.loads(subprocess.check_output(
                ['brew', 'info', '--json=v2', *formulae], text=True))['formulae']
            for formula in formulae:
                prefix = Path(subprocess.check_output(['brew', '--prefix', formula], text=True).strip()).resolve()
                found = []
                for notice in prefix.rglob('*'):
                    if notice.is_file() and notice.name.upper().startswith(('LICENSE', 'COPYING', 'COPYRIGHT')):
                        relative = notice.relative_to(prefix)
                        destination = licenses / formula / relative
                        destination.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(notice, destination)
                        found.append(str(relative))
                if not found and formula != 'innoextract':
                    raise ValueError(f'Missing redistribution notices in Homebrew {formula}')
            name, digest = ARCHIVES['source']
            archive = temporary / name
            download('https://constexpr.org/innoextract/files/' + name, archive, digest)
            with tarfile.open(archive) as bundle:
                (licenses / 'innoextract.LICENSE').write_bytes(bundle.extractfile('innoextract-1.9/LICENSE').read())
        else:
            raise ValueError(f'Unsupported launcher platform: {sys.platform}')
    executable.chmod(0o755)
    result = subprocess.run([str(executable), '--version'], capture_output=True, text=True, check=True)
    if not result.stdout.startswith('innoextract ' + VERSION):
        raise ValueError(f'Unexpected extractor version: {result.stdout}')
    metadata['version_output'] = result.stdout
    (output / 'BUILD.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8')
    print(f'Prepared bundled {result.stdout.splitlines()[0]}: {executable}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    prepare(parser.parse_args().output)
