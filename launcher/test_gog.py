"""Offline importer regressions; no proprietary installer or game data needed."""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "packaging"))

import gog
import install


def game_data(root):
    root.mkdir(parents=True, exist_ok=True)
    for name in install.DATA_FILES:
        (root / name).write_bytes(b"original " + name.encode())
    for name in install.DATA_DIRS:
        (root / name).mkdir()
        (root / name / "resource").write_bytes(name.encode())
    return root


class ImportTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name) / "launcher"
        self.root.mkdir()
        self.installer = Path(self.temporary.name) / "setup game.exe"
        self.installer.write_bytes(b"MZ offline installer fixture")
        self.addCleanup(patch.stopall)
        patch("gog.innoextract", return_value="innoextract-fixture").start()
        patch("install.require_space").start()
        self.progress = Mock()

    def extract_fixture(self, command, *, layout="app/Silent Storm", **options):
        unpacked = Path(command[command.index("--output-dir") + 1])
        data = game_data(unpacked / layout)
        (data / "game.exe").write_bytes(b"never import Windows executables")
        (data / "fmod.dll").write_bytes(b"never import Windows libraries")
        game_data(unpacked / "app/Silent Storm Sentinels")
        return Mock(returncode=0, poll=Mock(return_value=0))

    def run_import(self, cancel=lambda: None):
        return gog.import_installer(self.root, self.installer, self.progress, cancel)

    def assert_no_staging(self):
        self.assertFalse(list(self.root.glob("gog-*")))

    def test_gold_imports_only_original_game_data(self):
        with patch("install.start_child", side_effect=self.extract_fixture) as child:
            destination = self.run_import()
        self.assertEqual(destination, self.root / gog.DATA_DIRECTORY)
        self.assertFalse(install.missing_data(destination))
        self.assertEqual(set(p.name for p in destination.iterdir()),
                         set(install.DATA_FILES + install.DATA_DIRS))
        self.assertEqual(self.installer.read_bytes(), b"MZ offline installer fixture")
        command = child.call_args.args[0]
        self.assertEqual(command[0], "innoextract-fixture")
        self.assertEqual(command[-1], str(self.installer.resolve()))
        filters = [command[i + 1] for i, value in enumerate(command) if value == "--include"]
        self.assertEqual(set(filters), {f"{root}/{name}" for root in gog.INSTALLER_ROOTS
                                       for name in install.DATA_FILES + install.DATA_DIRS})
        self.assertEqual(child.call_args.kwargs["stdin"], subprocess.DEVNULL)
        self.progress.assert_called_with("extract")
        self.assert_no_staging()

    def test_standalone_layout(self):
        def extract(command, **options):
            return self.extract_fixture(command, layout="app", **options)
        with patch("install.start_child", side_effect=extract):
            self.assertFalse(install.missing_data(self.run_import()))

    def test_failure_keeps_previous_import(self):
        old = game_data(self.root / gog.DATA_DIRECTORY)
        (old / "keep.txt").write_text("old data")
        def extract(command, **options):
            options["stdout"].write(b"Missing setup-1.bin")
            return Mock(returncode=2, poll=Mock(return_value=2))
        with patch("install.start_child", side_effect=extract):
            with self.assertRaisesRegex(RuntimeError, r"\.bin"):
                self.run_import()
        self.assertEqual((old / "keep.txt").read_text(), "old data")
        self.assert_no_staging()

    def test_cancellation_stops_extractor_and_keeps_previous_import(self):
        old = game_data(self.root / gog.DATA_DIRECTORY)
        child = Mock(returncode=-9, poll=Mock(return_value=None))
        child.wait.side_effect = [subprocess.TimeoutExpired("innoextract", .25), 0]
        cancel = Mock(side_effect=[None, install.Cancelled()])
        with patch("install.start_child", return_value=child):
            with self.assertRaises(install.Cancelled):
                self.run_import(cancel)
        child.kill.assert_called_once()
        self.assertFalse(install.missing_data(old))
        self.assert_no_staging()

    def test_cancellation_before_activation_is_atomic(self):
        old = game_data(self.root / gog.DATA_DIRECTORY)
        (old / "keep.txt").write_text("old")
        def cancel():
            if list(self.root.glob("gog-*/payload/scripts")):
                raise install.Cancelled()
        with patch("install.start_child", side_effect=self.extract_fixture):
            with self.assertRaises(install.Cancelled):
                self.run_import(cancel)
        self.assertEqual((old / "keep.txt").read_text(), "old")
        self.assert_no_staging()

    def test_success_replaces_only_managed_data(self):
        old = game_data(self.root / gog.DATA_DIRECTORY)
        (old / "obsolete").touch()
        engine = self.root / "engine"
        engine.mkdir()
        (engine / "Game").write_bytes(b"engine")
        with patch("install.start_child", side_effect=self.extract_fixture):
            self.run_import()
        self.assertFalse((old / "obsolete").exists())
        self.assertEqual((engine / "Game").read_bytes(), b"engine")
        self.assertFalse((self.root / ".game-data.previous").exists())
        self.assert_no_staging()

    def test_sentinels_only_and_empty_installer_are_rejected(self):
        for empty in (False, True):
            with self.subTest(empty=empty):
                def extract(command, **options):
                    if not empty:
                        unpacked = Path(command[command.index("--output-dir") + 1])
                        game_data(unpacked / "app/Silent Storm Sentinels")
                    return Mock(returncode=0, poll=Mock(return_value=0))
                with patch("install.start_child", side_effect=extract):
                    with self.assertRaisesRegex(ValueError, "No complete Silent Storm"):
                        self.run_import()
                self.assertFalse((self.root / gog.DATA_DIRECTORY).exists())
                self.assert_no_staging()

    def test_insufficient_space_does_not_start_extractor(self):
        with patch("install.require_space", side_effect=OSError("Free space required")), \
                patch("install.start_child") as child:
            with self.assertRaises(OSError):
                self.run_import()
        child.assert_not_called()
        self.assert_no_staging()

    def test_incomplete_data_does_not_replace_previous_import(self):
        old = game_data(self.root / gog.DATA_DIRECTORY)
        (old / "keep.txt").write_text("old")
        def extract(command, **options):
            result = self.extract_fixture(command, **options)
            unpacked = Path(command[command.index("--output-dir") + 1])
            (unpacked / "app/Silent Storm/tree.mma").unlink()
            return result
        with patch("install.start_child", side_effect=extract):
            with self.assertRaisesRegex(ValueError, "No complete Silent Storm"):
                self.run_import()
        self.assertEqual((old / "keep.txt").read_text(), "old")
        self.assert_no_staging()

    @unittest.skipIf(sys.platform == "win32", "creating symlinks may require Windows administrator rights")
    def test_extracted_symlinks_are_rejected(self):
        def extract(command, **options):
            result = self.extract_fixture(command, **options)
            unpacked = Path(command[command.index("--output-dir") + 1])
            (unpacked / "app/Silent Storm/res/link").symlink_to(self.installer)
            return result
        with patch("install.start_child", side_effect=extract):
            with self.assertRaisesRegex(ValueError, "Symbolic links"):
                self.run_import()
        self.assertFalse((self.root / gog.DATA_DIRECTORY).exists())
        self.assertEqual(self.installer.read_bytes(), b"MZ offline installer fixture")
        self.assert_no_staging()

    def test_invalid_source_and_symlink_destination(self):
        with self.assertRaisesRegex(ValueError, "offline GOG"):
            gog.import_installer(self.root, self.root / "not-an-installer", self.progress, lambda: None)
        if sys.platform != "win32":
            (self.root / gog.DATA_DIRECTORY).symlink_to(self.root, target_is_directory=True)
            with self.assertRaisesRegex(ValueError, "regular directory"):
                self.run_import()


class ExtractorDiscoveryTests(unittest.TestCase):
    def test_finder_homebrew_path(self):
        with patch.object(gog.sys, "platform", "darwin"), \
                patch("gog.shutil.which", side_effect=lambda p: p if p == "/opt/homebrew/bin/innoextract" else None):
            self.assertEqual(gog.innoextract(), "/opt/homebrew/bin/innoextract")

    def test_missing_extractor(self):
        with patch("gog.shutil.which", return_value=None):
            with self.assertRaises(gog.ExtractorMissing):
                gog.innoextract()


if __name__ == "__main__":
    unittest.main()
