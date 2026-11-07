"""Compile redistributable multipart Inno Setup test data on a Windows builder."""

import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
import tempfile

from prepare_innoextract import DEFAULT_OUTPUT, download


def create(output):
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='gog-fixture-') as directory:
        root = Path(directory)
        compiler_setup = root / 'innosetup.exe'
        download('https://github.com/jrsoftware/issrc/releases/download/is-6_0_5/innosetup-6.0.5.exe',
                 compiler_setup, 'ae6823b523df87e9441789e51845434e4e0e70aac0b88afe80f94f20f4b98acb')
        compiler = root / 'compiler'
        subprocess.run([str(DEFAULT_OUTPUT / 'innoextract.exe'), '--extract', '--silent',
                        '--output-dir', str(compiler), str(compiler_setup)], check=True)
        payload = root / 'payload'
        files = {name: b'launcher test fixture, no original game data\n' for name in
                 ('game.db', 'start.cfg', 'tree.mma', 'cfg/test.cfg', 'scripts/test.lua')}
        files['res/payload.bin'] = random.Random(20261107).randbytes(5 * 1024 * 1024)
        for name, data in files.items():
            path = payload / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        script = root / 'fixture.iss'
        script.write_text(f'''[Setup]
AppName=Silent Storm launcher test data
AppVersion=1
DefaultDirName={{tmp}}\\SilentStormLauncherFixture
Uninstallable=no
DiskSpanning=yes
DiskSliceSize=4194304
Compression=lzma2
SolidCompression=yes
OutputDir={output}
OutputBaseFilename=setup
[Files]
Source: "{payload}\\*"; DestDir: "{{app}}\\Silent Storm"; Flags: recursesubdirs createallsubdirs
Source: "{payload}\\game.db"; DestDir: "{{app}}\\Silent Storm Sentinels"
Source: "{payload}\\game.db"; DestDir: "{{app}}\\Silent Storm"; DestName: "excluded.exe"
''', encoding='utf-8')
        subprocess.run([str(compiler / 'app/ISCC.exe'), str(script)], check=True)
        if not list(output.glob('setup-*.bin')):
            raise ValueError('Fixture must exercise external .bin slices')
        manifest = {name: hashlib.sha256(data).hexdigest() for name, data in files.items()}
        (output / 'manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    create(parser.parse_args().output)
