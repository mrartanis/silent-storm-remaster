"""Find registered Windows copies of Silent Storm without scanning drives."""

import os
from pathlib import Path
import re
import sys

from install import missing_data


STEAM_APP = "254960"
GOG_PRODUCT = "1207659058"
MAX_METADATA = 1024 * 1024
VDF_FIELD = re.compile(r'^\s*"([^"]+)"\s*"((?:\\.|[^"\\\r\n])*)"\s*$', re.MULTILINE)


def manifest_fields(path):
    try:
        with path.open("rb") as stream:
            contents = stream.read(MAX_METADATA + 1)
        if len(contents) > MAX_METADATA:
            return []
        return [(key.lower(), re.sub(r'\\([\\"])', r'\1', value))
                for key, value in VDF_FIELD.findall(contents.decode("utf-8-sig"))]
    except (OSError, ValueError):
        return []


def steam_folders(root):
    libraries = [root]
    for key, value in manifest_fields(root / "steamapps/libraryfolders.vdf"):
        if (key == "path" or key.isdecimal()) and Path(value).is_absolute():
            libraries.append(Path(value))
    for library in dict.fromkeys(libraries):
        steamapps = library / "steamapps"
        fields = dict(manifest_fields(steamapps / f"appmanifest_{STEAM_APP}.acf"))
        name = fields.get("installdir", "")
        if name and name not in (".", "..") and Path(name).name == name:
            yield steamapps / "common" / name


def registry_path(registry, hive, key, name, view=0):
    try:
        with registry.OpenKey(hive, key, 0, registry.KEY_READ | view) as handle:
            value, _ = registry.QueryValueEx(handle, name)
        if isinstance(value, str) and value:
            path = Path(os.path.expandvars(value))
            if path.is_absolute():
                return path
    except (OSError, ValueError):
        pass
    return None


def find_windows_data():
    if sys.platform != "win32":
        return []
    import winreg

    views = (winreg.KEY_WOW64_32KEY, winreg.KEY_WOW64_64KEY)
    steam_key = r"Software\Valve\Steam"
    steam_roots = [registry_path(winreg, winreg.HKEY_CURRENT_USER, steam_key, "SteamPath")]
    for view in views:
        steam_roots.append(
            registry_path(winreg, winreg.HKEY_LOCAL_MACHINE, steam_key, "InstallPath", view))
    candidates = []
    for root in dict.fromkeys(steam_roots):
        if root is not None:
            candidates.extend((path, "Steam") for path in steam_folders(root))

    gog_keys = (
        (rf"Software\GOG.com\Games\{GOG_PRODUCT}", "path"),
        (rf"Software\Microsoft\Windows\CurrentVersion\Uninstall\{GOG_PRODUCT}_is1", "InstallLocation"),
    )
    gog_roots = set()
    for hive in (winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER):
        for view in views:
            for key, name in gog_keys:
                root = registry_path(winreg, hive, key, name, view)
                if root is not None and root not in gog_roots:
                    gog_roots.add(root)
                    candidates.extend(((root, "GOG"), (root / "Silent Storm", "GOG")))

    matches = {}
    for path, store in candidates:
        try:
            if not missing_data(path):
                matches.setdefault(path.resolve(), store)
        except (OSError, ValueError, RuntimeError):
            continue
    return list(matches.items())
