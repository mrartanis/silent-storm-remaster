"""Exercise the packaged launcher on a clean desktop and temporary install."""

import hashlib
from pathlib import Path
import tarfile
from tempfile import TemporaryDirectory
import zipfile

from PySide6.QtCore import Qt, QTimer

import install


def engine_fixture(root, target):
    tag = "v0.0.0"
    name = install.engine_name(tag, target)
    is_zip = name.endswith(".zip")
    source = root / name.removesuffix(".zip" if is_zip else ".tar.gz")
    (source / "fonts").mkdir(parents=True)
    (source / target.executable).write_bytes(b"test engine")
    (source / "network-version.txt").write_text("test version", encoding="ascii")
    archive = root / name
    if is_zip:
        with zipfile.ZipFile(archive, "w") as bundle:
            for path in source.rglob("*"):
                bundle.write(path, path.relative_to(root))
    else:
        with tarfile.open(archive, "w:gz") as bundle:
            bundle.add(source, arcname=source.name)
    return {"tag_name": tag, "assets": [
        {"name": name, "browser_download_url": archive.as_uri(), "size": archive.stat().st_size,
         "digest": "sha256:" + hashlib.sha256(archive.read_bytes()).hexdigest()}
    ]}


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
            engine = root / "engine"
            if not install.release_at(engine):
                if not (window.engine_release and window.hd_release and window.engine_button.isEnabled()):
                    fail("Could not find both releases and enable the engine download")
                    return
                record(f"Found {window.engine_release['tag_name']} and {window.hd_release['tag_name']}")
                try:
                    window.engine_release = engine_fixture(root, window.target)
                except Exception as error:
                    fail(error)
                    return
                window.engine_button.click()
                return
            executable = engine / window.target.executable
            if not executable.is_file() or not window.install_folder.isEnabled():
                fail("The installed engine is missing or its folder action is disabled")
                return
            if executable.read_bytes() != b"test engine":
                fail("The installed engine payload does not match the test archive")
                return
            record("Installed and verified the local engine fixture")
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
        record(f"Window rendered on {window.target.label} ({app.platformName()})")
        QTimer.singleShot(180_000, lambda: fail("Timed out waiting for the launcher"))
        return app.exec()
