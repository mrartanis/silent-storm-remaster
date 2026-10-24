"""Keep the Windows and Linux game regression entry points in sync."""
import pathlib
import re
import sys


def registrations(path):
    text = path.read_text(encoding="utf-8")
    entries = re.findall(
        r"add_test\(NAME\s+((?:Upstream\S*|NativeBindInitialization))\s+"
        r"COMMAND\s+UpstreamBugTests\s+([^\s)]+)", text)
    result = {}
    for name, mode in entries:
        if name in result:
            raise AssertionError(f"Duplicate test {name} in {path.name}")
        result[name] = mode
    return result


def main():
    root = pathlib.Path(__file__).resolve().parent.parent
    windows = registrations(root / "CMakeLists.txt")
    linux = registrations(root / "cmake/LinuxGame.cmake")
    assert len(windows) >= 30 and len(linux) >= 30, "No upstream game registrations found"
    missing_linux = sorted(windows.keys() - linux.keys())
    missing_windows = sorted(linux.keys() - windows.keys())
    mismatch = sorted(name for name in windows.keys() & linux.keys()
                      if windows[name] != linux[name])
    if missing_linux or missing_windows or mismatch:
        print(f"FAIL: missing Linux: {missing_linux}; missing Windows: {missing_windows}; "
              f"different commands: {mismatch}")
        return 1
    print(f"PASS: {len(windows)} upstream regression entry points match on Windows and Linux")
    return 0


if __name__ == "__main__":
    sys.exit(main())
