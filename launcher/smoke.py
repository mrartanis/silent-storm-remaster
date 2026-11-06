"""Exercise the packaged launcher on a clean desktop and temporary install."""

import hashlib
import json
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory

from PySide6.QtCore import Qt, QTimer

import install


def hd_fixture(root):
    source = root / "fixture"
    textures = source / "res-hd"
    textures.mkdir(parents=True)
    (textures / "Textures.res").write_bytes(b"test textures")
    (textures / "manifest.json").write_text("{}", encoding="ascii")
    checksums = {f"res-hd/{file.name}": hashlib.sha256(file.read_bytes()).hexdigest()
                 for file in textures.iterdir()}
    archive = source / "fixture.7z"
    child = install.start_child(
        [install.hd_extractor(), "a", str(archive), "res-hd"], cwd=source,
        env=install.host_environment(), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    if child.wait(timeout=15):
        raise RuntimeError("Could not create HD test archive")
    tag = "hd-v0.0.0"
    index = source / f"{tag}-index.json"
    index.write_text(json.dumps({
        "release": tag,
        "payloadSha256": checksums,
        "manifestSha256": checksums["res-hd/manifest.json"],
        "nativeBytes": sum(file.stat().st_size for file in textures.iterdir()),
        "parts": [{"file": archive.name, "bytes": archive.stat().st_size,
                   "sha256": hashlib.sha256(archive.read_bytes()).hexdigest()}],
    }), encoding="ascii")
    return {"tag_name": tag, "assets": [
        {"name": file.name, "browser_download_url": file.as_uri(), "size": file.stat().st_size}
        for file in (archive, index)
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
                window.engine_button.click()
                return
            if not (engine / window.target.executable).is_file() or not window.install_folder.isEnabled():
                fail("The installed engine is missing or its folder action is disabled")
                return
            if not install.release_at(engine / "res-hd"):
                record(f"Installed and verified {install.release_at(engine)}")
                try:
                    window.hd_release = hd_fixture(root)
                except Exception as error:
                    fail(error)
                    return
                window.hd_button.click()
                return
            if (engine / "res-hd/Textures.res").read_bytes() != b"test textures":
                fail("The installed HD payload does not match the test archive")
                return
            record("Installed and verified the HD fixture")
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
