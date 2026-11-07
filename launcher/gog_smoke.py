"""Verify the relocated frozen extractor with no system tools on PATH."""

import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import gog
import install


def verify_bundled_import(root, fixture, record):
    if not getattr(sys, 'frozen', False):
        raise ValueError('This smoke test must run from a frozen launcher archive')
    bundle = Path(sys._MEIPASS).resolve()
    executable = Path(gog.innoextract()).resolve()
    if not executable.is_relative_to(bundle):
        raise ValueError('The launcher selected an extractor outside its own package')
    metadata = bundle / 'innoextract/BUILD.json'
    if not metadata.is_file() or not any((bundle / 'innoextract/license').rglob('*')):
        raise ValueError('Bundled extractor build metadata or licenses are missing')
    env = install.host_environment()
    env['PATH'] = ''
    child = install.start_child([str(executable), '--version'], cwd=root, env=env,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
        creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == 'win32' else 0)
    try:
        output, _ = child.communicate(timeout=20)
    finally:
        if child.poll() is None:
            child.kill()
            child.wait()
    if child.returncode or not output.startswith('innoextract 1.9'):
        raise ValueError('Bundled extractor failed without system tools: ' + output)
    record('Bundled innoextract ran with an empty PATH')
    if fixture is None:
        return
    fixture = fixture.resolve()
    expected = json.loads((fixture / 'manifest.json').read_text(encoding='utf-8'))
    if not list(fixture.glob('setup-*.bin')):
        raise ValueError('The smoke fixture must include real external .bin slices')
    previous = os.environ.get('PATH')
    try:
        os.environ['PATH'] = ''
        data = gog.import_installer(root, fixture / 'setup.exe', lambda _phase: None, lambda: None)
        with tempfile.TemporaryDirectory(prefix='missing-slice-', dir=root) as missing:
            missing = Path(missing)
            slices = sorted(fixture.glob('setup-*.bin'))
            shutil.copy2(fixture / 'setup.exe', missing / 'setup.exe')
            for part in slices[:-1]:
                shutil.copy2(part, missing / part.name)
            try:
                gog.import_installer(root, missing / 'setup.exe', lambda _phase: None, lambda: None)
            except RuntimeError:
                pass
            else:
                raise ValueError('An incomplete multipart installer unexpectedly replaced existing data')
    finally:
        if previous is None:
            os.environ.pop('PATH', None)
        else:
            os.environ['PATH'] = previous
    actual = {str(path.relative_to(data).as_posix()): hashlib.file_digest(path.open('rb'), 'sha256').hexdigest()
              for path in data.rglob('*') if path.is_file()}
    if actual != expected:
        raise ValueError('The packaged extractor did not import the complete multipart fixture')
    record('Imported and verified the multipart GOG fixture with an empty PATH')
