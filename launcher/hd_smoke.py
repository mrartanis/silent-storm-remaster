"""Exercise HD volume downloads and native extraction from a relocated launcher."""

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

import install


def verify_bundled_hd(root, fixture, record):
    bundle = Path(sys._MEIPASS).resolve()
    executable = Path(install.hd_extractor()).resolve()
    if not executable.is_relative_to(bundle):
        raise ValueError('HD extraction selected an external program')
    if not (bundle / 'sevenzip/BUILD.json').is_file() or not list((bundle / 'sevenzip/source').glob('*.tar.xz')):
        raise ValueError('Bundled HD extractor metadata or source archive is missing')
    env = install.host_environment()
    env['PATH'] = ''
    child = install.start_child([str(executable), 'i'], env=env, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, text=True,
        creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == 'win32' else 0)
    try:
        output, _ = child.communicate(timeout=20)
    finally:
        if child.poll() is None:
            child.kill()
            child.wait()
    if child.returncode or '26.06' not in output:
        raise ValueError('Bundled HD extractor failed without system tools: ' + output)
    record('Bundled HD extractor ran with an empty PATH')
    if fixture is None:
        return {}
    fixture = fixture.resolve()
    index = json.loads((fixture / 'hd-test-index.json').read_text(encoding='utf-8'))
    assets = [{'name': p.name, 'size': p.stat().st_size, 'browser_download_url': p.as_uri(),
               'digest': 'sha256:' + hashlib.file_digest(p.open('rb'), 'sha256').hexdigest()}
              for p in fixture.iterdir() if p.is_file()]
    previous = os.environ.get('PATH')
    try:
        os.environ['PATH'] = ''
        (root / 'engine').mkdir(exist_ok=True)
        install.install_hd(root, {'tag_name': 'hd-test', 'assets': assets}, lambda *args: None, lambda: None)
    finally:
        if previous is None:
            os.environ.pop('PATH', None)
        else:
            os.environ['PATH'] = previous
    expected = install.payloads(index)
    for name, digest in expected.items():
        path = root / 'engine/res-hd' / name
        if hashlib.file_digest(path.open('rb'), 'sha256').hexdigest() != digest:
            raise ValueError('HD fixture payload hash mismatch: ' + name)
    record('Installed and verified split solid HD fixture with an empty PATH')
    return expected
