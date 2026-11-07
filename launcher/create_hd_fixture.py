"""Create a small split solid Delta/LZMA2 archive through the production codec."""

import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
import tempfile

from prepare_sevenzip import DEFAULT_OUTPUT


def create(output):
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='hd-fixture-') as directory:
        root = Path(directory)
        payload = root / 'res-hd'
        payload.mkdir()
        (payload / 'Textures.res').write_bytes(random.Random(20261107).randbytes(1024 * 1024))
        (payload / 'manifest.json').write_text('{"fixture":true}\n', encoding='utf-8')
        archive = output / 'hd-test.7z'
        subprocess.run([str(DEFAULT_OUTPUT / '7za.exe'), 'a', '-t7z', '-m0=Delta:4',
                        '-m1=LZMA2:d=1m', '-ms=on', '-v256k', str(archive), 'res-hd'], cwd=root, check=True)
        digest = lambda p: hashlib.file_digest(p.open('rb'), 'sha256').hexdigest()
        parts = sorted(output.glob('hd-test.7z.*'))
        if len(parts) < 2:
            raise ValueError('HD fixture must use multiple volumes')
        hashes = {p.name: digest(p) for p in payload.iterdir()}
        index = {'release': 'hd-test', 'nativeBytes': sum(p.stat().st_size for p in payload.iterdir()),
                 'parts': [{'file': p.name, 'bytes': p.stat().st_size, 'sha256': digest(p)} for p in parts],
                 'manifestSha256': hashes['manifest.json'],
                 'payloadSha256': {'res-hd/' + name: value for name, value in hashes.items()}}
        (output / 'hd-test-index.json').write_text(json.dumps(index, indent=2), encoding='utf-8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    create(parser.parse_args().output)
