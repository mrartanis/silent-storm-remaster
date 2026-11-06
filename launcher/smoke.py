"""Exercise the packaged launcher on a clean desktop and temporary install."""

from pathlib import Path
from tempfile import TemporaryDirectory

from PySide6.QtCore import Qt, QTimer

import install


def smoke_test(app, launcher, hero, log):
    with TemporaryDirectory(prefix="silent-storm-launcher-smoke-") as directory:
        root = Path(directory)
        log.write_text("Starting packaged launcher\n", encoding="utf-8")
        window = launcher(root, False)

        def record(line):
            with log.open("a", encoding="utf-8") as stream:
                stream.write(line + "\n")

        def fail(error):
            record(str(error))
            app.exit(1)

        def next_step(_result):
            if not install.release_at(root / "engine"):
                if not (window.engine_release and window.hd_release and window.engine_button.isEnabled()):
                    fail("Could not find both releases and enable the engine download")
                    return
                record(f"Found {window.engine_release['tag_name']} and {window.hd_release['tag_name']}")
                window.engine_button.click()
                return
            engine = root / "engine"
            if not (engine / window.target.executable).is_file() or not window.install_folder.isEnabled():
                fail("The installed engine is missing or its folder action is disabled")
                return
            record(f"Installed and verified {install.release_at(engine)}")
            app.exit(0)

        window.failed.disconnect(window.failure)
        window.failed.connect(fail, Qt.ConnectionType.QueuedConnection)
        window.succeeded.connect(next_step, Qt.ConnectionType.QueuedConnection)
        window.show()
        app.processEvents()
        if not window.target or not window.autocheck.isChecked() or not window.install_folder.isVisible():
            record("Desktop target or default controls missing")
            return 1
        # Exercise the real Qt plugins in the relocated archive.
        if window.findChild(hero).art.isNull() or window.grab().isNull():
            record("Launcher artwork or window capture is missing")
            return 1
        record(f"Window rendered on {window.target.label}")
        QTimer.singleShot(180_000, lambda: fail("Timed out waiting for the launcher"))
        return app.exec()
