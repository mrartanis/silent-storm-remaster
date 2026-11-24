"""Test frozen HD installation, cache reuse, corruption recovery and source restoration."""
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch
from urllib.error import URLError
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


class HDSourceReleaseTests(unittest.TestCase):
    def fixture(self, root):
        stream = io.BytesIO()
        data = b'PNG fixture'
        with zipfile.ZipFile(stream, 'w') as zipped:
            zipped.writestr('object.png', data)
        archive = stream.getvalue()
        manifest = {'archives': [{'file': 'sources.zip', 'bytes': len(archive),
                                  'sha256': hashlib.sha256(archive).hexdigest()}],
                    'paths': {'assets/terrain-hd/a.png': 'oid', 'assets/terrain-hd/b.png': 'oid'},
                    'objects': {'oid': {'archive': 'sources.zip', 'member': 'object.png',
                                        'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}},
                    'file_count': 2, 'unique_file_count': 1}
        index = root / 'manifest.json'
        index.write_text(json.dumps(manifest))
        self.release(index)
        return index, archive

    def release(self, index, **changes):
        release = {'schema': 1, 'repository': 'mrartanis/silent-storm-remaster',
                   'tag': 'assets-hd-sources-v0.1', 'manifestSha256': hd.digest(index)}
        release.update(changes)
        index.with_name('release.json').write_text(json.dumps(release))

    def test_missing_archive_explains_explicit_download(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, _ = self.fixture(root)
            with patch.object(sources, 'verified_download') as fetch:
                with self.assertRaisesRegex(RuntimeError, '--download'):
                    sources.restore(index, root, verify_only=True)
                fetch.assert_not_called()
            self.assertFalse((root / 'assets').exists())

    def test_download_pinned_release_then_reuse_without_network(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, archive = self.fixture(root)

            def download(url, destination, digest, **options):
                self.assertEqual(url, 'https://github.com/mrartanis/silent-storm-remaster/'
                                      'releases/download/assets-hd-sources-v0.1/sources.zip')
                self.assertEqual(digest, hashlib.sha256(archive).hexdigest())
                destination.write_bytes(archive)

            with patch.object(sources, 'verified_download', side_effect=download) as fetch:
                sources.restore(index, root, verify_only=True, download=True)
                fetch.assert_called_once()
            self.assertFalse((root / 'assets').exists())
            with patch.object(sources, 'verified_download', side_effect=AssertionError('must reuse cache')):
                sources.restore(index, root, download=True)
            self.assertEqual((root / 'assets/terrain-hd/b.png').read_bytes(), b'PNG fixture')

    def test_offline_backup_directory_and_legacy_manifest(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, archive = self.fixture(root)
            index.with_name('release.json').unlink()
            backup = root / 'backup'
            backup.mkdir()
            (backup / 'sources.zip').write_bytes(archive)
            sources.restore(index, root, verify_only=True, archive_dir=backup)
            self.assertFalse((root / 'assets').exists())
            self.assertFalse((root / 'sources.zip').exists())

    def test_mismatched_manifest_blocks_network_and_restoration(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, _ = self.fixture(root)
            self.release(index, manifestSha256='0' * 64)
            with patch.object(sources, 'verified_download') as fetch:
                with self.assertRaisesRegex(ValueError, 'manifest checksum'):
                    sources.restore(index, root, download=True)
                fetch.assert_not_called()
            self.assertFalse((root / 'assets').exists())

    def test_runtime_release_tag_is_not_a_source_release(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, _ = self.fixture(root)
            self.release(index, tag='hd-v0.1')
            with patch.object(sources, 'verified_download') as fetch:
                with self.assertRaisesRegex(ValueError, 'source release'):
                    sources.restore(index, root, download=True)
                fetch.assert_not_called()

    def test_invalid_archive_path_is_rejected_before_download(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, _ = self.fixture(root)
            manifest = json.loads(index.read_text())
            manifest['archives'][0]['file'] = '../outside.zip'
            index.write_text(json.dumps(manifest))
            self.release(index)
            with patch.object(sources, 'verified_download') as fetch:
                with self.assertRaisesRegex(ValueError, 'archive path'):
                    sources.restore(index, root, download=True)
                fetch.assert_not_called()

    def test_corrupt_existing_archive_is_preserved(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, archive = self.fixture(root)
            damaged = b'!' * len(archive)
            (root / 'sources.zip').write_bytes(damaged)
            with patch.object(sources, 'verified_download') as fetch:
                with self.assertRaisesRegex(ValueError, 'checksum mismatch'):
                    sources.restore(index, root, download=True)
                fetch.assert_not_called()
            self.assertEqual((root / 'sources.zip').read_bytes(), damaged)
            self.assertFalse((root / 'assets').exists())

    def test_legacy_lfs_pointer_is_replaced_by_verified_download(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, archive = self.fixture(root)
            (root / 'sources.zip').write_bytes(b'version https://git-lfs.github.com/spec/v1\n')

            def download(url, destination, digest, **options):
                destination.write_bytes(archive)

            with patch.object(sources, 'verified_download', side_effect=download):
                sources.restore(index, root, verify_only=True, download=True)
            self.assertEqual((root / 'sources.zip').read_bytes(), archive)

    def test_modified_png_is_preserved_after_download(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, archive = self.fixture(root)
            edited = root / 'assets/terrain-hd/a.png'
            edited.parent.mkdir(parents=True)
            edited.write_bytes(b'user edits')

            def download(url, destination, digest, **options):
                destination.write_bytes(archive)

            with patch.object(sources, 'verified_download', side_effect=download):
                with self.assertRaisesRegex(RuntimeError, 'overwrite modified PNG'):
                    sources.restore(index, root, download=True)
            self.assertEqual(edited.read_bytes(), b'user edits')

    def test_failed_download_does_not_restore_pngs(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, _ = self.fixture(root)
            with patch.object(sources, 'verified_download', side_effect=URLError('offline')):
                with self.assertRaises(URLError):
                    sources.restore(index, root, download=True)
            self.assertFalse((root / 'assets').exists())

    def test_shared_downloader_checks_sha256_before_replacing_pointer(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, archive = self.fixture(root)
            pointer = b'version https://git-lfs.github.com/spec/v1\n'
            target = root / 'sources.zip'
            target.write_bytes(pointer)
            response = io.BytesIO(b'!' * len(archive))
            response.headers = {'Content-Length': str(len(archive))}
            with patch('verified_download.urllib.request.urlopen', return_value=response):
                with self.assertRaisesRegex(ValueError, 'SHA-256'):
                    sources.restore(index, root, download=True)
            self.assertEqual(target.read_bytes(), pointer)
            self.assertFalse(list(root.glob('*.part')))
            self.assertFalse((root / 'assets').exists())

    def test_shared_downloader_verifies_then_restores(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            index, archive = self.fixture(root)
            response = io.BytesIO(archive)
            response.headers = {'Content-Length': str(len(archive))}
            with patch('verified_download.urllib.request.urlopen', return_value=response):
                sources.restore(index, root, download=True)
            self.assertEqual((root / 'sources.zip').read_bytes(), archive)
            self.assertEqual((root / 'assets/terrain-hd/a.png').read_bytes(), b'PNG fixture')
            self.assertFalse(list(root.glob('*.part')))

    def test_playable_hd_installer_ignores_newer_source_release(self):
        releases = [{'tag_name': 'assets-hd-sources-v0.1', 'draft': False, 'prerelease': False,
                     'published_at': '2026-11-23T12:00:00Z'},
                    {'tag_name': 'hd-v0.1', 'draft': False, 'prerelease': False,
                     'published_at': '2026-10-25T19:15:30Z'}]
        with patch.object(hd, 'github_open', return_value=io.BytesIO(json.dumps(releases).encode())):
            self.assertEqual(hd.latest_hd_release()['tag_name'], 'hd-v0.1')


if __name__ == '__main__':
    unittest.main()
