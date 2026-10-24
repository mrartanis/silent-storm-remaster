"""Test frozen HD installation, cache reuse, corruption recovery and source restoration."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def load(name, file):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'packaging' / file)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


hd = load('prepare_hd', 'prepare_hd.py')
sources = load('restore_hd_sources', 'restore_hd_sources.py')


class HDStorageTests(unittest.TestCase):
    def fixture(self, root):
        payload = root / 'payload/res-hd'
        payload.mkdir(parents=True)
        (payload / 'Textures.res').write_bytes(b'actual native fixture bytes')
        (payload / 'manifest.json').write_text('{"fixture":true}\n')
        release = root / 'release'
        release.mkdir()
        executable = hd.seven_zip()
        part = release / 'fixture.7z'
        subprocess.run([executable, 'a', '-t7z', '-bd', '-bso0', str(part), 'res-hd'],
                       cwd=payload.parent, check=True)
        index = {'release': 'fixture', 'textureCount': 1,
                 'manifestSha256': hd.digest(payload / 'manifest.json'),
                 'payloadSha256': {'res-hd/' + p.name: hd.digest(p) for p in payload.iterdir()},
                 'parts': [{'file': part.name, 'bytes': part.stat().st_size, 'sha256': hd.digest(part)}]}
        index_path = release / 'index.json'
        index_path.write_text(json.dumps(index))
        return index_path, root / 'cache', root / 'output'

    def test_extract_then_reuse_without_reading_archives_or_regenerating(self):
        with tempfile.TemporaryDirectory() as temp:
            index, cache, output = self.fixture(Path(temp))
            hd.prepare(index, cache, output)
            self.assertEqual((output / 'Textures.res').read_bytes(), b'actual native fixture bytes')
            with patch.object(hd.subprocess, 'run', side_effect=AssertionError('must not extract')):
                with patch.object(hd, 'digest', wraps=hd.digest) as hashes:
                    hd.prepare(index, cache, output)
                    self.assertEqual(hashes.call_count, 1)  # Small index only.

    def test_corrupt_cache_is_repaired_from_verified_archive(self):
        with tempfile.TemporaryDirectory() as temp:
            index, cache, output = self.fixture(Path(temp))
            hd.prepare(index, cache, output)
            (cache / 'Textures.res').write_bytes(b'corrupted')
            hd.prepare(index, cache, output)
            self.assertEqual((output / 'Textures.res').read_bytes(), b'actual native fixture bytes')

    def test_missing_lfs_archive_explains_fetch(self):
        with tempfile.TemporaryDirectory() as temp:
            index, cache, output = self.fixture(Path(temp))
            (index.parent / 'fixture.7z').write_text('version https://git-lfs.github.com/spec/v1\n')
            with self.assertRaisesRegex(RuntimeError, 'git lfs pull'):
                hd.prepare(index, cache, output)
            self.assertFalse(cache.exists())

    def test_corrupt_download_is_rejected_before_extraction(self):
        with tempfile.TemporaryDirectory() as temp:
            index, cache, output = self.fixture(Path(temp))
            part = index.parent / 'fixture.7z'
            part.write_bytes(b'!' * part.stat().st_size)
            with self.assertRaisesRegex(RuntimeError, 'checksum mismatch'):
                hd.prepare(index, cache, output)

    def test_unmanaged_extra_shard_is_preserved_and_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            index, cache, output = self.fixture(Path(temp))
            output.mkdir()
            extra = output / 'Textures-9999.res'
            extra.write_bytes(b'user data')
            with self.assertRaisesRegex(RuntimeError, 'Unmanaged old HD shard'):
                hd.prepare(index, cache, output)
            self.assertEqual(extra.read_bytes(), b'user data')

    def test_restore_duplicate_paths_and_protect_modified_png(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            part = root / 'sources.zip'
            data = b'PNG fixture'
            with zipfile.ZipFile(part, 'w') as z:
                z.writestr('object.png', data)
            manifest = {'archives': [{'file': part.name, 'bytes': part.stat().st_size,
                                      'sha256': hd.digest(part)}],
                        'paths': {'assets/terrain-hd/a.png': 'oid', 'assets/terrain-hd/b.png': 'oid'},
                        'objects': {'oid': {'archive': part.name, 'member': 'object.png',
                                            'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}},
                        'file_count': 2, 'unique_file_count': 1}
            index = root / 'manifest.json'
            index.write_text(json.dumps(manifest))
            sources.restore(index, root, verify_only=True)
            self.assertFalse((root / 'assets').exists())
            sources.restore(index, root)
            self.assertEqual((root / 'assets/terrain-hd/b.png').read_bytes(), data)
            (root / 'assets/terrain-hd/a.png').write_bytes(b'edited')
            with self.assertRaisesRegex(RuntimeError, 'overwrite modified PNG'):
                sources.restore(index, root)


if __name__ == '__main__':
    unittest.main()
