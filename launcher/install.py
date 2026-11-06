"""Release selection, verified installation, and the desktop engine entry point."""

import ctypes
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import ssl
import stat
import subprocess
import sys
import tarfile
import tempfile
from urllib.request import Request, urlopen
import zipfile

import certifi
from prepare_hd import payloads, seven_zip, valid


REPOSITORY = "https://github.com/mrartanis/silent-storm-remaster"
RELEASES = "https://api.github.com/repos/mrartanis/silent-storm-remaster/releases"
USER_AGENT = "silent-storm-desktop-launcher/0.1"
MARKER = ".launcher-release"
DATA_FILES = ("game.db", "start.cfg", "tree.mma")
DATA_DIRS = ("cfg", "res", "scripts")


@dataclass(frozen=True)
class Target:
    suffix: str
    executable: str
    label: str


def desktop_target():
    machine = platform.machine().lower()
    if machine in ("x86_64", "amd64"):
        if sys.platform == "win32":
            return Target("windows-x64.zip", "Game.exe", "Windows x64")
        if sys.platform == "linux":
            return Target("linux-x64.tar.gz", "Game", "Linux x64")
    if sys.platform == "darwin" and machine in ("arm64", "aarch64"):
        return Target("macos-arm64.tar.gz", "Game", "macOS ARM64")
    return None


def missing_data(path):
    if path is None:
        return [*DATA_FILES, *DATA_DIRS]
    return ([name for name in DATA_FILES if not (path / name).is_file()]
            + [name for name in DATA_DIRS if not (path / name).is_dir()])


def version(tag):
    if re.fullmatch(r"v\d+(?:\.\d+){1,3}", tag):
        return tuple(int(field) for field in tag[1:].split(".")) + (0,) * (3 - tag.count("."))
    return ()


def asset_table(release):
    return {asset["name"]: asset for asset in release["assets"]}


def engine_name(tag, target):
    return f"silent-storm-engine-{tag}-{target.suffix}"


def choose_releases(releases, target):
    engines, hd = [], []
    for release in releases:
        if release["draft"] or release["prerelease"] or not release["published_at"]:
            continue
        tag = release["tag_name"]
        assets = asset_table(release)
        if target and version(tag) and engine_name(tag, target) in assets:
            engines.append(release)
        if re.fullmatch(r"hd-[A-Za-z0-9][A-Za-z0-9._-]*", tag) and f"{tag}-index.json" in assets:
            hd.append(release)
    return (max(engines, key=lambda r: version(r["tag_name"]), default=None),
            max(hd, key=lambda r: r["published_at"], default=None))


def github_open(url, *, api_token=None):
    context = ssl.create_default_context()
    context.load_verify_locations(cafile=certifi.where())
    request = Request(url, headers={"User-Agent": USER_AGENT})
    if api_token:
        request.add_unredirected_header("Authorization", f"Bearer {api_token}")
    return urlopen(request, timeout=20, context=context)


def check_releases(target, *, api_token=None):
    releases = []
    for page in range(1, 101):
        with github_open(f"{RELEASES}?per_page=100&page={page}", api_token=api_token) as response:
            batch = json.load(response)
        releases.extend(batch)
        if len(batch) < 100:
            break
    return choose_releases(releases, target)


class Cancelled(Exception):
    pass


def require_space(path, size):
    if shutil.disk_usage(path).free < size:
        raise OSError(f"Free space required: {size / 1e9:.2f} GB ({path})")


def asset_digest(asset):
    value = asset.get("digest") or ""
    return value[7:].lower() if re.fullmatch(r"sha256:[0-9a-fA-F]{64}", value) else None


def download(asset, destination, cancel, report=lambda size: None, expected=None):
    cancel()
    sha = hashlib.sha256()
    size = 0
    with github_open(asset["browser_download_url"]) as response, destination.open("xb") as output:
        while block := response.read(1024 * 1024):
            cancel()
            size += len(block)
            if size > asset["size"]:
                raise ValueError(f"Unexpected download size: {destination.name}")
            sha.update(block)
            output.write(block)
            report(size)
    if size != asset["size"]:
        raise ValueError(f"Incomplete download: {destination.name}")
    for digest in (expected, asset_digest(asset)):
        if digest and sha.hexdigest() != digest:
            raise ValueError(f"SHA-256 mismatch: {destination.name}")
    return sha.hexdigest()


def engine_checksum(assets, name, staging, cancel):
    if digest := asset_digest(assets[name]):
        return digest
    sums = staging / "SHA256SUMS"
    download(assets["SHA256SUMS"], sums, cancel)
    for line in sums.read_text(encoding="ascii").splitlines():
        fields = line.split(maxsplit=1)
        if len(fields) == 2 and fields[1].lstrip("*") == name:
            if re.fullmatch(r"[0-9a-fA-F]{64}", fields[0]):
                return fields[0].lower()
    raise ValueError(f"No SHA-256 entry for {name}")


def unpack_engine(archive, staging, target, cancel):
    is_zip = target.suffix.endswith(".zip")
    expected = archive.name.removesuffix(".zip" if is_zip else ".tar.gz")
    with (zipfile.ZipFile(archive) if is_zip else tarfile.open(archive, "r:gz")) as bundle:
        entries = bundle.infolist() if is_zip else bundle.getmembers()
        require_space(staging, sum(entry.file_size if is_zip else entry.size for entry in entries))
        for entry in entries:
            cancel()
            name = entry.filename if is_zip else entry.name
            parts = name.rstrip("/").split("/")
            if (parts[0] != expected or any(
                    part in ("", ".", "..") or part.endswith((" ", "."))
                    or re.search(r'[\\:*?"<>|\x00-\x1f]', part) for part in parts)
                    or (len(parts) > 1 and parts[1].casefold() in ("res-hd", MARKER))):
                raise ValueError(f"Invalid engine archive entry: {name}")
            directory = entry.is_dir() if is_zip else entry.isdir()
            regular = (stat.S_IFMT(entry.external_attr >> 16) in (0, stat.S_IFREG)
                       if is_zip else entry.isfile())
            destination = staging.joinpath(*parts)
            if directory:
                destination.mkdir(parents=True, exist_ok=True)
            elif regular:
                destination.parent.mkdir(parents=True, exist_ok=True)
                source = bundle.open(entry) if is_zip else bundle.extractfile(entry)
                with source, destination.open("xb") as output:
                    while block := source.read(1024 * 1024):
                        cancel()
                        output.write(block)
                if not is_zip:
                    destination.chmod(entry.mode & 0o777)
            else:
                raise ValueError(f"Unsupported engine archive entry: {name}")
    payload = staging / expected
    for name in (target.executable, "network-version.txt", "fonts"):
        if not (payload / name).exists():
            raise ValueError(f"Incomplete engine package: {name}")
    return payload


def release_at(path):
    marker = path / MARKER
    try:
        return marker.read_text(encoding="ascii").strip()
    except (OSError, ValueError):
        return ""


def recover_swap(destination, shared=None):
    previous = destination.with_name(f".{destination.name}.previous")
    if previous.exists():
        if not destination.exists():
            previous.rename(destination)
        else:
            if shared and (previous / shared).exists() and not (destination / shared).exists():
                (previous / shared).rename(destination / shared)
            shutil.rmtree(previous)
    return previous


def activate(payload, destination, shared=None):
    previous = recover_swap(destination, shared)
    if destination.exists():
        destination.rename(previous)
    try:
        payload.rename(destination)
    finally:
        recover_swap(destination, shared)


def install_engine(root, target, release, progress, cancel):
    tag = release["tag_name"]
    name = engine_name(tag, target)
    assets = asset_table(release)
    asset = assets[name]
    require_space(root, asset["size"])
    with tempfile.TemporaryDirectory(prefix="engine-", dir=root) as temporary:
        staging = Path(temporary)
        checksum = engine_checksum(assets, name, staging, cancel)
        archive = staging / name
        progress("download", 0, asset["size"])
        download(asset, archive, cancel,
                 lambda size: progress("download", size, asset["size"]), checksum)
        progress("extract")
        payload = unpack_engine(archive, staging, target, cancel)
        (payload / MARKER).write_text(tag, encoding="ascii")
        cancel()
        activate(payload, root / "engine", shared="res-hd")


def start_child(command, *, cwd=None, env=None, **options):
    # Frozen launchers change the library lookup for their own Qt. Restore the
    # host lookup while starting the separately distributed engine or 7-Zip.
    frozen_windows = sys.platform == "win32" and getattr(sys, "frozen", False)
    if frozen_windows:
        ctypes.windll.kernel32.SetDllDirectoryW(None)
    try:
        return subprocess.Popen(command, cwd=cwd, env=env, **options)
    finally:
        if frozen_windows:
            ctypes.windll.kernel32.SetDllDirectoryW(sys._MEIPASS)


def host_environment():
    env = os.environ.copy()
    if sys.platform == "linux" and getattr(sys, "frozen", False):
        env.pop("LD_LIBRARY_PATH", None)
        if "LD_LIBRARY_PATH_ORIG" in env:
            env["LD_LIBRARY_PATH"] = env.pop("LD_LIBRARY_PATH_ORIG")
    return env


def hd_extractor():
    try:
        return seven_zip()
    except RuntimeError:
        if sys.platform == "darwin":
            # Finder does not inherit Homebrew's shell PATH.
            for path in ("/opt/homebrew/bin/7zz", "/usr/local/bin/7zz"):
                if executable := shutil.which(path):
                    return executable
        raise


def install_hd(root, release, progress, cancel):
    target = root / "engine" / "res-hd"
    tag = release["tag_name"]
    assets = asset_table(release)
    with tempfile.TemporaryDirectory(prefix="hd-", dir=root) as temporary:
        staging = Path(temporary)
        index_path = staging / f"{tag}-index.json"
        signature = download(assets[index_path.name], index_path, cancel)
        index = json.loads(index_path.read_text(encoding="utf-8"))
        if index["release"] != tag or not index["parts"]:
            raise ValueError(f"Invalid HD index for {tag}")
        expected = payloads(index)
        progress("verify")
        if valid(target, expected, signature):
            cancel()
            (target / MARKER).write_text(tag, encoding="ascii")
            return
        extractor = hd_extractor()
        parts = index["parts"]
        total = sum(part["bytes"] for part in parts)
        require_space(root, total + index["nativeBytes"] + 16 * 1024 * 1024)
        done = 0
        progress("download", 0, total)
        for part in parts:
            name = part["file"]
            if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*\.7z(?:\.\d+)?", name):
                raise ValueError(f"Invalid HD volume name: {name}")
            asset = assets[name]
            if asset["size"] != part["bytes"] or not re.fullmatch(r"[0-9a-f]{64}", part["sha256"]):
                raise ValueError(f"Invalid HD volume metadata: {name}")
            download(asset, staging / name, cancel,
                     lambda size: progress("download", done + size, total), part["sha256"])
            done += part["bytes"]
        progress("extract")
        log = staging / "7zip.log"
        with log.open("wb") as output:
            child = start_child(
                [extractor, "x", "-y", "-bd", "-o" + str(staging), str(staging / parts[0]["file"])],
                env=host_environment(), stdout=output, stderr=subprocess.STDOUT,
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
            raise RuntimeError(log.read_text(encoding="utf-8", errors="replace").strip()[-1200:])
        progress("verify")
        unpacked = staging / "res-hd"
        if not valid(unpacked, expected, signature):
            raise ValueError("Extracted HD pack did not pass SHA-256 verification")
        (unpacked / MARKER).write_text(tag, encoding="ascii")
        cancel()
        activate(unpacked, target)


def saves_root():
    home = Path.home()
    if override := os.environ.get("S2_USER_DATA_DIR"):
        return Path(override).expanduser()
    if sys.platform == "win32":
        return Path(os.environ.get("LOCALAPPDATA", home / "AppData/Local")) / "Silent Storm Reconstruction"
    if sys.platform == "darwin":
        return home / "Library/Application Support/Silent Storm Reconstruction"
    return Path(os.environ.get("XDG_DATA_HOME") or home / ".local/share") / "silent-storm-reconstruction"


def start_game(root, target, data, windowed=False):
    engine = root / "engine"
    env = host_environment()
    if sys.platform == "linux":
        env["LD_LIBRARY_PATH"] = os.pathsep.join(
            [str(engine / "lib"), *filter(None, [env.get("LD_LIBRARY_PATH")])])
    args = [str(engine / target.executable), "-windowed" if windowed else "-borderless"]
    with (root / "game.log").open("wb") as log:
        return start_child(args, cwd=data, env=env, stdout=log, stderr=subprocess.STDOUT)
