"""Query real CTest inventory without modifying the build's active reports."""
import argparse
import json
from pathlib import Path
import sys
import tempfile

from UpstreamRegistrationTests import query_inventory


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ctest", required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="inventory query ") as temporary:
        build = Path(temporary) / "build with spaces"
        nested = build / "nested"
        nested.mkdir(parents=True)
        executable = Path(sys.executable).as_posix()
        (build / "CTestTestfile.cmake").write_text(
            f'add_test(inventory_regular "{executable}" "-c" "pass")\n'
            'set_tests_properties(inventory_regular PROPERTIES LABELS "unit")\n'
            'if(CTEST_CONFIGURATION_TYPE STREQUAL "InventoryProbe")\n'
            f'  add_test(inventory_audio "{executable}" "-c" "pass")\n'
            '  set_tests_properties(inventory_audio PROPERTIES DISABLED TRUE LABELS "native-audio")\n'
            'endif()\nsubdirs("nested")\n', encoding="utf-8")
        (nested / "CTestTestfile.cmake").write_text(
            f'add_test(inventory_nested "{executable}" "-c" "pass")\n', encoding="utf-8")
        reports = build / "Testing" / "Temporary"
        reports.mkdir(parents=True)
        for name in ("LastTest.log", "LastTestsFailed.log", "CTestCostData.txt"):
            (reports / name).write_bytes(b"existing active report: " + name.encode("ascii") + b"\n")
        original = {p.relative_to(build): p.read_bytes() for p in build.rglob("*") if p.is_file()}
        for config in ("InventoryProbe", ""):
            result = query_inventory(args.ctest, build, config)
            assert result.returncode == 0, result.stderr
            inventory = {test["name"]: test for test in json.loads(result.stdout)["tests"]}
            expected = {"inventory_regular", "inventory_nested"}
            if config:
                expected.add("inventory_audio")
            assert set(inventory) == expected, inventory
            for test in inventory.values():
                assert Path(test["command"][0]) == Path(sys.executable) and test["command"][1:] == ["-c", "pass"], test
            if config:
                properties = {p["name"]: p["value"] for p in inventory["inventory_audio"]["properties"]}
                assert properties["DISABLED"] is True and properties["LABELS"] == ["native-audio"], properties
            current = {p.relative_to(build): p.read_bytes() for p in build.rglob("*") if p.is_file()}
            assert current == original, "read-only CTest inventory query overwrote the build's active reports"
    print("PASS: real CTest inventory preserves reports, nested tests, config, commands and audio properties")


if __name__ == "__main__":
    main()
