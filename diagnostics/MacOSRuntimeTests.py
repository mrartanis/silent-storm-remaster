"""Loader-path resolution must never silently depend on the build machine."""
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import MacOSRuntime as runtime


class MacOSRuntimeTests(unittest.TestCase):
    def test_executable_rpath_uses_executable_loader_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            game = root / 'Game'
            source = root / 'vendor/libfirst.dylib'
            (root / 'lib').mkdir()
            expected = root / 'lib/libsecond.dylib'
            expected.touch()
            with patch.object(runtime, 'rpaths', side_effect=lambda p: ['@loader_path/lib'] if p == game else []):
                self.assertEqual(runtime.resolve_library('@rpath/libsecond.dylib', source, game, []), expected.resolve())

    def test_missing_transitive_library_fails_instead_of_using_host(self):
        with patch.object(runtime, 'rpaths', return_value=[]):
            with self.assertRaisesRegex(ValueError, 'Unresolved macOS runtime dependency'):
                runtime.resolve_library('@rpath/unpackaged-codec.dylib', Path('vendor/first.dylib'), Path('Game'), [])

    def test_wrong_architecture_fails_before_packaging(self):
        with tempfile.TemporaryDirectory() as temporary:
            stage = Path(temporary)
            with patch.object(runtime, 'output', return_value='x86_64\n'):
                with self.assertRaisesRegex(ValueError, 'Expected macOS arm64'):
                    runtime.bundle_runtime(stage / 'Game', stage, stage / 'deps')


if __name__ == '__main__':
    unittest.main()
