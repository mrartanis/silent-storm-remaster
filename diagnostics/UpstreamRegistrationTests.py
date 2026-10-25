"""Keep the Windows and Linux game regression entry points in sync."""
import pathlib
import re
import sys
import argparse
import json
import subprocess


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
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=pathlib.Path)
    parser.add_argument("--ctest", default="ctest")
    parser.add_argument("--config", default="")
    parser.add_argument("--check-configured", choices=("0", "1"), default="0")
    args = parser.parse_args()
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
    if args.check_configured == "1":
        if args.build_dir is None:
            parser.error("--build-dir is required for the configured test check")
        # A no-argument engine regression must be available without game assets.
        # Check the generated CTest inventory, not just matching source files:
        # identical misplaced conditions on both platforms hide tests in CI.
        expected = set()
        for path in (root / "CMakeLists.txt", root / "cmake/LinuxGame.cmake"):
            expected.update(re.findall(
                r"^\s*add_test\(NAME\s+((?:Upstream\w*|NativeBindInitialization))\s+"
                r"COMMAND\s+UpstreamBugTests\s+[^\s)]+\s*\)",
                path.read_text(encoding="utf-8"), re.MULTILINE))
        command = [args.ctest, "--test-dir", str(args.build_dir), "--show-only=json-v1"]
        if args.config:
            command += ["-C", args.config]
        result = subprocess.run(command, capture_output=True, encoding="utf-8", errors="replace", timeout=30)
        if result.returncode:
            print(f"FAIL: cannot inspect configured CTest inventory: {result.stderr.strip()}")
            return 1
        configured = {test["name"] for test in json.loads(result.stdout)["tests"]}
        missing = sorted(expected - configured)
        if missing:
            print(f"FAIL: {len(missing)} resource-free engine regressions are absent from configured CTest: {missing}")
            return 1
        print(f"PASS: all {len(expected)} resource-free engine regressions are configured")
    return 0


if __name__ == "__main__":
    sys.exit(main())
