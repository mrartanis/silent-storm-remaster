"""Launcher import controls and worker completion without network access."""

import json
from pathlib import Path
import tempfile
import time
import unittest
from unittest.mock import patch

from PySide6.QtWidgets import QApplication, QFileDialog, QMessageBox
from PySide6.QtGui import QCloseEvent

import launcher as ui
import gog
import install
from test_gog import game_data


class ImportControlsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.base = Path(self.temporary.name).resolve()
        self.root = self.base / "launcher"
        self.root.mkdir()
        self.original = game_data(self.base / "original")
        (self.root / "settings.json").write_text(json.dumps({"autocheck": False, "data": str(self.original)}))
        self.window = ui.Launcher(self.root, True)
        self.window.show()
        self.app.processEvents()

    def tearDown(self):
        self.window.close()
        self.window.deleteLater()
        self.app.processEvents()

    def wait_job(self):
        deadline = time.monotonic() + 5
        while self.window.job is not None and time.monotonic() < deadline:
            self.app.processEvents()
            time.sleep(.01)
        self.assertIsNone(self.window.job, "import worker did not complete")

    def select_installer(self):
        return patch.object(QFileDialog, "getOpenFileName", return_value=(str(self.base / "setup.exe"), ""))

    def test_import_selects_and_persists_managed_data(self):
        def importer(root, installer, report, cancel):
            cancel()
            report("extract")
            return game_data(root / gog.DATA_DIRECTORY)
        with self.select_installer(), patch("gog.import_installer", side_effect=importer), \
                patch.object(self.window, "show_error") as error:
            self.window.gog_button.click()
            self.assertEqual(self.window.job, "gog")
            self.assertFalse(self.window.browse.isEnabled())
            self.assertFalse(self.window.gog_button.isEnabled())
            self.assertFalse(self.window.play.isEnabled())
            self.assertFalse(self.window.cancel_button.isHidden())
            self.wait_job()
        error.assert_not_called()
        self.assertEqual(self.window.data, self.root / gog.DATA_DIRECTORY)
        settings = json.loads((self.root / "settings.json").read_text())
        self.assertEqual(settings["data"], str(self.window.data))
        self.assertTrue(self.window.gog_button.isEnabled())
        self.assertIn("GOG", self.window.status.text())

    def test_cancellation_preserves_selected_data(self):
        with self.select_installer(), patch("gog.import_installer", side_effect=install.Cancelled()), \
                patch.object(self.window, "show_error") as error:
            self.window.gog_button.click()
            self.wait_job()
        error.assert_not_called()
        self.assertEqual(self.window.data, self.original)
        self.assertIn("отменена", self.window.status.text())
        self.assertTrue(self.window.gog_button.isEnabled())

    def test_missing_extractor_instructions(self):
        with self.select_installer(), patch("gog.import_installer", side_effect=gog.ExtractorMissing()), \
                patch.object(self.window, "show_error") as error:
            self.window.gog_button.click()
            self.wait_job()
        self.assertIn("brew install innoextract", error.call_args.args[0])
        self.assertEqual(self.window.data, self.original)

    def test_existing_import_requires_confirmation(self):
        game_data(self.root / gog.DATA_DIRECTORY)
        with self.select_installer(), patch.object(QMessageBox, "question", return_value=QMessageBox.StandardButton.Cancel), \
                patch("gog.import_installer") as importer:
            self.window.gog_button.click()
        importer.assert_not_called()
        self.assertIsNone(self.window.job)
        self.assertEqual(self.window.data, self.original)

    def test_game_blocks_import_and_engine_directory_still_rejected(self):
        self.window.game = object()
        self.window.refresh()
        self.assertFalse(self.window.gog_button.isEnabled())
        self.window.game = None
        self.window.refresh()
        with patch.object(self.window, "show_error") as error:
            self.window.set_data(self.root / "engine", "not allowed")
        error.assert_called_once()
        self.assertEqual(self.window.data, self.original)

    def test_window_cannot_close_during_extraction(self):
        self.window.job = "gog"
        event = QCloseEvent()
        self.window.closeEvent(event)
        self.assertFalse(event.isAccepted())
        self.window.job = None


if __name__ == "__main__":
    unittest.main()
