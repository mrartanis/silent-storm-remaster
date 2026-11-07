"""Import original game data from a user's offline GOG Inno Setup installer."""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import install


DATA_DIRECTORY = "game-data"
# Both standalone Silent Storm and Gold (which also contains Sentinels).
INSTALLER_ROOTS = ("app/Silent Storm", "app")
REQUIRED_SPACE = 4 * 1024**3


class ExtractorMissing(RuntimeError):
    pass


def innoextract():
    executable = 'innoextract.exe' if sys.platform == 'win32' else 'innoextract'
    if getattr(sys, 'frozen', False):
        bundled = Path(sys._MEIPASS) / 'innoextract' / executable
        if bundled.is_file():
            return str(bundled)
        raise ExtractorMissing('The launcher archive is incomplete: bundled innoextract is missing.')
    bundled = Path(__file__).resolve().parents[1] / 'build/launcher-innoextract' / executable
    if bundled.is_file():
        return str(bundled)
    candidates = ["innoextract"]
    if sys.platform == "darwin":
        # Finder does not inherit the user's Homebrew PATH.
        candidates += ["/opt/homebrew/bin/innoextract", "/usr/local/bin/innoextract"]
    for candidate in candidates:
        if executable := shutil.which(candidate):
            return executable
    raise ExtractorMissing("Install innoextract and add it to PATH (macOS: brew install innoextract).")


def extract(executable, installer, staging, cancel):
    command = [executable, "--extract", "--silent", "--color=0", "--progress=0",
               "--exclude-temp", "--no-extract-unknown", "--output-dir", str(staging / "unpacked")]
    # Supported path filters, not installer scripts: never execute the EXE.
    for root in INSTALLER_ROOTS:
        for name in (*install.DATA_FILES, *install.DATA_DIRS):
            command += ["--include", str(Path(root) / name)]
    command.append(str(installer))
    log = staging / "innoextract.log"
    with log.open("wb") as output:
        child = install.start_child(
            command, cwd=staging, env=install.host_environment(), stdin=subprocess.DEVNULL,
            stdout=output, stderr=subprocess.STDOUT,
            creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0,
        )
        try:
            while child.poll() is None:
                try:
                    child.wait(timeout=0.25)
                except subprocess.TimeoutExpired:
                    cancel()
        finally:
            if child.poll() is None:
                child.kill()
                child.wait()
    if child.returncode:
        with log.open("rb") as stream:
            stream.seek(max(0, log.stat().st_size - 2000))
            details = stream.read().decode("utf-8", errors="replace").strip()
        raise RuntimeError("innoextract could not read the installer. "
                           "For multipart downloads, keep all .bin files beside the .exe.\n" + details)


def data_root(unpacked):
    # Match installation-folder case without changing resource filenames.
    app = next((p for p in unpacked.iterdir() if p.name.casefold() == "app" and p.is_dir()
                and not p.is_symlink()), None) if unpacked.is_dir() else None
    candidates = ([p for p in app.iterdir() if p.name.casefold() == "silent storm" and p.is_dir()]
                  + [app]) if app else []
    for candidate in candidates:
        if not install.missing_data(candidate):
            return candidate
    raise ValueError("No complete Silent Storm 1.2 data found in the installer "
                     "(Sentinels alone is not supported).")


def import_installer(root, installer, progress, cancel):
    installer = Path(installer).resolve()
    if installer.suffix.lower() != ".exe" or not installer.is_file():
        raise ValueError("Select an offline GOG setup .exe file.")
    executable = innoextract()
    destination = root / DATA_DIRECTORY
    if destination.is_symlink() or (destination.exists() and not destination.is_dir()):
        raise ValueError("The game-data destination must be a regular directory.")
    cancel()
    install.require_space(root, REQUIRED_SPACE)
    with tempfile.TemporaryDirectory(prefix="gog-", dir=root) as temporary:
        staging = Path(temporary)
        progress("extract")
        extract(executable, installer, staging, cancel)
        cancel()
        source = data_root(staging / "unpacked")
        if any(path.is_symlink() for path in source.rglob("*")):
            raise ValueError("Symbolic links are not supported in imported game data.")
        payload = staging / "payload"
        payload.mkdir()
        for name in (*install.DATA_FILES, *install.DATA_DIRS):
            cancel()
            (source / name).rename(payload / name)
        cancel()
        install.activate(payload, destination)
    return destination
