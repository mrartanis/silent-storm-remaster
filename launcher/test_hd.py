"""Packaged HD installation must not depend on a system archiver."""

from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'packaging'))
import install


class ExtractorDiscoveryTests(unittest.TestCase):
    def test_packaged_hd_extractor_is_used_without_system_lookup(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            tool = root / 'sevenzip' / ('7za.exe' if sys.platform == 'win32' else '7zz')
            tool.parent.mkdir()
            tool.touch()
            with patch.object(install.sys, '_MEIPASS', str(root), create=True), \
                    patch.object(install.sys, 'frozen', True, create=True), \
                    patch('install.seven_zip') as external:
                self.assertEqual(install.hd_extractor(), str(tool))
            external.assert_not_called()

    def test_incomplete_package_is_not_hidden_by_a_system_archiver(self):
        with tempfile.TemporaryDirectory() as directory, \
                patch.object(install.sys, '_MEIPASS', directory, create=True), \
                patch.object(install.sys, 'frozen', True, create=True), \
                patch('install.seven_zip', return_value='system-7zip') as external:
            with self.assertRaises(RuntimeError):
                install.hd_extractor()
            external.assert_not_called()


if __name__ == '__main__':
    unittest.main()
